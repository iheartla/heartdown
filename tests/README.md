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
