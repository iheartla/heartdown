"""
Open each demo of the in-browser editor (docs/) from its Demos menu, compile it and check that it and its figures
succeed, in headless Google Chrome. This tests what only the editor does: the compiler in Pyodide, figure code with
Pyodide's packages (some from its CDN or PyPI), and the Demos menu fetching papers and their files.
It doesn't compare the output with expected files; test_corpus.py does that.
It also checks that figures get packages from PyPI: from their imports and from the paper's front matter.

It needs Google Chrome and a network connection, and takes a few minutes, so it only runs with --browser:

    uv run --group browser pytest --browser tests/test_browser.py
"""
import base64
import functools
import http.server
import json
import threading
import urllib.parse
import zlib
from pathlib import Path

import pytest

DOCS_DIR = Path(__file__).parent.parent / "docs"
DEMOS = json.loads((DOCS_DIR / "examples" / "demos.json").read_text(encoding="utf-8"))
# The first compile loads NumPy, SciPy and Matplotlib, and some figures install packages
COMPILE_TIMEOUT = 300000
LOAD_TIMEOUT = 180000

pytestmark = pytest.mark.skipif("not config.getoption('--browser')", reason="needs --browser")


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass


@pytest.fixture(scope="module")
def editor():
    """
    :return: open_editor(link=""), which (re)loads the editor in a page and returns the page, once its compiler is
    ready. link: what follows index.html in the editor's URL, such as "?paper=<url>"
    """
    sync_playwright = pytest.importorskip("playwright.sync_api").sync_playwright
    server = http.server.ThreadingHTTPServer(("localhost", 0), functools.partial(QuietHandler, directory=DOCS_DIR))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    with sync_playwright() as p:
        browser = p.chromium.launch(channel="chrome", headless=True)
        # Share copies a link to the clipboard
        page = browser.new_context(permissions=["clipboard-read", "clipboard-write"]).new_page()
        # Opening a demo asks before replacing the paper kept in the browser
        page.on("dialog", lambda dialog: dialog.accept())

        def open_editor(link=""):
            # (about:blank first: a link that changes only the fragment wouldn't reload the editor)
            page.goto("about:blank")
            page.goto("http://localhost:{}/index.html{}".format(server.server_port, link))
            page.wait_for_function("typeof workerReady !== 'undefined' && workerReady", timeout=LOAD_TIMEOUT)
            # Keep what the worker answers to a compile
            page.evaluate("""() => {
                const ask_worker = ask;
                window.ask = async (request) => {
                    const data = await ask_worker(request);
                    if (request.type === "compile") window.lastCompile = data;
                    return data;
                };
            }""")
            return page
        yield open_editor
        browser.close()
    server.shutdown()


@pytest.fixture
def page(editor):
    # the editor reloaded for each demo, so that one's failure doesn't affect the next
    return editor()


@pytest.mark.parametrize("title", [demo["title"] for group in DEMOS["groups"] for demo in group["demos"]])
def test_demo(page, title):
    page.evaluate("window.lastCompile = null")
    page.evaluate("clickDemos()")
    page.get_by_role("button", name=title, exact=True).click()
    page.wait_for_function("document.getElementById('msg').textContent.includes('Loaded the demo')", timeout=LOAD_TIMEOUT)
    page.evaluate("clickCompile()")
    page.wait_for_function("window.lastCompile !== null && !compiling", timeout=COMPILE_TIMEOUT, polling=500)
    data = page.evaluate("window.lastCompile")
    if data["ret"] != 0:
        pytest.fail(data.get("msg"), pytrace=False)
    if data.get("fig"):
        pytest.fail("\n".join("figure {}:\n{}".format(name, error) for name, error in data["fig"].items()), pytrace=False)


def paper_with_figure(front_matter, code):
    """
    :return: a paper with this YAML front matter (without the ---s) and a figure drawn by code, which saves
    cotangent.png. (A figure's code is saved as its image's name, so cotangent.py doesn't hide a module.)
    """
    return """---
full_paper: False
{}
---
❤: requirements

# Requirements

```iheartla
f(x) = 2x where x ∈ ℝ
```

<figure>
```python
{}
```
<img src="cotangent.png">
</figure>
""".format(front_matter, code)


# The cotangent Laplacian of a square of two triangles, with libigl, which the version must be
IGL_FIGURE = """from lib import *
import importlib.metadata
import numpy as np
import igl
import matplotlib.pyplot as plt
assert importlib.metadata.version("libigl") == {version!r}, importlib.metadata.version("libigl")
L = igl.cotmatrix(np.array([[0., 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0]]), np.array([[0, 1, 2], [0, 2, 3]]))
assert np.allclose(L.toarray(), [[-1, .5, 0, .5], [.5, -1, .5, 0], [0, .5, -1, .5], [.5, 0, .5, -1]])
plt.imshow(L.toarray())
plt.savefig("cotangent.png")"""


def compile_source(page, source):
    """
    :return: the compile's result, failing the test if the paper or a figure fails
    """
    data = page.evaluate("source => ask({type: 'compile', name: 'requirements', source})", source)
    if data["ret"] != 0:
        pytest.fail(data.get("msg"), pytrace=False)
    if data.get("fig"):
        pytest.fail("\n".join("figure {}:\n{}".format(name, error) for name, error in data["fig"].items()), pytrace=False)
    return data


def test_import_from_pypi(page):
    # import igl installs libigl, whose only WebAssembly wheel is a development release
    compile_source(page, paper_with_figure("", IGL_FIGURE.format(version="2.6.4.dev3")))


def test_import_sympy(page):
    # The editor doesn't load SymPy or TatSu, which compiling no longer needs; a figure that imports SymPy loads it
    # (from Pyodide's CDN) on first use
    compile_source(page, paper_with_figure("", """import sys
assert "tatsu" not in sys.modules and "appdirs" not in sys.modules, [m for m in ("tatsu", "appdirs") if m in sys.modules]
import sympy
import matplotlib.pyplot as plt
assert sympy.integrate(sympy.Symbol("x"), (sympy.Symbol("x"), 0, 2)) == 2
plt.plot([0, 1])
plt.savefig("cotangent.png")"""))


def test_front_matter_requirements(page):
    compile_source(page, paper_with_figure("requirements:\n  - libigl==2.6.4.dev3",
                                           IGL_FIGURE.format(version="2.6.4.dev3")))


def test_front_matter_requirement_fails(page):
    error = pytest.importorskip("playwright.sync_api").Error
    with pytest.raises(error, match="Couldn't install no-such-package-heartdown-test from PyPI"):
        page.evaluate("source => ask({type: 'compile', name: 'requirements', source})",
                      paper_with_figure("requirements: no-such-package-heartdown-test", "import numpy"))


def test_front_matter_requirements_figure_run(page):
    # Running one figure, without compiling the paper first, installs the paper's requirements.
    # python-slugify is imported as slugify, so only the front matter can say to install it.
    code = """import importlib.metadata
import slugify
import matplotlib.pyplot as plt
importlib.metadata.version("python-slugify")
assert slugify.slugify("Hello World") == "hello-world"
plt.plot([0, 1])
plt.savefig("cotangent.png")"""
    result = page.evaluate("a => ask({type: 'figure', name: 'cotangent', source: a[0], paper: a[1]})",
                           [code, paper_with_figure("requirements:\n  - python-slugify", code)])
    assert result["ok"], result["msg"]


# A paper for links, with what URLs encode: %, #, +, and Unicode
LINK_PAPER = "---\nfull_paper: False\n---\n# A ❤️ paper in a link\n\nIt has % and # and a+b=c, ünïcode, and $x^2$.\n"


def wait_for_paper(page, text):
    page.wait_for_function("text => project !== null && ace.edit('editor').getValue() === text", arg=text,
                           timeout=LOAD_TIMEOUT)


@pytest.mark.parametrize("link", [
    "?paper=data:text/markdown;base64," + base64.b64encode(LINK_PAPER.encode()).decode(),
    "?paper=" + urllib.parse.quote("data:text/markdown;base64," + base64.b64encode(LINK_PAPER.encode()).decode(), safe=""),
    "?paper=" + urllib.parse.quote("data:text/markdown;charset=utf-8," + urllib.parse.quote(LINK_PAPER), safe=""),
], ids=["base64", "base64, percent-encoded", "text, percent-encoded"])
def test_data_url(editor, link):
    page = editor(link)
    wait_for_paper(page, LINK_PAPER)
    # the link is then forgotten, so that reloading keeps the edits
    assert "paper=" not in page.url


def test_code_link(editor):
    # links from older versions of the editor
    page = editor("?code=" + urllib.parse.quote(LINK_PAPER, safe=""))
    wait_for_paper(page, LINK_PAPER)


def test_share(page):
    # long enough that the compressed text is converted to base64 in several parts
    text = LINK_PAPER + "".join("Line {} of the paper, {}.\n".format(i, i * i) for i in range(5000))
    page.evaluate("text => ace.edit('editor').setValue(text, -1)", text)
    page.evaluate("clickShare()")
    link = page.evaluate("navigator.clipboard.readText()")
    assert "#zip=" in link and "?" not in link
    assert len(link) < len(text) / 3
    page.evaluate("ace.edit('editor').setValue('', -1)")
    # pasted into this page's address bar: only the fragment changes
    page.evaluate("link => { window.location.href = link; }", link)
    wait_for_paper(page, text)
    # and opened in a new page
    page.evaluate("ace.edit('editor').setValue('', -1)")
    page = page.context.new_page()
    page.goto(link)
    wait_for_paper(page, text)
    assert "#" not in page.url
    page.close()


def test_text_link(editor):
    # Share's links before they had the paper's files: the text, deflated, in base64url
    deflate = zlib.compressobj(9, zlib.DEFLATED, -15)
    z = base64.urlsafe_b64encode(deflate.compress(LINK_PAPER.encode()) + deflate.flush()).decode().rstrip("=")
    page = editor("#z=" + z)
    wait_for_paper(page, LINK_PAPER)


def share(page):
    """
    :return: the link Share copies, and its message
    """
    page.evaluate("navigator.clipboard.writeText('')")
    page.evaluate("clickShare()")
    return page.evaluate("navigator.clipboard.readText()"), page.evaluate("document.getElementById('msg').textContent")


def open_link(page, link, text):
    """
    Open link in a new page of the browser
    :return: the page, once it has the paper, whose text is text
    """
    page = page.context.new_page()
    page.on("dialog", lambda dialog: dialog.accept())
    page.goto(link)
    wait_for_paper(page, text)
    return page


def project_files(page):
    return page.evaluate("Object.fromEntries([...project.files].map(([path, bytes]) => [path, bytes.length]))")


def test_share_files(page):
    # The paper, a module, a small image, an image too big for a link, and what compiling made
    page.evaluate("""async text => {
        // (random: doesn't compress, as an image doesn't. getRandomValues fills 64 KB at most.)
        const random = (n) => {
            const bytes = new Uint8Array(n);
            for (let i = 0; i < n; i += 65536) crypto.getRandomValues(bytes.subarray(i, i + 65536));
            return bytes;
        };
        const files = new Map([["paper.md", new TextEncoder().encode(text)],
                               ["shape.py", new TextEncoder().encode("SIDES = 3\\n")],
                               ["small.png", random(20000)], ["big.png", random(300000)]]);
        await setProject(await openSharedProject(files, "paper.md"));
        await project.write("figure.png", random(1000), true);
        await project.write("heartdown-resource/paper.js", new TextEncoder().encode("// made by compiling"));
    }""", LINK_PAPER)
    link, msg = share(page)
    assert "base=" not in link and len(link) < 50000
    assert "leaves out big.png" in msg
    shared = open_link(page, link, LINK_PAPER)
    assert project_files(shared) == {"paper.md": len(LINK_PAPER.encode()), "shape.py": 10, "small.png": 20000}
    shared.close()


def test_share_demo(page):
    # A paper from a URL: the link has its URL, where its image comes from
    page.evaluate("clickDemos()")
    page.get_by_role("button", name="Image filtering with a box filter", exact=True).click()
    page.wait_for_function("document.getElementById('msg').textContent.includes('Loaded the demo')", timeout=LOAD_TIMEOUT)
    text = page.evaluate("ace.edit('editor').getValue()")
    link, msg = share(page)
    assert "base=" in link and len(link) < 5000
    shared = open_link(page, link, text)
    shared.wait_for_function("project.files.has('wikipedia.png')", timeout=LOAD_TIMEOUT)
    shared.wait_for_function("typeof workerReady !== 'undefined' && workerReady", timeout=LOAD_TIMEOUT)
    shared.evaluate("""() => {
        const ask_worker = ask;
        window.ask = async (request) => {
            const data = await ask_worker(request);
            if (request.type === "compile") window.lastCompile = data;
            return data;
        };
        window.lastCompile = null;
    }""")
    shared.evaluate("clickCompile()")
    shared.wait_for_function("window.lastCompile !== null && !compiling", timeout=COMPILE_TIMEOUT, polling=500)
    data = shared.evaluate("window.lastCompile")
    assert data["ret"] == 0 and not data.get("fig"), data
    # what compiling made isn't shared
    link, msg = share(shared)
    assert "base=" in link and len(link) < 5000
    shared.close()


def test_damaged_link(editor):
    page = editor("#z=bm90IGRlZmxhdGVk")
    page.wait_for_function("document.getElementById('msg').textContent.includes('damaged')", timeout=LOAD_TIMEOUT)


def test_edits_saved(editor):
    # without compiling or Ctrl/⌘-S
    page = editor()
    page.evaluate("text => ace.edit('editor').setValue(text, -1)", LINK_PAPER + "Edited.\n")
    page.wait_for_function("text => project.readText(project.mdPath) === text", arg=LINK_PAPER + "Edited.\n")
    page = editor()
    wait_for_paper(page, LINK_PAPER + "Edited.\n")
