# H❤️rtDown: Document Processor for Executable Linear Algebra Papers

H❤️rtDown is an authoring environment for linear algebra papers. The papers compile to interactive web pages and code libraries that can be used inline to create figures or in external code.

## The editor

The editor runs entirely in the browser: H❤️rtDown, the I❤️LA compiler and the Python code of figures run in [Pyodide](https://pyodide.org). It is a static site in `docs/`. To run it locally, serve that folder with any web server:

    cd docs
    python3 -m http.server

and open `http://localhost:8000/`. (Opening `docs/index.html` as a file won't work.)

A paper is a Markdown file in a folder, next to the files it uses (`.bib`, imported `.ihla` modules, images). In the editor:

- **Open folder** (Chrome and Edge) opens a paper's folder on your computer. Compiling saves the paper there and writes the generated HTML (`<paper>.html`), code (`output_code/lib.py`, `lib.h`, `lib.m`), figures and `heartdown-resource/` into it, like the old Python server did. The editor offers to reopen the folder next time.
- **Open files** and **Import folder** (any browser) copy a paper and its files into the browser's storage. **Paper** and **Zip** download the paper, or its folder with everything generated, as a zip.
- Compile with the button or Ctrl/⌘-R; save with Ctrl/⌘-S.

The first compile of a paper with Python figure code loads NumPy, SciPy and Matplotlib. Other packages that figure code imports load from the Pyodide CDN if Pyodide provides them.

### Updating the editor

`docs/` holds a copy of the Python code and of Pyodide, so after changing `app.py`, the `markdown` or `iheartla` submodules, or updating the submodules, run:

    sh docs/update_python.sh     # app.py, markdown/, iheartla/, extras/heartdown-resource -> docs/heartdown-py.zip

To update Pyodide or the vendored Python packages, edit the versions in `docs/update_pyodide.sh` and run it.

## The command line

To compile a paper from the command line, install [Anaconda](https://www.anaconda.com/products/individual) or [Miniconda](https://docs.conda.io/en/latest/miniconda.html). Then:

    conda env create -f environment-heartdown.yml
    conda activate heartdown

To update an already created environment if the `environment-heartdown.yml` file changes, activate it and then run `conda env update --file environment-heartdown.yml --prune`.

Then:

    python app.py --paper path/to/document.md

This writes `path/to/document.html`, the generated code and the figures next to the paper.
