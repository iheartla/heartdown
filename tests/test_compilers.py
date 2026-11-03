"""
Compile each paper of the corpus and each demo of the editor with the Rust and the Python I❤️LA compilers
(HEARTDOWN_COMPILER), and check that the papers are the same: the HTML, and the data it gives the page's scripts,
apart from the names of sequences' sizes, which the page doesn't use. The generated code (lib.py, ...) may differ.

The Python compiler is slower (about a minute for all of them), so this only runs with --compilers:

    uv run pytest --compilers tests/test_compilers.py
"""
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest
import regex as re

from test_corpus import COMPILE, CORPUS_DIR

pytestmark = pytest.mark.skipif("not config.getoption('--compilers')", reason="needs --compilers")

DOCS_EXAMPLES_DIR = Path(__file__).parent.parent / "docs" / "examples"
PAPERS = sorted([("corpus", p.relative_to(CORPUS_DIR).as_posix()) for p in CORPUS_DIR.rglob("*.md")] +
                [("docs", p.relative_to(DOCS_EXAMPLES_DIR).as_posix()) for p in DOCS_EXAMPLES_DIR.rglob("*.md")])
BASE_DIRS = {"corpus": CORPUS_DIR, "docs": DOCS_EXAMPLES_DIR}

# Papers whose pages differ: path: reason. Their tests check that they still do.
KNOWN_DIFFERENCES = {
    "heartdown_examples_simple/example-fairing/L/fairing.md": "w = 10^6 is in ℤ for Rust (an integer to a non-negative literal power), in ℝ for Python",
    "heartdown_examples_simple/example-fairing/L2/fairing.md": "w = 10^6 is in ℤ for Rust (an integer to a non-negative literal power), in ℝ for Python",
    "heartdown_examples_simple/example-fairing/L3/fairing.md": "w = 10^6 is in ℤ for Rust (an integer to a non-negative literal power), in ℝ for Python",
    "simple/fairing/L.md": "w = 10^6 is in ℤ for Rust (an integer to a non-negative literal power), in ℝ for Python",
    "simple/fairing/L2.md": "w = 10^6 is in ℤ for Rust (an integer to a non-negative literal power), in ℝ for Python",
    "simple/fairing/L3.md": "w = 10^6 is in ℤ for Rust (an integer to a non-negative literal power), in ℝ for Python",
}

# const iheartla_data = JSON.parse('...');
JSON_LINE_RE = re.compile(r"^(?P<before>\s*const (?P<name>\w+) = JSON\.parse\(')(?P<json>.*)('\);)$")
JS_ESCAPE_RE = re.compile(r"\\(u[0-9a-fA-F]{4}|x[0-9a-fA-F]{2}|.)")
JS_ESCAPES = {"n": "\n", "r": "\r", "t": "\t", "b": "\b", "f": "\f", "v": "\v", "0": "\0"}


def js_string(text):
    """The value of a single-quoted JavaScript string literal's text"""
    def unescape(m):
        e = m.group(1)
        if e[0] in "ux" and len(e) > 1:
            return chr(int(e[1:], 16))
        return JS_ESCAPES.get(e, e)
    return JS_ESCAPE_RE.sub(unescape, text)


def without_sequence_sizes(value):
    if isinstance(value, dict):
        return {k: without_sequence_sizes(v) for k, v in value.items() if not (k == "size" and value.get("type") == "sequence")}
    if isinstance(value, list):
        return [without_sequence_sizes(v) for v in value]
    return value


def page(html):
    """
    :return: (the page's lines, except its JSON data; {name: data})
    """
    lines, data = [], {}
    for line in html.split("\n"):
        m = JSON_LINE_RE.match(line)
        if m:
            data[m.group("name")] = without_sequence_sizes(json.loads(js_string(m.group("json"))))
            lines.append(m.group("before"))
        else:
            lines.append(line)
    return lines, data


def compile_html(base, paper, work_dir, compiler):
    """Compile a copy of the paper with the compiler; return its HTML"""
    paper_dir = work_dir / compiler
    shutil.copytree((BASE_DIRS[base] / paper).parent, paper_dir)
    source = paper_dir / Path(paper).name
    ret = subprocess.run([sys.executable, "-c", COMPILE, str(source)], capture_output=True, text=True,
                         env=dict(os.environ, HEARTDOWN_COMPILER=compiler))
    assert ret.returncode == 0, "{}: {}".format(compiler, ret.stderr)
    return source.with_suffix(".html").read_text(encoding="utf-8")


@pytest.mark.parametrize("base, paper", PAPERS, ids=["{}/{}".format(*p) for p in PAPERS])
def test_same_paper(base, paper, tmp_path):
    rust_lines, rust_data = page(compile_html(base, paper, tmp_path, "rust"))
    python_lines, python_data = page(compile_html(base, paper, tmp_path, "python"))
    same = rust_lines == python_lines and rust_data == python_data
    if paper in KNOWN_DIFFERENCES:
        assert not same, "the same now; remove it from KNOWN_DIFFERENCES"
        pytest.xfail(KNOWN_DIFFERENCES[paper])
    assert rust_lines == python_lines
    assert rust_data == python_data
