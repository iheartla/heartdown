"""
Run the H❤️rtDown editor on this computer, on a paper on disk, in any browser:

    python server.py path/to/paper.md     (or: uv run heartdown-server path/to/paper.md)

This serves the editor in docs/ and gives it the paper's folder: the editor
reads the folder's files and writes the paper and what compiling generates
back to it. Compiling still happens in the browser, as when the editor is
served from anywhere else. Only Python's standard library is needed.

The editor finds the folder through these requests (see docs/js/project.js):
    GET /local/paper          {"folder": folder name, "paper": the paper's path in it}
    GET /local/files          {"files": [path, ...]}, every file in the folder
    GET /local/file/<path>    a file
    PUT /local/file/<path>    write a file
"""
import argparse
import json
import shutil
import sys
import webbrowser
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote, urlparse

DOCS = Path(__file__).resolve().parent / "docs"
STARTER = DOCS / "examples" / "starter.md"
# As in docs/js/project.js
SKIP_DIRS = {".git", "__pycache__", "node_modules", "input_history", ".venv", ".idea"}
MAX_FILE_SIZE = 50 * 1024 * 1024


class EditorHandler(SimpleHTTPRequestHandler):
    folder = None   # the paper's folder (Path)
    paper = None    # the paper's path in it (str)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(DOCS), **kwargs)

    def end_headers(self):
        # The editor and the paper change while it runs
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def local_host(self):
        # Only answer pages from this computer (e.g., not a site that rebinds its DNS to 127.0.0.1)
        host = self.headers.get("Host", "").rsplit(":", 1)[0]
        return host in ("localhost", "127.0.0.1", "[::1]")

    def file_path(self, path):
        """The file for /local/file/<path>, or None if it is outside the folder"""
        target = (self.folder / unquote(path)).resolve()
        return target if target.is_relative_to(self.folder) and target != self.folder else None

    def send_json(self, data):
        body = json.dumps(data).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        path = urlparse(self.path).path
        if not path.startswith("/local/"):
            return super().do_GET()
        if not self.local_host():
            return self.send_error(403)
        if path == "/local/paper":
            return self.send_json({"folder": self.folder.name, "paper": self.paper})
        if path == "/local/files":
            return self.send_json({"files": list_files(self.folder)})
        if path.startswith("/local/file/"):
            target = self.file_path(path[len("/local/file/"):])
            if target is None or not target.is_file():
                return self.send_error(404)
            body = target.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        self.send_error(404)

    def do_PUT(self):
        # PUT, unlike POST, can't be sent by another site's page without CORS approval, which this never gives
        path = urlparse(self.path).path
        if not self.local_host():
            return self.send_error(403)
        target = self.file_path(path[len("/local/file/"):]) if path.startswith("/local/file/") else None
        if target is None:
            return self.send_error(404)
        body = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(body)
        self.send_response(204)
        self.end_headers()

    def log_message(self, format, *args):
        pass


def list_files(folder):
    files = []
    for path in sorted(folder.rglob("*")):
        relative = path.relative_to(folder)
        if (path.is_file() and path.name != ".DS_Store" and not SKIP_DIRS.intersection(relative.parts)
                and path.stat().st_size <= MAX_FILE_SIZE):
            files.append(relative.as_posix())
    return files


def main():
    arg_parser = argparse.ArgumentParser(description="Run the H❤️rtDown editor on a paper on this computer")
    arg_parser.add_argument("paper", help="the paper (.md); a new one is created from the example if it doesn't exist")
    arg_parser.add_argument("--port", type=int, default=8000, help="the port to serve the editor on (default: 8000)")
    arg_parser.add_argument("--no-browser", action="store_true", help="don't open the editor in a browser")
    args = arg_parser.parse_args()

    paper = Path(args.paper).resolve()
    if not paper.exists():
        paper.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(STARTER, paper)
        print("Created {} from the example paper".format(paper))
    EditorHandler.folder = paper.parent
    EditorHandler.paper = paper.name

    try:
        server = ThreadingHTTPServer(("127.0.0.1", args.port), EditorHandler)
    except OSError as e:
        sys.exit("Can't serve on port {}: {}. Try another with --port.".format(args.port, e))
    url = "http://localhost:{}/".format(server.server_address[1])
    print("Editing {}\nThe editor is at {} (Ctrl-C to stop)".format(paper, url))
    if not args.no_browser:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
