"""
Open each demo of the in-browser editor (docs/) from its Demos menu, compile it and check that it and its figures
succeed, in headless Google Chrome. This tests what only the editor does: the compiler in Pyodide, figure code with
Pyodide's packages (some from its CDN or PyPI), and the Demos menu fetching papers and their files.
It doesn't compare the output with expected files; test_corpus.py does that.

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
