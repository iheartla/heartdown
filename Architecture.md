# H❤️rtDown architecture

H❤️rtDown turns a Markdown paper with I❤️LA code blocks into an interactive web page, plus code libraries (NumPy, C++/Eigen, MATLAB) that the paper's figures and other programs can use. This note describes how it is built and why, and in particular how the editor compiles papers in the browser.

## Overview

```
paper.md ──▶ app.py: process_input
               ├─ Python-Markdown (deps/markdown) with H❤️rtDown's extensions:
               │    iheartla_code ── compiles each context's I❤️LA code ──▶ I❤️LA compiler (deps/iheartla)
               │    mdx_bib, mdx_math, meta (YAML front matter), …
               ├─ post-processing: title, abstract, section numbers, contexts, equation numbers
               ├─ figures: runs each <figure>'s Python code against the generated lib.py
               └─ writes paper.html, output_code/lib.{py,h,m}, <context>.ihla, heartdown-resource/
```

There are two ways to run it:

- **The command line:** `uv run heartdown --paper paper.md` (`app.main`).
- **The editor in `docs/`:** a web page that runs the same Python code in [Pyodide](https://pyodide.org). It can be served from anywhere, or by `server.py` for a paper on disk.

The editor used to call a Tornado server (`server.py`) for every compile, which ran H❤️rtDown natively. Now `server.py` is a small standard-library server that only serves the editor and the paper's files; compiling happens in the browser in every case.

## The compiler pipeline (Python)

`app.py` is the single entry point for both ways of running H❤️rtDown.

- `process_input(content, input_dir, file_name)` converts a paper and writes its outputs next to it.
- `compile_paper()` wraps it for the editor. It reports errors in a dict instead of raising them, including the I❤️LA code that has the error, so the editor can highlight it.
- `run_figure()` re-runs one figure's code after it is edited in the editor.

The Markdown processing is Python-Markdown with H❤️rtDown's extensions, in `deps/markdown`. `iheartla_code` does most of the work:

- It groups the I❤️LA blocks of a paper by context (`❤: name`) and compiles each context with `compile_la_content(..., get_json=True)`. That returns the code for each output, plus the equation data: symbols, types, descriptions and dependencies.
- It replaces each block with MacroMathJax that the paper's `paper.js` makes clickable.
- It builds the glossary JSON (`iheartla_data`, `sym_data`).
- Symbol descriptions come only from the author: a description after the type in a `where` block, or a `<span class="def:K">` or `<span class="def">` in the prose. A context's prose also describes the symbols of the other modules whose code is in that context. An imported symbol, or a module parameter and the argument bound to it, is the same symbol in both modules, so a description of either describes both. Descriptions aren't inferred from ordinary prose: on the gallery papers, a "$X$ is …" heuristic found definitions for 7 of 104 undescribed symbols, some of them wrong.
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

- **`js/heartdown-worker.js`** (a module Web Worker) loads Pyodide, the vendored packages and `heartdown-py.zip`. That zip holds `app.py`, `extras/heartdown-resource`, and the `iheartla` and `markdown` packages at the top level, as if installed.
  - The paper's folder is mirrored at `/paper` in Pyodide's file system.
  - Each request (`compile`, `figure`, `reset`) carries the files that changed in the editor since the last one.
  - Each reply carries the files that the compile or figure run wrote. The worker finds them by comparing modification times and sizes before and after the run.
- **`js/project.js`** holds the open paper's folder: every file, in memory, keyed by its path in the folder. It stores the folder in one of three ways (below). It also answers the service worker's requests and builds zip downloads.
- **`sw.js`** serves `paper/<session>/<path>` to the preview `<iframe>`. It asks the editor tab with that session id for the file. The generated `paper.html` can therefore load its images, figures and `heartdown-resource/` through relative URLs, exactly as it did from the server and as it does from disk. The iframe has the editor's origin, so `paper.js` can still call the editor (`parent.onEditEquation`, `parent.onClickFigure`, `parent.showMsg`).
- **`js/editor.js`** is the old editor's UI (Ace, equation and figure dialogs, Unicode shortcuts), with its server calls replaced by worker requests and file I/O.

### Storage

- **Local:** when `server.py` serves the editor for a paper (`python3 server.py paper.md`), in any browser. The editor reads the folder through `GET /local/files` and `/local/file/<path>`, and writes through `PUT /local/file/<path>`. The server only listens on 127.0.0.1, refuses other `Host` names (against DNS rebinding) and paths outside the folder, and uses `PUT` for writes, which another site's page can't send without CORS approval. The editor checks for `/local/paper` when it loads, so the same `docs/` works with or without `server.py`.
- **Folder:** the File System Access API (Chrome, Edge). The editor reads the folder once and writes every saved or generated file straight to it. The folder's handle is kept in IndexedDB, so the next visit offers to reopen it; that needs a click to grant permission again.
- **Browser:** for browsers without the File System Access API, or for papers not on disk. **Open files**, **Import folder** or drag-and-drop copies files into IndexedDB. **Paper**, **Code** and **Zip** download the paper, its generated code, or the folder with its generated files. Only one browser paper is kept at a time.
- A paper in the link opens as a browser paper: `?code=<the paper's text>` (from "Copy as URL"), or `?paper=<url>` (or **Open URL**).
  - For a `.md` or `.txt`, the editor fetches the paper, its `.bib`, and the relative files it refers to (`src`, `href`, Markdown links), relative to its URL. It remembers that URL, so the preview fetches anything else it asks for from there on first use; in sw.js terms, a file missing from the folder.
  - For a zip (detected by its contents, not its name), it unzips the folder (`DecompressionStream`).
  - The parameter is then removed from the address bar, so a reload restores the browser paper with its edits rather than loading the link again.
  - **Demos** lists the basic example (`docs/examples/starter.md`, which a new paper starts from), the simple examples (`docs/examples/simple/`) and the gallery's case studies (`docs/examples/demos.json`). It opens a simple example or case study as it opens a URL, with the other files the demo lists (such as a module its figure code imports), which the paper doesn't refer to with `src`. The simple examples are copies of [heartdown_examples_simple](https://github.com/iheartla/heartdown_examples_simple) whose figures run in Pyodide: `make_cylinder.py` without NumPy 1's `asfarray`, the filters' output saved as 8-bit, and the smoothing's cotangent Laplacian from `laplacian.py` (SciPy) instead of libigl, which has no WebAssembly build.
  - Opening a paper replaces the browser paper. The editor asks first only if that paper was edited since it was created or loaded (`Project.edited`, kept with it): an untouched example, a paper from a link, or opened files are easy to get back.
  - Compiling runs the paper's figure code in the page's origin, so the editor says to compile only papers you trust. Messages escape text from papers and URLs.

### Vendored files

- **`docs/js/pyodide/`** (44 MB, from `docs/update_pyodide.sh`): the Pyodide runtime, the packages compiling needs (regex, sympy, pyyaml, TatSu, appdirs, pybtex), and NumPy, SciPy and Matplotlib.
  - The generated `lib.py` imports NumPy and SciPy. These and Matplotlib load the first time a paper has figure code.
  - `pyodide-lock.json` lists every other Pyodide package with its CDN URL. A figure that imports one of them (e.g. scikit-image) loads it from the CDN on first use. A figure that imports a package Pyodide doesn't have (e.g. plotly) installs it from PyPI with micropip, which works for pure-Python packages. `OPTIONAL_PACKAGES` in `heartdown-worker.js` adds the packages a module needs without requiring them (pandas for `plotly.express`).
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

**The `iheartla` submodule tracks the `main` branch.**
- `main` contains the `mesh` branch, where the Python compiler was developed.
- The newer compiler broke 6 of the 14 gallery papers. The fixes were committed on `mesh` (see its log): sum scopes in the equation data, `.ihla` module lookup, empty names, multiple imports from one package, symbol keywords such as `π` beginning a name (`π𝑑` is π times 𝑑), and subscripted function parameters. A parameter `D_i: ℝ^3 -> ℝ` is now a function of its subscript too (`D_i(u_i)` calls `D(i, u_i)`), as subscripted local functions already were; sequences of functions are no longer in the language. Three bugs in the generated code are fixed there too: function parameters called in local functions, parameters mistaken for another local function's, and misplaced comments.

**The `markdown` submodule is H❤️rtDown's fork, rebased onto upstream Python-Markdown 3.11.2.**
- The fork was Python-Markdown 3.3.4 plus about 170 H❤️rtDown commits. Its `heartdown` branch replays those commits onto 3.11.2 and ports the extensions to the 3.4+ API.
- The output for every gallery paper is byte-identical to the old fork's.
- Upstream's test suite fails only 4 tests (the old fork failed 28). All 4 come from one deliberate change: `meta` parses YAML front matter.
- The old fork is on the `heartdown-py3` branch.
- The core changes are small: `Markdown` carries H❤️rtDown's state (`path`, `lib_py`, `json_data`, …), and preprocessors accept `path`.

**Dependencies are in `pyproject.toml`, managed with uv; the two submodules are editable path sources.**
- `iheartla` and `markdown` are submodules in `deps/` and are listed in `[tool.uv.sources]` as editable paths. So they are imported as installed packages (`iheartla.*`, `markdown.*`), while edits to them take effect immediately and are committed in the submodule, as in this repository's history.
- The alternative was git dependencies pinned to commits: a clone wouldn't need `--recursive`, but every change to the compiler or the fork would need a local path override that mustn't be committed. Both are changed often enough alongside H❤️rtDown to make that the worse trade. Switching later is a two-line change in `[tool.uv.sources]`.
- The submodules are in `deps/`, not at the top level, because a `markdown/` directory next to `app.py` would shadow the installed `markdown` package.
- (Before, `app.py` imported them through the repository layout, as `iheartla.iheartla.*` and `markdown.markdown.*`.)

**`server.py` serves files; it doesn't compile.** It restores the old `python server.py paper.md` workflow for any browser, with only the standard library. Compiling natively on the server, as the old one did, would be faster and could use any installed package in figures. But it would give the editor a second compile path, with its own errors and caching, and it would no longer match the editor served from anywhere else.

**Serve the preview with a service worker, not `srcdoc` or blob URLs.**
- The generated HTML refers to files by relative paths, and so do `paper.js`, MathJax's `attr-label` loader, figure iframes and the paper's own HTML.
- A service worker serves them all unchanged, with the editor's origin, so `paper.js`'s `parent.*` calls still work.
- Service workers need `https://` or `http://localhost`, so the editor needs a web server: `server.py`, or any static server.

**Figures run inside Pyodide's interpreter.** Pyodide has no subprocesses or threads.
- `run_figure_script` runs each script with `runpy` in the paper's folder.
- It reloads `lib` so the script sees the latest `lib.py`, closes Matplotlib figures afterwards, and returns the traceback as the figure's error.
- On the command line, the script runs in a subprocess, as before, with the same Python (`sys.executable`).

**One worker, mirrored files.**
- The worker keeps Pyodide and the compile caches across compiles, and receives only the files that changed.
- Opening another paper resets the mirror and the figure caches.

## Limitations

- The generated `paper.html` still loads MathJax, Popper and Tippy from CDNs. The page is H❤️rtDown's output, not part of the editor, and is meant to be viewed online.
- Without `server.py`, opening a folder on disk works only in Chromium browsers. Other browsers use browser storage and downloads.
- The editor itself also loads MathJax from a CDN, so neither it nor the generated papers work fully offline.
- A file written by a compile is detected by its modification time and size. A file rewritten in the same millisecond with the same size would be missed. The paper's outputs are always written during the compile, after the snapshot is taken.
