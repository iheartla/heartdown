// The open paper's folder: every file, in memory, by its path relative to the folder.
// Three ways to keep it:
//   - "folder": a folder on disk, opened with the File System Access API
//     (Chromium). Writes go straight to disk.
//   - "local": a folder on disk, through server.py, which serves the editor
//     for one paper (any browser). Writes go to disk through the server.
//   - "browser": IndexedDB. Files come in by opening or dropping them, and go
//     out by downloading the paper or a zip of the folder.
// Either way, the folder is served to the preview by sw.js (see serveFile) and
// mirrored into the Pyodide worker (see takeChanges).

const DB_NAME = "heartdown";
const SKIP_DIRS = new Set([".git", "__pycache__", "node_modules", "input_history", ".venv", ".idea"]);
const MAX_FILE_SIZE = 50 * 1024 * 1024;

function openDB() {
    return new Promise((resolve, reject) => {
        const request = indexedDB.open(DB_NAME, 1);
        request.onupgradeneeded = () => {
            request.result.createObjectStore("files");   // path -> Uint8Array (browser projects)
            request.result.createObjectStore("meta");    // "project" -> {mode, name, mdPath, handle}
        };
        request.onsuccess = () => resolve(request.result);
        request.onerror = () => reject(request.error);
    });
}

async function idb(store, mode, fn) {
    const db = await openDB();
    return new Promise((resolve, reject) => {
        const tx = db.transaction(store, mode);
        const result = fn(tx.objectStore(store));
        tx.oncomplete = () => resolve(result.result !== undefined ? result.result : result);
        tx.onerror = () => reject(tx.error);
    });
}

const encoder = new TextEncoder();
const decoder = new TextDecoder();

class Project {
    constructor(mode, name, files, handle = null) {
        this.mode = mode;          // "folder", "local" or "browser"
        this.name = name;          // folder name, for display
        this.files = files;        // Map: path -> Uint8Array
        this.handle = handle;      // FileSystemDirectoryHandle in folder mode
        this.mdPath = null;        // the paper, e.g. "paper.md"
        // Paths changed or removed here since the worker last got them
        this.changed = new Set(files.keys());
        this.removed = new Set();
        // Identifies this editor tab's files to sw.js
        this.session = Math.random().toString(36).slice(2);
    }

    static canOpenFolder() {
        return "showDirectoryPicker" in window;
    }

    get paperName() {  // "paper" for "paper.md"
        const base = this.mdPath.slice(this.mdPath.lastIndexOf("/") + 1);
        return base.slice(0, base.lastIndexOf("."));
    }

    mdFiles() {
        return [...this.files.keys()].filter(p => /\.(md|txt)$/i.test(p) && !p.includes("/")).sort();
    }

    read(path) {
        return this.files.get(path);
    }

    readText(path) {
        const bytes = this.files.get(path);
        return bytes === undefined ? undefined : decoder.decode(bytes);
    }

    // fromWorker: the file came from the worker, which already has it
    async write(path, data, fromWorker = false) {
        const bytes = typeof data === "string" ? encoder.encode(data) : data;
        this.files.set(path, bytes);
        if (!fromWorker) this.changed.add(path);
        this.removed.delete(path);
        if (this.mode === "folder") {
            await writeToFolder(this.handle, path, bytes);
        } else if (this.mode === "local") {
            const response = await fetch(localFileURL(path), { method: "PUT", body: bytes });
            if (!response.ok) throw new Error(`Couldn't save ${path}: ${response.status} ${response.statusText}`);
        } else {
            await idb("files", "readwrite", store => store.put(bytes, path));
        }
    }

    // The files the worker needs to mirror this folder, since the last call
    takeChanges() {
        const files = {};
        for (const path of this.changed) {
            if (this.files.has(path)) files[path] = this.files.get(path);
        }
        const removed = [...this.removed];
        this.changed.clear();
        this.removed.clear();
        return { files, removed };
    }

    // Make the worker get every file again (e.g., after it restarts)
    resync() {
        this.changed = new Set(this.files.keys());
    }

    previewURL(path) {
        return new URL(`paper/${this.session}/${path.split("/").map(encodeURIComponent).join("/")}`,
                       document.baseURI).href;
    }

    async remember() {
        await idb("meta", "readwrite", store => store.put(
            { mode: this.mode, name: this.name, mdPath: this.mdPath, handle: this.handle }, "project"));
    }
}

async function writeToFolder(dir, path, bytes) {
    const parts = path.split("/");
    for (const part of parts.slice(0, -1)) {
        dir = await dir.getDirectoryHandle(part, { create: true });
    }
    const file = await dir.getFileHandle(parts[parts.length - 1], { create: true });
    const writable = await file.createWritable();
    await writable.write(bytes);
    await writable.close();
}

async function readFolder(dir, prefix = "", files = new Map()) {
    for await (const [name, handle] of dir.entries()) {
        if (handle.kind === "directory") {
            if (!SKIP_DIRS.has(name)) await readFolder(handle, `${prefix}${name}/`, files);
        } else if (name !== ".DS_Store") {
            const file = await handle.getFile();
            if (file.size <= MAX_FILE_SIZE) {
                files.set(`${prefix}${name}`, new Uint8Array(await file.arrayBuffer()));
            }
        }
    }
    return files;
}

// Open a folder on disk (File System Access API). Returns null if cancelled.
async function openFolderProject(handle = null) {
    if (handle === null) {
        try {
            handle = await window.showDirectoryPicker({ id: "heartdown", mode: "readwrite" });
        } catch (e) {
            if (e.name === "AbortError") return null;
            throw e;
        }
    } else if (await handle.queryPermission({ mode: "readwrite" }) !== "granted" &&
               await handle.requestPermission({ mode: "readwrite" }) !== "granted") {
        return null;
    }
    return new Project("folder", handle.name, await readFolder(handle), handle);
}

function localFileURL(path) {
    return "local/file/" + path.split("/").map(encodeURIComponent).join("/");
}

// The paper server.py was started with, or null if the editor isn't served by server.py
async function openLocalProject() {
    let info;
    try {
        const response = await fetch("local/paper", { cache: "no-store" });
        if (!response.ok || !(response.headers.get("Content-Type") || "").includes("json")) return null;
        info = await response.json();
    } catch (e) {
        return null;
    }
    const { files: paths } = await (await fetch("local/files", { cache: "no-store" })).json();
    const files = new Map();
    await Promise.all(paths.map(async (path) => {
        const response = await fetch(localFileURL(path), { cache: "no-store" });
        if (response.ok) files.set(path, new Uint8Array(await response.arrayBuffer()));
    }));
    const project = new Project("local", info.folder, files);
    project.mdPath = info.paper;
    return project;
}

// Files from an <input type=file> (multiple or a folder) or a drop, into a browser project
async function openFilesProject(fileList) {
    const files = new Map();
    let name = "";
    for (const file of fileList) {
        let path = file.webkitRelativePath || file.relativePath || file.name;
        // A chosen folder's files come as "folder/…": keep paths relative to it
        if (path.includes("/")) {
            const top = path.slice(0, path.indexOf("/"));
            name = name || top;
            path = path.slice(top.length + 1);
        }
        if (path.split("/").some(part => SKIP_DIRS.has(part)) || file.size > MAX_FILE_SIZE) continue;
        files.set(path, new Uint8Array(await file.arrayBuffer()));
    }
    const project = new Project("browser", name || "browser", files);
    await saveBrowserProject(project);
    return project;
}

async function newBrowserProject(mdPath, source) {
    const project = new Project("browser", "browser", new Map([[mdPath, encoder.encode(source)]]));
    project.mdPath = mdPath;
    await saveBrowserProject(project);
    return project;
}

async function saveBrowserProject(project) {
    await idb("files", "readwrite", store => {
        store.clear();
        for (const [path, bytes] of project.files) store.put(bytes, path);
        return store;
    });
}

// The project open last time, or null. A folder needs permission again, which
// needs a click, so it comes back as {reopen: handle, ...} for a button to call.
async function restoreProject() {
    const meta = await idb("meta", "readonly", store => store.get("project"));
    if (!meta) return null;
    if (meta.mode === "folder") {
        if (!meta.handle) return null;
        if (await meta.handle.queryPermission({ mode: "readwrite" }) === "granted") {
            const project = await openFolderProject(meta.handle);
            project.mdPath = meta.mdPath;
            return project;
        }
        return { reopen: meta.handle, mdPath: meta.mdPath };
    }
    const files = new Map();
    await idb("files", "readonly", store => {
        const request = store.openCursor();
        request.onsuccess = () => {
            const cursor = request.result;
            if (cursor) { files.set(cursor.key, cursor.value); cursor.continue(); }
        };
        return store;
    });
    if (!files.has(meta.mdPath)) return null;
    const project = new Project("browser", meta.name, files);
    project.mdPath = meta.mdPath;
    return project;
}

// Answer sw.js's requests for this tab's files
function serveProjectFiles(getProject) {
    navigator.serviceWorker.addEventListener("message", (event) => {
        const { type, session, path } = event.data || {};
        if (type !== "heartdown-file") return;
        const project = getProject();
        if (!project || project.session !== session) {
            event.ports[0].postMessage(null);
            return;
        }
        const bytes = project.read(path);
        event.ports[0].postMessage(bytes ? { found: true, bytes } : { found: false });
    });
}

// A zip (stored, not compressed) of the project's files
function zipProject(project) {
    const crcTable = new Uint32Array(256).map((_, n) => {
        let c = n;
        for (let k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1;
        return c >>> 0;
    });
    const crc32 = (bytes) => {
        let c = 0xFFFFFFFF;
        for (const b of bytes) c = crcTable[(c ^ b) & 0xFF] ^ (c >>> 8);
        return (c ^ 0xFFFFFFFF) >>> 0;
    };
    const parts = [], central = [];
    let offset = 0;
    for (const [path, bytes] of project.files) {
        const name = encoder.encode(path);
        const crc = crc32(bytes);
        const header = new DataView(new ArrayBuffer(30));
        header.setUint32(0, 0x04034b50, true);
        header.setUint16(4, 20, true);
        header.setUint16(6, 0x0800, true);   // UTF-8 names
        header.setUint32(14, crc, true);
        header.setUint32(18, bytes.length, true);
        header.setUint32(22, bytes.length, true);
        header.setUint16(26, name.length, true);
        parts.push(header, name, bytes);
        const entry = new DataView(new ArrayBuffer(46));
        entry.setUint32(0, 0x02014b50, true);
        entry.setUint16(4, 20, true);
        entry.setUint16(6, 20, true);
        entry.setUint16(8, 0x0800, true);
        entry.setUint32(16, crc, true);
        entry.setUint32(20, bytes.length, true);
        entry.setUint32(24, bytes.length, true);
        entry.setUint16(28, name.length, true);
        entry.setUint32(42, offset, true);
        central.push(entry, name);
        offset += 30 + name.length + bytes.length;
    }
    const centralSize = central.reduce((n, p) => n + p.byteLength, 0);
    const end = new DataView(new ArrayBuffer(22));
    end.setUint32(0, 0x06054b50, true);
    end.setUint16(8, project.files.size, true);
    end.setUint16(10, project.files.size, true);
    end.setUint32(12, centralSize, true);
    end.setUint32(16, offset, true);
    return new Blob([...parts, ...central, end], { type: "application/zip" });
}

function download(blob, filename) {
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = filename;
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 10000);
}
