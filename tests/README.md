# Tests

`test_corpus.py` compiles each paper in `corpus/` and checks that the compiler writes the files in `expected/`, with the same contents: the paper's `.html`, `output_code/lib.py`, `lib.h` and `lib.m`, the `.ihla` modules and the figures' code. Figure code doesn't run.

    uv run pytest

When a change to the compiler changes its output on purpose, check the diffs, then save the new output as the expected output and commit it:

    uv run pytest --update-expected

A paper that doesn't compile yet can go in `KNOWN_FAILURES` in `test_corpus.py`. Its test checks that it still fails to compile, so once it compiles, the test fails until it's removed from there and its expected output is saved.

## The corpus

The `.md` and `.bib` files of the papers in

- `corpus/heartdown_examples_simple/`: <https://github.com/iheartla/heartdown_examples_simple> at 3f7e0b7
- `corpus/paper_md/`: the `papers/` folder of <https://github.com/iheartla/paper_md> at 2aa0ba1

without the images, PDFs and generated files.

`test_requirements.py` checks how `app.paper_requirements` reads the `requirements:` of a paper's front matter.

## The editor's demos

`test_browser.py` opens each demo in the in-browser editor's Demos menu (`docs/examples/demos.json`), compiles it and checks that it and its figures succeed, in headless Google Chrome. It tests what only the editor does: the compiler in Pyodide, figure code with Pyodide's packages (some from its CDN or PyPI), and the Demos menu fetching papers and their files. It doesn't compare the output with expected files. It also checks that figures get the packages they need from PyPI: libigl for `import igl`, and the requirements a paper's front matter lists, when compiling the paper or running one figure. And it checks the papers in links: `data:` URLs, **Share** links and older `?code=` links, and that edits are saved without compiling.

It needs Google Chrome and a network connection and takes a few minutes, so it runs only with `--browser`:

    uv run --group browser pytest --browser tests/test_browser.py
