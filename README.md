# H❤️rtDown: Document Processor for Executable Linear Algebra Papers

H❤️rtDown is an authoring environment for linear algebra papers. The papers compile to interactive web pages and code libraries that can be used inline to create figures or in external code.

The code for the [SIGGRAPH Asia 2022 paper](https://iheartla.github.io/heartdown/) is at the tag [`SIGGRAPH_Asia_2022`](https://github.com/iheartla/heartdown/tree/SIGGRAPH_Asia_2022) (`git checkout SIGGRAPH_Asia_2022`); see the README there. That version compiles papers with a local Python server. The current version compiles them in the browser.

## Running the editor

The editor is a web page in `docs/`. H❤️rtDown, the I❤️LA compiler and the Python code of figures run in the browser with [Pyodide](https://pyodide.org), so there is nothing to install besides Python 3 for the small local server.

Get the code (no submodules needed; `docs/` has everything):

    git clone https://github.com/iheartla/heartdown.git
    cd heartdown

Serve `docs/` via GitHub Pages or:

    cd docs
    python3 -m http.server

Served this way, the editor starts with an example paper, and you choose where papers are kept:

### Loading examples

- **Open folder** (Chrome and Edge) works on a paper's folder on your computer, reading and writing it directly. The editor offers to reopen the folder next time.
- **Open files**, **Import folder** and drag-and-drop (any browser) copy a paper and its files into the browser's storage, where the editor keeps them. **Paper** downloads the `.md` file. **Zip** downloads the folder with everything it generated. **New** starts a new paper in the browser.
- **Demos** opens the basic example (the one a new paper starts from), one of the simple examples from [heartdown_examples_simple](https://github.com/iheartla/heartdown_examples_simple) (in `docs/examples/simple/`), or one of the case studies from the [H❤️rtDown gallery](https://iheartla.github.io/heartdown/CaseStudies/gallery/index.html) (all listed in `docs/examples/demos.json`).
- **Open URL**, or a link to the editor with `?paper=<url>`, opens a paper on the web into the browser's storage. The URL can be:
  - a paper (`.md` or `.txt`): the editor also fetches its `.bib` and the files it refers to (such as `<img src="./img/fig1.png">`), relative to the paper's URL;
  - a zip of the paper's folder;
  - a `data:` URL of the paper's text or a zip, such as `data:text/markdown;base64,…`.

  For example, `https://<editor>/?paper=https://iheartla.github.io/heartdown/CaseStudies/static/gallery_res/Judder/judder.txt`. The paper's server must allow other sites to load it (CORS), as GitHub Pages does. Compiling a paper runs its figure code in your browser, so compile only papers you trust.
- **Share** copies a link to the editor with the paper's text in it, compressed, after `#z=`. Opening the link opens the paper into the browser's storage. The link has only the paper's text, not its other files. Since it's in the URL's fragment, the text isn't sent to the editor's server. Links with the text uncompressed, `?code=<the paper's text>`, open too.

Wherever the paper is kept, the editor saves edits to it as you type. Ctrl/⌘-S saves right away.

### On a paper on your computer

    python3 server.py path/to/paper.md

This opens the editor in your browser (any browser) on that paper. Edits to the paper are saved into the paper's folder as you type, and compiling writes everything it generates there. If the paper doesn't exist yet, it is created from an example. Options: `--port` (default 8000) and `--no-browser`. `server.py` only needs Python's standard library.

### Using the editor

Click **Compile** (Ctrl/⌘-R) to compile the paper and show it. Click an equation to edit its I❤️LA code, or a figure to edit the Python code that draws it.

A paper is a Markdown file (`.md`) in a folder, next to the files it uses: a `.bib` file, imported `.ihla` modules and images. Compiling writes into the paper's folder:

- `<paper>.html`: the interactive paper. It needs `heartdown-resource/` and the paper's images next to it.
- `output_code/lib.py`, `lib.h`, `lib.m`: the paper's code in NumPy, C++/Eigen and MATLAB.
- The figures, and each figure's code (`<figure>.py`).

**Code** downloads the generated code (`lib.py`, `lib.h` or `lib.m`, or all three), the same code the command line generates.

The first compile of a paper with figure code takes a few seconds longer, because it loads NumPy, SciPy and Matplotlib. If figure code imports other packages that Pyodide provides, such as scikit-image, they load from the Pyodide CDN. Packages Pyodide doesn't provide, such as plotly or libigl (`import igl`), install from PyPI if they're pure Python or have a WebAssembly wheel. Both need a network connection. A figure can also need a package it doesn't import by that name, a particular version, or a package that only its own modules import. List those PyPI requirements in the paper's YAML front matter, and the editor installs them before running the figures:

    ---
    requirements:
      - libigl>=2.6.4.dev3
      - python-slugify
    ---

Outside the editor, figure code runs with the environment's Python, which needs these packages installed already.

## Compiling from the command line

The command line needs the submodules and [uv](https://docs.astral.sh/uv/):

    git clone --recursive https://github.com/iheartla/heartdown.git
    cd heartdown
    uv run heartdown --paper path/to/paper.md

In an existing clone, run `git submodule update --init` first. The first `uv run` creates a virtual environment (`.venv/`) with the dependencies from `pyproject.toml`. The command writes next to the paper, as the editor does:

- `path/to/paper.html`, the interactive paper, with `heartdown-resource/` (its scripts and styles) next to it;
- `output_code/`, the paper's code;
- the figures.

The paper can be a `.md` or `.txt` file, and `--paper` takes several. To share one `heartdown-resource/` among several papers, as the gallery does, give its location relative to the papers with `--resource_dir`. For example, for `gallery_res/Judder/judder.txt`:

    uv run heartdown --paper gallery_res/Judder/judder.txt --resource_dir ..

writes `gallery_res/Judder/judder.html`, which loads `gallery_res/heartdown-resource/`. Figure code runs with this environment's Python, so add any packages it needs with `uv add`.

Without a clone, uv can install H❤️rtDown and its submodules from GitHub and run it:

    uvx --from git+https://github.com/iheartla/heartdown heartdown --paper path/to/paper.md

`heartdown-server` in place of `heartdown` opens the editor on a paper, as `server.py` does. Add packages for figure code with `--with`, as in `uvx --with scikit-image --from ...`.

## Developing

[Architecture.md](Architecture.md) describes how H❤️rtDown and the editor work.

The submodules in `deps/` are H❤️rtDown's two main dependencies, and uv installs them from there in editable mode:

- `deps/iheartla`: the I❤️LA compiler, on its `main` branch.
- `deps/markdown`: H❤️rtDown's fork of Python-Markdown, on its `heartdown` branch.

Changes to either take effect immediately. Commit them in the submodule, then commit the submodule's new version here.

To check that a change doesn't change the compiler's output, run the tests ([tests/README.md](tests/README.md)), which compile a corpus of papers:

    uv run pytest

`docs/` holds copies of the Python code and of Pyodide. After changing `app.py` or a submodule, rebuild the copy of the Python code:

    sh docs/update_python.sh     # writes docs/heartdown-py.zip

To update Pyodide or the Python packages in `docs/js/pyodide/`, edit the versions in `docs/update_pyodide.sh` and run it.
