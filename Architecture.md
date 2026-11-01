# H❤️rtDown architecture

H❤️rtDown turns a Markdown paper with I❤️LA code blocks into an interactive web page, plus code libraries (NumPy, C++/Eigen, MATLAB) that the paper's figures and other programs can use. This note describes how it is built and why, and in particular how the editor runs in a browser without a server.

## Overview

```
paper.md ──▶ app.py: process_input
               ├─ Python-Markdown (markdown submodule) with H❤️rtDown's extensions:
               │    iheartla_code ── compiles each context's I❤️LA code ──▶ I❤️LA compiler (iheartla submodule)
               │    mdx_bib, mdx_math, meta (YAML front matter), …
               ├─ post-processing: title, abstract, section numbers, contexts, equation numbers
               ├─ figures: runs each <figure>'s Python code against the generated lib.py
               └─ writes paper.html, output_code/lib.{py,h,m}, <context>.ihla, heartdown-resource/
```

There are two ways to run it:

- **The command line:** `python app.py --paper paper.md`.
- **The editor in `docs/`:** a static web page that runs the same Python code in [Pyodide](https://pyodide.org).

The editor used to be a Tornado server (`server.py`) that the page called for every compile. That server is gone.

## The compiler pipeline (Python)

`app.py` is the single entry point for both ways of running H❤️rtDown.

- `process_input(content, input_dir, file_name)` converts a paper and writes its outputs next to it.
- `compile_paper()` wraps it for the editor. It reports errors in a dict instead of raising them, including the I❤️LA code that has the error, so the editor can highlight it.
- `run_figure()` re-runs one figure's code after it is edited in the editor.

The Markdown processing is Python-Markdown with H❤️rtDown's extensions, in the `markdown` submodule. `iheartla_code` does most of the work:

- It groups the I❤️LA blocks of a paper by context (`❤: name`) and compiles each context with `compile_la_content(..., get_json=True)`. That returns the code for each output, plus the equation data: symbols, types, descriptions and dependencies.
- It replaces each block with MacroMathJax that the paper's `paper.js` makes clickable.
- It builds the glossary JSON (`iheartla_data`, `sym_data`).
- Compiles are cached by a hash of each context's code, so editing prose doesn't recompile.

Figures are `<figure>`s that contain a ```` ```python ```` block. `iheartla_code` saves each block as `<image name>.py`. `app.py` copies the generated `lib.py` next to it and runs it. The script draws the figure's image.

## The editor (browser)

```
index.html + js/editor.js ─────────────── main thread
   │   ▲                        │
   │   │ files the compile      │ serves the paper's files
   │   │ wrote                  ▼
   │   │                  js/project.js ◀──────── sw.js (service worker)
   ▼   │                  the paper's folder            ▲
js/heartdown-worker.js      (disk or IndexedDB)         │ GET paper/<session>/<path>
   Pyodide: app.py, markdown,                           │
   iheartla, figure code                       preview <iframe> (the generated paper.html)
   /paper = mirror of the folder
```

- **`js/heartdown-worker.js`** (a module Web Worker) loads Pyodide, the vendored packages and `heartdown-py.zip`. That zip holds `app.py`, the `markdown` and `iheartla` Python code and `extras/heartdown-resource`, laid out as in this repository.
  - The paper's folder is mirrored at `/paper` in Pyodide's file system.
  - Each request (`compile`, `figure`, `reset`) carries the files that changed in the editor since the last one.
  - Each reply carries the files that the compile or figure run wrote. The worker finds them by comparing modification times and sizes before and after the run.
- **`js/project.js`** holds the open paper's folder: every file, in memory, keyed by its path in the folder. It stores the folder in one of two ways (below). It also answers the service worker's requests and builds zip downloads.
- **`sw.js`** serves `paper/<session>/<path>` to the preview `<iframe>`. It asks the editor tab with that session id for the file. The generated `paper.html` can therefore load its images, figures and `heartdown-resource/` through relative URLs, exactly as it did from the server and as it does from disk. The iframe has the editor's origin, so `paper.js` can still call the editor (`parent.onEditEquation`, `parent.onClickFigure`, `parent.showMsg`).
- **`js/editor.js`** is the old editor's UI (Ace, equation and figure dialogs, Unicode shortcuts), with its server calls replaced by worker requests and file I/O.

### Storage

- **Folder:** the File System Access API (Chrome, Edge). The editor reads the folder once and writes every saved or generated file straight to it, as the server did. The folder's handle is kept in IndexedDB, so the next visit offers to reopen it; that needs a click to grant permission again.
- **Browser:** for browsers without the File System Access API, or for papers not on disk. **Open files**, **Import folder** or drag-and-drop copies files into IndexedDB. **Paper** and **Zip** download the paper, or the folder with its generated files. Only one browser paper is kept at a time.
- A paper in the URL (`?code=`, from "Copy as URL") opens as a browser paper.

### Vendored files

- **`docs/js/pyodide/`** (44 MB, from `docs/update_pyodide.sh`): the Pyodide runtime, the packages compiling needs (regex, sympy, pyyaml, TatSu, appdirs, pybtex), and NumPy, SciPy and Matplotlib.
  - The generated `lib.py` imports NumPy and SciPy. These and Matplotlib load the first time a paper has figure code.
  - `pyodide-lock.json` lists every other Pyodide package with its CDN URL. A figure that imports one of them (e.g. scikit-image) loads it from the CDN on first use.
- **`docs/heartdown-py.zip`** (from `docs/update_python.sh`): the Python code. Rebuild it after changing `app.py` or a submodule.

## Decisions

**Run H❤️rtDown's Python code in Pyodide, rather than porting it to JavaScript or Rust.**
- Pyodide keeps one implementation for the command line and the editor, and the editor's output matches the command line's exactly.
- Figures are Python code, so Pyodide is needed in any case.
- A JavaScript port would mean rewriting about 1,500 lines of H❤️rtDown's extensions and replacing Python-Markdown's behavior with a different Markdown parser's.
- The cost is a 3 s start-up (cached after the first visit) and the 44 MB of vendored files.

**Use the Python I❤️LA compiler, not the Rust one.**
- The iheartla repository's `rust` branch has a Rust compiler built to WebAssembly, which is much faster, especially for I❤️MESH imports.
- Its WebAssembly interface returns only code strings. H❤️rtDown needs `compile_la_content`'s equation data: symbol tables with types, descriptions, dependencies, local functions and the `expr_dict` for MathJax.
- With the Python compiler, a plain paper compiles in 0.4–3 s on first load and in 0.1 s after that, thanks to the per-context cache.
- `app.compile_paper()` is the seam for a later switch: a Rust compiler that exports the same equation data could replace `compile_la_content` in `iheartla_code`.

**The `iheartla` submodule tracks the `mesh` branch.**
- Its Python compiler is identical to the `rust` branch's (`rust` adds the Rust compiler), and it is the branch where the Python compiler is developed.
- The newer compiler broke 5 gallery papers. Five small fixes are committed on `mesh` (see its log): sum scopes in the equation data, `.ihla` module lookup, empty names, multiple imports from one package, and subscripted function parameters.

**The `markdown` submodule is H❤️rtDown's fork, rebased onto upstream Python-Markdown 3.11.2.**
- The fork was Python-Markdown 3.3.4 plus about 170 H❤️rtDown commits. Its `heartdown` branch replays those commits onto 3.11.2 and ports the extensions to the 3.4+ API.
- The output for every gallery paper is byte-identical to the old fork's.
- Upstream's test suite fails only 4 tests (the old fork failed 28). All 4 come from one deliberate change: `meta` parses YAML front matter.
- The old fork is on the `heartdown-py3` branch.
- The core changes are small: `Markdown` carries H❤️rtDown's state (`path`, `lib_py`, `json_data`, …), and preprocessors accept `path`.

**The compiler is imported as `iheartla.*`.** `app.py` puts the `iheartla` submodule on `sys.path`, because the package now uses absolute imports. (It used to be imported through the repository layout as `iheartla.iheartla.*`.)

**Serve the preview with a service worker, not `srcdoc` or blob URLs.**
- The generated HTML refers to files by relative paths, and so do `paper.js`, MathJax's `attr-label` loader, figure iframes and the paper's own HTML.
- A service worker serves them all unchanged, with the editor's origin, so `paper.js`'s `parent.*` calls still work.
- Service workers need `https://` or `http://localhost`, so the editor needs a web server. Any static server works.

**Figures run inside Pyodide's interpreter.** Pyodide has no subprocesses or threads.
- `run_figure_script` runs each script with `runpy` in the paper's folder.
- It reloads `lib` so the script sees the latest `lib.py`, closes Matplotlib figures afterwards, and returns the traceback as the figure's error.
- On the command line, the script runs in a subprocess, as before, with the same Python (`sys.executable`).

**One worker, mirrored files.**
- The worker keeps Pyodide and the compile caches across compiles, and receives only the files that changed.
- Opening another paper resets the mirror and the figure caches.

## Limitations

- **Eccentricity**, one of the gallery papers, doesn't compile with the newer compiler. A keyword followed by a letter (`π𝑑`) now lexes as one identifier, so `π𝑑(𝐿)` is an undefined symbol.
- The generated `paper.html` still loads MathJax, Popper and Tippy from CDNs. The page is H❤️rtDown's output, not part of the editor, and is meant to be viewed online.
- Opening a folder on disk works only in Chromium browsers. Other browsers use browser storage and downloads.
- A file written by a compile is detected by its modification time and size. A file rewritten in the same millisecond with the same size would be missed. The paper's outputs are always written during the compile, after the snapshot is taken.
