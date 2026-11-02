// Runs H❤️rtDown (app.py, its Markdown extensions and the I❤️LA compiler) in
// Pyodide, off the main thread. Pyodide needs a module worker:
// new Worker(url, { type: "module" }).
//
// The paper's folder is mirrored at PAPER_DIR in Pyodide's file system. Each
// request carries the files that changed in the editor since the last one; each
// reply carries the files the compile or figure run wrote.
//
// Messages, main -> worker:
//   {id, type: "compile", name, source, files: {path: Uint8Array}, removed: [path]}
//   {id, type: "figure", name, source, files, removed}   (run one figure's code)
//   {id, type: "reset"}   (another paper is opened: forget this one's files)
// worker -> main:
//   {ready: true} once (or {ready: false, error})
//   {status: text} while loading packages
//   {id, result, written: {path: Uint8Array}} or {id, error}
//   compile result: app.compile_paper()'s dict; figure result: {ok, msg}

import { loadPyodide } from "./pyodide/pyodide.mjs";

const local = new URL("pyodide/", import.meta.url).href;
const bundle = new URL("../heartdown-py.zip", import.meta.url).href;
const APP_DIR = "/heartdown";
const PAPER_DIR = "/paper";
// Generated code imports these; figure code usually does too. Loaded on first use.
const FIGURE_PACKAGES = ["numpy", "scipy", "matplotlib"];
// What modules need for what figures usually do but don't require, which Pyodide provides
const OPTIONAL_PACKAGES = { "plotly.express": ["pandas"] };  // without a DataFrame

const status = (text) => self.postMessage({ status: text });

const ready = (async () => {
    const pyodide = await loadPyodide({ indexURL: local });
    status("Loading the compiler…");
    await pyodide.loadPackage(["micropip", "regex", "sympy", "pyyaml"]);
    pyodide.globals.set("wheels", [local + "appdirs-1.4.4-py2.py3-none-any.whl",
                                   local + "tatsu-5.14.0-py3-none-any.whl",
                                   local + "latexcodec-3.0.1-py3-none-any.whl",
                                   local + "pybtex-0.26.1-py3-none-any.whl"]);
    await pyodide.runPythonAsync("import micropip\nawait micropip.install(wheels.to_py(), deps=False)");
    const zip = await (await fetch(bundle)).arrayBuffer();
    pyodide.unpackArchive(zip, "zip", { extractDir: APP_DIR });
    pyodide.FS.mkdirTree(PAPER_DIR);
    pyodide.runPython(`
import os, sys, logging
sys.path.insert(0, "${APP_DIR}")
os.chdir("${APP_DIR}")
import app
from iheartla.la_tools.la_logger import LaLogger
LaLogger.getInstance().set_level(logging.ERROR)
`);
    return pyodide;
})();
ready.then(() => self.postMessage({ ready: true }),
           e => self.postMessage({ ready: false, error: "Failed to load H❤️rtDown: " + e.message }));

let figurePackagesLoaded = false;
async function loadFigurePackages(pyodide, code) {
    if (!figurePackagesLoaded) {
        status("Loading NumPy, SciPy and Matplotlib for figures…");
        await pyodide.loadPackage(FIGURE_PACKAGES);
        pyodide.runPython("import matplotlib\nmatplotlib.use('Agg')");
        figurePackagesLoaded = true;
    }
    // Anything else the figure code imports that Pyodide provides (e.g. skimage)
    status("Loading the packages figures import…");
    await pyodide.loadPackagesFromImports(code);
    pyodide.globals.set("code", code);
    const imports = pyodide.runPython("from pyodide.code import find_imports\nfind_imports(code)").toJs();
    const optional = imports.flatMap(m => OPTIONAL_PACKAGES[m] || []);
    if (optional.length) await pyodide.loadPackage(optional);
    // and what Pyodide doesn't provide, if it's on PyPI as pure Python (e.g. plotly). lib and the paper's own modules are in PAPER_DIR.
    const missing = pyodide.runPython(`
import importlib.util
[m for m in sorted({m.split(".")[0] for m in find_imports(code)})
 if m != "lib" and importlib.util.find_spec(m) is None
 and not os.path.exists(f"${PAPER_DIR}/{m}.py") and not os.path.isdir(f"${PAPER_DIR}/{m}")]
`).toJs();
    for (const name of missing) {
        status(`Installing ${name} from PyPI…`);
        pyodide.globals.set("name", name);
        // If it can't be installed, running the figure code reports the ImportError
        await pyodide.runPythonAsync("import micropip\ntry:\n    await micropip.install(name)\nexcept Exception:\n    pass");
    }
}

// The ```python blocks inside <figure>s, which app.py runs
function figureCode(source) {
    const code = [];
    for (const figure of source.matchAll(/<figure[^\n>]*>([\s\S]*?)<\/figure>/g)) {
        for (const block of figure[1].matchAll(/^(~{3,}|`{3,})[ ]*python\s*\n([\s\S]*?\n)\1[ ]*$/gm)) {
            code.push(block[2]);
        }
    }
    return code.join("\n");
}

function syncFiles(pyodide, files, removed) {
    const FS = pyodide.FS;
    for (const path of removed || []) {
        try { FS.unlink(`${PAPER_DIR}/${path}`); } catch (e) { /* already gone */ }
    }
    for (const [path, bytes] of Object.entries(files || {})) {
        const full = `${PAPER_DIR}/${path}`;
        FS.mkdirTree(full.substring(0, full.lastIndexOf("/")));
        FS.writeFile(full, bytes);
    }
}

// path -> modification time, for every file under PAPER_DIR
function snapshot(pyodide) {
    const FS = pyodide.FS;
    const times = new Map();
    const walk = (dir, prefix) => {
        for (const name of FS.readdir(dir)) {
            if (name === "." || name === ".." || name === "__pycache__") continue;
            const full = `${dir}/${name}`;
            const stat = FS.stat(full);
            if (FS.isDir(stat.mode)) walk(full, `${prefix}${name}/`);
            else times.set(`${prefix}${name}`, stat.mtime.getTime() + ":" + stat.size);
        }
    };
    walk(PAPER_DIR, "");
    return times;
}

function written(pyodide, before) {
    const out = {};
    for (const [path, time] of snapshot(pyodide)) {
        if (before.get(path) !== time) out[path] = pyodide.FS.readFile(`${PAPER_DIR}/${path}`);
    }
    return out;
}

async function compile(pyodide, { name, source }) {
    const code = figureCode(source);
    if (code) await loadFigurePackages(pyodide, code);
    status("Compiling…");
    pyodide.globals.set("source", source);
    pyodide.globals.set("name", name);
    const result = pyodide.runPython(`app.compile_paper(source, "${PAPER_DIR}", name)`);
    const js = result.toJs({ dict_converter: Object.fromEntries });
    result.destroy();
    return js;
}

async function runFigure(pyodide, { name, source }) {
    await loadFigurePackages(pyodide, source);
    status("Running the figure code…");
    pyodide.globals.set("source", source);
    pyodide.globals.set("name", name);
    const result = pyodide.runPython(`app.run_figure(name, source, "${PAPER_DIR}")`);
    const [ok, msg] = result.toJs();
    result.destroy();
    return { ok, msg };
}

self.onmessage = async (event) => {
    const request = event.data;
    const { id } = request;
    let pyodide;
    try {
        pyodide = await ready;
    } catch (e) {
        self.postMessage({ id, error: "H❤️rtDown failed to load: " + e.message });
        return;
    }
    try {
        if (request.type === "reset") {
            pyodide.runPython(`
import shutil
shutil.rmtree("${PAPER_DIR}")
os.mkdir("${PAPER_DIR}")
app.cached_figure_dict = {}
app.cached_figure_code = {}
`);
            self.postMessage({ id, result: true, written: {} });
            return;
        }
        syncFiles(pyodide, request.files, request.removed);
        const before = snapshot(pyodide);
        const result = request.type === "figure" ? await runFigure(pyodide, request)
                                                 : await compile(pyodide, request);
        const out = written(pyodide, before);
        self.postMessage({ id, result, written: out }, Object.values(out).map(b => b.buffer));
    } catch (e) {
        console.log(e);
        self.postMessage({ id, error: String(e.message).split("\n").slice(-3).join("\n") });
    }
};
