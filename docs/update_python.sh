# Bundle H❤️rtDown's Python code for the in-browser editor into docs/heartdown-py.zip:
# app.py, the Markdown fork, the I❤️LA compiler (iheartla submodule) and
# extras/heartdown-resource, laid out as in this repository. The editor's
# Pyodide worker (docs/js/heartdown-worker.js) unpacks it and imports app.py.
# Run it after changing any of them (or updating the submodules).
# Usage: sh docs/update_python.sh
set -e
cd "$(dirname "$0")/.."
python3 - <<'PY'
import os, zipfile
SKIP_DIRS = {"__pycache__", "la_gui", "tests", "test"}
def add_tree(z, root, keep):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = sorted(d for d in dirnames if d not in SKIP_DIRS and not d.startswith("."))
        for f in sorted(filenames):
            if keep(f):
                z.write(os.path.join(dirpath, f))
with zipfile.ZipFile("docs/heartdown-py.zip", "w", zipfile.ZIP_DEFLATED) as z:
    z.write("app.py")
    z.write("markdown/__init__.py")
    add_tree(z, "markdown/markdown", lambda f: f.endswith(".py"))
    # the compiler, with its bundled I❤️MESH modules (.hlang)
    add_tree(z, "iheartla/iheartla", lambda f: f.endswith((".py", ".hlang")))
    add_tree(z, "extras/heartdown-resource", lambda f: not f.startswith("."))
PY
du -h docs/heartdown-py.zip
