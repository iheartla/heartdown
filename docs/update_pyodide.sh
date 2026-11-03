# Vendor the Pyodide runtime and the Python packages H❤️rtDown needs into
# docs/js/pyodide/, so the editor doesn't depend on a server, a CDN or PyPI.
# Usage: sh docs/update_pyodide.sh   (then rebuild the Python bundle: sh docs/update_python.sh)
#
# Vendored: what compiling a paper needs (the Markdown pipeline; the I❤️LA
# compiler is WebAssembly, in heartdown-py.zip) and what figure code usually needs (NumPy, SciPy, Matplotlib; the
# generated lib.py imports NumPy and SciPy). Any other Pyodide package a figure
# imports (e.g. scikit-image) stays in pyodide-lock.json with its CDN URL, so it
# loads from the CDN on first use.
set -e
PYODIDE=314.0.7
CDN="https://cdn.jsdelivr.net/pyodide/v$PYODIDE/full/"
# pure-Python wheels from PyPI
PYBTEX=0.26.1
LATEXCODEC=3.0.1
cd "$(dirname "$0")/js"
rm -rf pyodide && mkdir pyodide
# Runtime only (~14 MB); the full distribution with every package is ~320 MB compressed.
curl -fsSL "https://github.com/pyodide/pyodide/releases/download/$PYODIDE/pyodide-core-$PYODIDE.tar.bz2" |
    tar -xj -C pyodide --strip-components=1 pyodide/pyodide.js pyodide/pyodide.mjs pyodide/pyodide.asm.mjs \
        pyodide/pyodide.asm.wasm pyodide/python_stdlib.zip pyodide/pyodide-lock.json
# Download the vendored packages with their dependencies, and point the lock
# file's other packages at the CDN.
for f in $(CDN="$CDN" python3 - <<'PY'
import json, os
lock = json.load(open("pyodide/pyodide-lock.json"))
pkgs = lock["packages"]
need, todo = set(), ["micropip", "regex", "pyyaml", "numpy", "scipy", "matplotlib"]
while todo:
    n = todo.pop()
    if n not in need:
        need.add(n)
        todo += pkgs[n].get("depends", [])
for name, pkg in pkgs.items():
    if name not in need:
        pkg["file_name"] = os.environ["CDN"] + pkg["file_name"]
json.dump(lock, open("pyodide/pyodide-lock.json", "w"))
print("\n".join(sorted(pkgs[n]["file_name"] for n in need)))
PY
); do
    curl -fsSL -o "pyodide/$f" "$CDN$f"
done
pypi() {  # wheel file name -> pyodide/
    curl -fsSL -o "pyodide/$1" "https://files.pythonhosted.org/packages/$2/$(echo "$1" | cut -c1)/$(echo "$1" | cut -d- -f1)/$1"
}
pypi pybtex-$PYBTEX-py3-none-any.whl py3
pypi latexcodec-$LATEXCODEC-py3-none-any.whl py3
du -sh pyodide
