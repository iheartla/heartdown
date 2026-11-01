# H❤️rtDown: Document Processor for Executable Linear Algebra Papers

H❤️rtDown is an authoring environment for linear algebra papers. The papers compile to interactive web pages and code libraries that can be used inline to create figures or in external code.

The code for the SIGGRAPH Asia 2022 paper is at the tag [`SIGGRAPH_Asia_2022`](https://github.com/iheartla/heartdown/tree/SIGGRAPH_Asia_2022) (`git checkout SIGGRAPH_Asia_2022`). That version's editor needs a local Python server; see the README there. The current version runs entirely in the browser.

## Running the editor

The editor is a static web page in `docs/`. H❤️rtDown, the I❤️LA compiler and the Python code of figures run in the browser with [Pyodide](https://pyodide.org), so there is nothing to install.

1. Get the code:

       git clone https://github.com/iheartla/heartdown.git

   (Running the editor doesn't need the submodules; `docs/` has everything it needs.)

2. Serve `docs/` with any web server, for example:

       cd heartdown/docs
       python3 -m http.server

3. Open <http://localhost:8000/> in a browser. Chrome or Edge is recommended, because only they can open a folder on your computer (see below).

The editor needs a web server, rather than opening `docs/index.html` as a file, because it uses a service worker to show the paper. Any `https://` host, such as GitHub Pages serving `docs/`, also works.

The editor starts with a short example paper. Click **Compile** (Ctrl/⌘-R) to see it. Click an equation to edit its I❤️LA code, or a figure to edit the Python code that draws it.

### Papers and files

A paper is a Markdown file (`.md`) in a folder, next to the files it uses: a `.bib` file, imported `.ihla` modules and images. Compiling writes the paper's outputs into its folder:

- `<paper>.html`: the interactive paper. It needs the `heartdown-resource/` folder and the paper's images next to it.
- `output_code/lib.py`, `lib.h`, `lib.m`: the paper's code in NumPy, C++/Eigen and MATLAB.
- The figures, and each figure's code (`<figure>.py`).

You can keep a paper in either of two places:

- **Open folder** (Chrome and Edge) works on a paper's folder on your computer. Compiling and saving (Ctrl/⌘-S) write the paper and its outputs to that folder. The editor offers to reopen the folder next time.
- **Open files**, **Import folder** and drag-and-drop (any browser) copy a paper and its files into the browser's storage, where the editor keeps them. **Paper** downloads the `.md` file. **Zip** downloads the folder with all its outputs. **New** starts a new paper in the browser.

The first compile of a paper with figure code takes a few seconds longer, because it loads NumPy, SciPy and Matplotlib. If figure code imports other packages that Pyodide provides, such as scikit-image, they load from the Pyodide CDN, which needs a network connection.

## Compiling from the command line

The command line needs Python and the submodules:

    git clone --recursive https://github.com/iheartla/heartdown.git
    cd heartdown

(In an existing clone, run `git submodule update --init` instead.)

Install the dependencies. With [Anaconda](https://www.anaconda.com/products/individual) or [Miniconda](https://docs.conda.io/en/latest/miniconda.html):

    conda env create -f environment-heartdown.yml
    conda activate heartdown

To update the environment when `environment-heartdown.yml` changes, run `conda env update --file environment-heartdown.yml --prune`.

Then compile a paper:

    python app.py --paper path/to/paper.md

This writes `path/to/paper.html`, `output_code/` and the figures next to the paper, as the editor does.

## Developing

[Architecture.md](Architecture.md) describes how H❤️rtDown and the editor work.

`docs/` holds copies of the Python code and of Pyodide. After changing `app.py`, or the `markdown` or `iheartla` submodules, rebuild the copy of the Python code:

    sh docs/update_python.sh     # writes docs/heartdown-py.zip

To update Pyodide or the Python packages in `docs/js/pyodide/`, edit the versions in `docs/update_pyodide.sh` and run it.
