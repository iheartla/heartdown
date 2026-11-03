# Bundle H❤️rtDown's Python code for the in-browser editor into docs/heartdown-py.zip:
# app.py and extras/heartdown-resource, and the packages from the submodules,
# iheartla/ (the I❤️LA compiler) and markdown/ (H❤️rtDown's fork of
# Python-Markdown), at the top level as if installed. The editor's Pyodide
# worker (docs/js/heartdown-worker.js) unpacks it and imports app.py.
# Run it after changing any of them (or updating the submodules).
# Usage: sh docs/update_python.sh
set -e
cd "$(dirname "$0")/.."
python3 - <<'PY'
import os, zipfile
SKIP_DIRS = {"__pycache__", "la_gui", "tests", "test"}
def add_tree(z, root, keep, prefix=""):
    # prefix: the directory in the repository that root is in, left out of the zip
    for dirpath, dirnames, filenames in os.walk(os.path.join(prefix, root)):
        dirnames[:] = sorted(d for d in dirnames if d not in SKIP_DIRS and not d.startswith("."))
        for f in sorted(filenames):
            if keep(f):
                path = os.path.join(dirpath, f)
                z.write(path, os.path.relpath(path, prefix or "."))
with zipfile.ZipFile("docs/heartdown-py.zip", "w", zipfile.ZIP_DEFLATED) as z:
    z.write("app.py")
    add_tree(z, "markdown", lambda f: f.endswith(".py"), "deps/markdown")
    # the compiler (the Rust compiler as WebAssembly, rust/iheartla.wasm, which
    # iheartla/rust_heartdown.py runs), with its bundled I❤️MESH modules (.hlang)
    add_tree(z, "iheartla", lambda f: f.endswith((".py", ".hlang", ".wasm")), "deps/iheartla")
    add_tree(z, "extras/heartdown-resource", lambda f: not f.startswith("."))
PY
du -h docs/heartdown-py.zip
