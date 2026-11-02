"""
Compile each paper in tests/corpus and compare everything the compiler writes with tests/expected.

Figure code doesn't run: the tests check the compiler, and figures need more packages (plotly, igl) and time.
After a change that should change the output, check the diffs and accept the new output with

    uv run pytest --update-expected
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

TESTS_DIR = Path(__file__).parent
CORPUS_DIR = TESTS_DIR / "corpus"
EXPECTED_DIR = TESTS_DIR / "expected"
# Copied from extras/ as is
RESOURCE_DIR = "heartdown-resource"
# Compiles the paper argv[1] without running figure code
COMPILE = """
import logging, os, sys
from heartdown import app
from iheartla.la_tools.la_logger import LaLogger
app.run_figure_script = lambda source, cwd: (True, '')
LaLogger.getInstance().set_level(logging.ERROR)
paper = sys.argv[1]
with open(paper, encoding='utf-8') as f:
    app.process_input(f.read(), os.path.dirname(paper), file_name=os.path.splitext(os.path.basename(paper))[0])
"""

# Papers that don't compile yet: path in the corpus: reason. Their tests check that they still don't.
KNOWN_FAILURES = {
}


def list_files(folder, skip=()):
    """
    :return: {path relative to folder: bytes} of the files in folder, except in the folders in skip
    """
    files = {}
    for root, dirs, names in os.walk(folder):
        dirs[:] = [d for d in dirs if d not in skip]
        for name in names:
            path = Path(root) / name
            files[path.relative_to(folder).as_posix()] = path.read_bytes()
    return files


def compile_paper(paper, work_dir):
    """
    Compile a copy of a corpus paper in work_dir, in a new Python, so that papers don't share the compiler's state
    :return: {path relative to the paper: bytes} of the files the compiler wrote
    """
    paper_dir = work_dir / "paper"
    shutil.copytree((CORPUS_DIR / paper).parent, paper_dir)
    inputs = list_files(paper_dir)
    ret = subprocess.run([sys.executable, "-c", COMPILE, str(paper_dir / Path(paper).name)],
                         capture_output=True, text=True)
    if paper in KNOWN_FAILURES:
        assert ret.returncode != 0, "compiles now; remove it from KNOWN_FAILURES and save its expected output"
        pytest.xfail(KNOWN_FAILURES[paper])
    assert ret.returncode == 0, ret.stderr
    outputs = list_files(paper_dir, skip=(RESOURCE_DIR,))
    return {path: content for path, content in outputs.items() if inputs.get(path) != content}


@pytest.mark.parametrize("paper", sorted(paper.relative_to(CORPUS_DIR).as_posix() for paper in CORPUS_DIR.rglob("*.md")))
def test_paper(paper, tmp_path, request):
    outputs = compile_paper(paper, tmp_path)
    expected_dir = EXPECTED_DIR / Path(paper).parent
    if request.config.getoption("--update-expected"):
        shutil.rmtree(expected_dir, ignore_errors=True)
        for path, content in outputs.items():
            (expected_dir / path).parent.mkdir(parents=True, exist_ok=True)
            (expected_dir / path).write_bytes(content)
        return
    assert expected_dir.exists(), "no expected output; run pytest --update-expected"
    expected = list_files(expected_dir)
    assert sorted(outputs) == sorted(expected), "the compiler wrote different files"
    for path in sorted(expected):
        # as text, for pytest's diff
        assert outputs[path].decode("utf-8") == expected[path].decode("utf-8"), "{} differs".format(path)
