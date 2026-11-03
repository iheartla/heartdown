"""
Open each demo of the in-browser editor (docs/) from its Demos menu, compile it and check that it and its figures
succeed, in headless Google Chrome. This tests what only the editor does: the compiler in Pyodide, figure code with
Pyodide's packages (some from its CDN or PyPI), and the Demos menu fetching papers and their files.
It doesn't compare the output with expected files; test_corpus.py does that.
It also checks that figures get packages from PyPI: from their imports and from the paper's front matter.

It needs Google Chrome and a network connection, and takes a few minutes, so it only runs with --browser:

    uv run --group browser pytest --browser tests/test_browser.py
"""
import functools
import http.server
import json
import threading
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
    :return: open_editor(), which (re)loads the editor in a page and returns the page, once its compiler is ready
    """
    sync_playwright = pytest.importorskip("playwright.sync_api").sync_playwright
    server = http.server.ThreadingHTTPServer(("localhost", 0), functools.partial(QuietHandler, directory=DOCS_DIR))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    with sync_playwright() as p:
        browser = p.chromium.launch(channel="chrome", headless=True)
        page = browser.new_page()
        # Opening a demo asks before replacing the paper kept in the browser
        page.on("dialog", lambda dialog: dialog.accept())

        def open_editor():
            page.goto("http://localhost:{}/index.html".format(server.server_port))
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
