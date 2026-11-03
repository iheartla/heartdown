let unicode_dict = {'R': 'ℝ', 'Z': 'ℤ', 'x': '×', 'times': '×', 'inf': '∞', 'in': '∈', 'sum': '∑',
                             'had': '∘', 'kro': '⊗', 'dot': '⋅', 'T': 'ᵀ', '^T': 'ᵀ', 'par': '∂', 'emp': '∅',
                             'arr': '→', 'int': '∫', 'dbl': '‖', '|': '‖', 'pi': 'π', 'sig': 'σ', 'rho': 'ρ',
                             'phi': 'ϕ', 'theta': 'θ', 'alpha': 'α', 'beta': 'β', 'gamma': 'γ', 'delta':'δ', 'epsilon': 'ε',
                             'zeta':'ζ', 'eta':'η', 'iota':'ι', 'kappa':'κ', 'lambda':'λ', 'mu':'μ', 'nu':'ν', 'xi':'ξ', 'omicron':'ο',
                             'sigma':'σ', 'tau':'τ', 'upsilon':'υ', 'chi':'χ', 'psi':'ψ', 'omega':'ω',
                             'u0': '₀', 'u1': '₁', 'u2': '₂', 'u3': '₃', 'u4': '₄', 'u5': '₅', 'u6': '₆', 'u7': '₇', 'u8': '₈', 'u9': '₉',
                             '_0': '₀', '_1': '₁', '_2': '₂', '_3': '₃', '_4': '₄', '_5': '₅', '_6': '₆', '_7': '₇', '_8': '₈', '_9': '₉',
                             's0': '⁰', 's1': '¹', 's2': '²', 's3': '³', 's4': '⁴', 's5': '⁵', 's6': '⁶', 's7': '⁷', 's8': '⁸', 's9': '⁹', 's-1': '⁻¹', '^-1': '⁻¹',
                             '^0': '⁰', '^1': '¹', '^2': '²', '^3': '³', '^4': '⁴', '^5': '⁵', '^6': '⁶', '^7': '⁷', '^8': '⁸', '^9': '⁹',
                             '_a': 'ₐ', '_e': 'ₑ', '_h': 'ₕ', '_i': 'ᵢ', '_j': 'ⱼ', '_k': 'ₖ',
                             '_l': 'ₗ', '_m': 'ₘ', '_n': 'ₙ', '_o': 'ₒ', '_p': 'ₚ', '_s': 'ₛ', '_t': 'ₜ', '_u': 'ᵤ',
                             '_v': 'ᵥ', '_x': 'ₓ', '1': '𝟙', 'cdot': '⋅', 'nabla': '∇',
                             'sqrt': '√', '+-': '±', '<=': '≤', '<=>': '⇔', '>=': '≥', '1/2': '½',
                             '1/3': '⅓', '1/4': '¼', '1/5': '⅕', '1/6': '⅙', '1/7': '⅐', '1/8': '⅛', '1/9': '⅑', '1/10': '⅒',
                             '2/3': '⅔', '2/5': '⅖', '3/4': '¾', '3/5': '⅗', '3/8': '⅜', '4/5': '⅘', '5/6': '⅚', '5/8': '⅝', '7/8': '⅞',
                             'heart': '❤️', 'iheartla': 'I❤️LA',
                             'le':'≤', 'ge':'≥', 'ne': '≠', 'notin':'∉', 'div':'÷', 'nplus': '±',
                             'linner': '⟨', 'rinner':'⟩', 'num1': '𝟙',
                             'hat': '\u0302', 'bar': '\u0304'
                             };
let preEqCode = '';
let preTestCode = '';
let curFigure = '';
let curFileName = '';
let msgTimeout = null;
let latestSource = '';
let project = null;        // the open paper's folder (project.js)
let reopenHandle = null;   // a folder from last time that needs permission again
let worker = null;         // Pyodide running H❤️rtDown (heartdown-worker.js)
let workerReady = false;
let compiling = false;
const pending = new Map(); // request id -> {resolve, reject}
let nextRequestId = 0;
const STARTER = "examples/starter.md";

// The editor needs a service worker (to serve the paper's files to the preview)
// and module workers, so a web server; any static one will do.
function checkBrowser() {
    if (window.location.protocol === "file:") {
        return "Please open the editor through a web server, e.g. run `python3 -m http.server` in docs/ and browse to http://localhost:8000/.";
    }
    if (!("serviceWorker" in navigator)) {
        return "This browser doesn't support service workers, which the editor needs. (They also need https:// or http://localhost.)";
    }
    return null;
}

function startWorker() {
    workerReady = false;
    worker = new Worker("./js/heartdown-worker.js", { type: "module" });
    worker.onmessage = (event) => {
        const data = event.data;
        if (data.ready !== undefined) {
            workerReady = data.ready;
            if (data.ready) {
                activateBtnStatus();
            } else {
                showMsg(escapeHtml(data.error), true);
                setBtnTitle("Failed to load");
            }
        } else if (data.status !== undefined) {
            if (compiling) setBtnTitle(data.status, true);
        } else if (pending.has(data.id)) {
            const { resolve, reject } = pending.get(data.id);
            pending.delete(data.id);
            if (data.error !== undefined) reject(new Error(data.error));
            else resolve(data);
        }
    };
    worker.onerror = (event) => {
        console.log(event);
        showMsg("H❤️rtDown failed to load: " + (event.message || "see the browser console"), true);
    };
}

// Send a request to the worker with the paper's changed files; save the files it wrote
async function ask(request) {
    if (request.type !== "reset") Object.assign(request, project.takeChanges());
    const id = nextRequestId++;
    const reply = await new Promise((resolve, reject) => {
        pending.set(id, { resolve, reject });
        worker.postMessage({ id, ...request });
    });
    for (const [path, bytes] of Object.entries(reply.written)) {
        await project.write(path, bytes, true);
    }
    return reply.result;
}

async function onLoad(){
    const problem = checkBrowser();
    if (problem) {
        showMsg(problem, true);
        setBtnTitle("Unavailable");
        return;
    }
    initBtnStatus();
    serveProjectFiles(() => project);
    startWorker();
    try {
        await navigator.serviceWorker.register("./sw.js");
        await navigator.serviceWorker.ready;
    } catch (e) {
        showMsg("Couldn't start the service worker for the preview: " + escapeHtml(e.message), true);
    }
    initDropTarget();
    window.addEventListener("pagehide", () => savePaper());
    window.addEventListener("hashchange", openHashLink);
    // The paper server.py serves comes first, then a paper in the link (see linkedPaper), then the paper from last time
    const local = await openLocalProject();
    if (local) {
        await setProject(local, false);
        return;
    }
    const linked = await linkedPaper();
    let restored = null;
    try {
        restored = await restoreProject();
    } catch (e) {
        console.log(e);
    }
    if (restored && !restored.reopen && restored.edited === null) {
        // kept by an older version of the editor: edited unless it's the example
        restored.edited = restored.readText(restored.mdPath) !== await (await fetch(STARTER)).text();
    }
    const replaceRestored = () => !hasEdits(restored) ||
        confirm("You've edited the paper kept in this browser. Replace it with the one in this link?");
    if (linked && linked.url !== undefined && replaceRestored()) {
        await openPaperURL(linked.url, restored);
    } else if (linked && linked.text !== undefined &&
               (restored && !restored.reopen && restored.mode === "browser" &&
                restored.readText(restored.mdPath) === linked.text || replaceRestored())) {
        await openPaperText(linked.text);
    } else if (restored && restored.reopen) {
        reopenHandle = restored.reopen;
        updateProjectInfo();
        await setProject(await newBrowserProject("paper.md", await (await fetch(STARTER)).text()), false);
        showMsg(`Click “Reopen ${escapeHtml(reopenHandle.name)}” to keep working on that folder.`);
    } else if (restored) {
        await setProject(restored);
    } else {
        await setProject(await newBrowserProject("paper.md", await (await fetch(STARTER)).text()));
    }
    if (linked && linked.error) {
        forgetLink();
        showMsg(escapeHtml(linked.error), true);
    }
}

// The paper in the link, or null: {url} for ?paper=<url> (a data: URL too), or {text} for the paper's text,
// from Share (#z=<the text, compressed>) or ?code=<the text> (older links), or {error} if it can't be read.
// Each may be in the query (?) or the fragment (#).
async function linkedPaper() {
    const url = new URL(window.location.href);
    const params = new URLSearchParams(url.hash.slice(1));
    for (const [key, value] of url.searchParams) {
        if (!params.has(key)) params.set(key, value);
    }
    if (params.has("z")) {
        try {
            return { text: await decompressText(params.get("z")) };
        } catch (e) {
            return { error: "The link's paper is damaged (it may have been cut off): " + e.message };
        }
    }
    if (params.has("code")) return { text: params.get("code") };
    let paper = params.get("paper");
    if (paper === null) return null;
    // A base64 data: URL that wasn't percent-encoded: its +s came as spaces
    if (/^data:[^,]*;base64,/i.test(paper)) paper = paper.replace(/ /g, "+");
    return { url: paper };
}

// A link pasted into the address bar of this page changes only its fragment
async function openHashLink() {
    const linked = await linkedPaper();
    if (linked === null) return;
    if (!confirmReplaceBrowserProject()) {
        forgetLink();
    } else if (linked.error) {
        forgetLink();
        showMsg(escapeHtml(linked.error), true);
    } else if (linked.url !== undefined) {
        await openPaperURL(linked.url);
    } else {
        await openPaperText(linked.text);
    }
}

// A new browser paper with this text, from a link
async function openPaperText(text) {
    await setProject(await newBrowserProject("paper.md", text));
    forgetLink();
    showMsg("Loaded the paper in the link. Compiling runs the paper's figure code in your browser, so compile only papers you trust.",
            false, true);
}

// Switch to another paper folder
async function setProject(newProject, remember = true) {
    // the last edits to the paper being closed
    await savePaper();
    if (project !== null && worker !== null) {
        await ask({ type: "reset" }).catch(e => console.log(e));
    }
    if (newProject.mdPath === null || !newProject.files.has(newProject.mdPath)) {
        newProject.mdPath = await choosePaper(newProject);
        if (newProject.mdPath === null) return;
    }
    project = newProject;
    project.resync();
    if (remember) {
        reopenHandle = null;
        await project.remember();
    }
    latestSource = project.readText(project.mdPath);
    let editor = ace.edit("editor");
    editor.setValue(latestSource, -1);
    curFileName = project.paperName;
    removeIframe();
    updateProjectInfo();
}

// The paper (.md) in a folder: the only one, one the user picks, or a new one
function choosePaper(newProject) {
    const papers = newProject.mdFiles();
    if (papers.length === 1) return Promise.resolve(papers[0]);
    if (papers.length === 0) {
        let name = prompt("This folder has no paper (.md) yet. Name of the new paper:", "paper.md");
        if (!name) return Promise.resolve(null);
        if (!/\.(md|txt)$/i.test(name)) name += ".md";
        return fetch(STARTER).then(r => r.text()).then(async text => {
            await newProject.write(name, text);
            return name;
        });
    }
    return new Promise((resolve) => {
        const list = document.getElementById("paperList");
        list.innerHTML = "";
        const modal = bootstrap.Modal.getOrCreateInstance(document.getElementById("choosePaper"));
        for (const path of papers) {
            const button = document.createElement("button");
            button.className = "list-group-item list-group-item-action";
            button.textContent = path;
            button.onclick = () => { modal.hide(); resolve(path); };
            list.appendChild(button);
        }
        document.getElementById("choosePaper").addEventListener("hidden.bs.modal", () => resolve(null), { once: true });
        modal.show();
    });
}

function updateProjectInfo() {
    const info = document.getElementById("projectInfo");
    if (project === null) {
        info.textContent = "";
    } else if (project.mode === "local") {
        info.innerHTML = `<i class="fa fa-hdd-o"></i> ${escapeHtml(project.name)}/${escapeHtml(project.mdPath)}`;
        info.title = "Saved in this folder on your computer, through server.py";
    } else if (project.mode === "folder") {
        info.innerHTML = `<i class="fa fa-folder-open"></i> ${escapeHtml(project.name)}/${escapeHtml(project.mdPath)}`;
        info.title = "Saved in this folder on your computer";
    } else {
        info.innerHTML = `<i class="fa fa-database"></i> ${escapeHtml(project.mdPath)} <span class="text-muted">(in this browser)</span>`;
        info.title = "Saved in this browser. Download the paper or a zip of all its files to keep them.";
    }
    document.getElementById("openFolder").hidden = !Project.canOpenFolder();
    const reopen = document.getElementById("reopenFolder");
    reopen.hidden = reopenHandle === null;
    if (reopenHandle !== null) reopen.innerHTML = `<i class="fa fa-folder-open"></i> Reopen ${escapeHtml(reopenHandle.name)}`;
}

function escapeHtml(text) {
    return text.replace(/[&<>"']/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
}

// The link's paper is now kept in the browser: reloading shouldn't load it again over any edits
function forgetLink() {
    const url = new URL(window.location.href);
    url.searchParams.delete("paper");
    url.searchParams.delete("code");
    url.searchParams.delete("z");
    url.hash = "";
    history.replaceState(null, "", url);
}

// Open the paper (.md or .txt) or zip of a paper's folder at url.
// loaded: the message once it's open (HTML), if not the warning for papers from anywhere
// files: other files to fetch with it (see openURLProject)
async function openPaperURL(url, fallback = null, loaded = null, files = []) {
    showMsg(`Loading ${escapeHtml(urlLabel(url))}…`, false, true);
    try {
        await setProject(await openURLProject(url, files));
        forgetLink();
        // Its figure code runs in this page when it's compiled
        showMsg(loaded || `Loaded ${escapeHtml(urlLabel(url))}. Compiling runs the paper's figure code in your browser, so compile only papers you trust.`,
                false, true);
    } catch (e) {
        if (project === null) {
            await setProject(fallback && !fallback.reopen ? fallback :
                             await newBrowserProject("paper.md", await (await fetch(STARTER)).text()));
        }
        // (after setProject, which clears messages)
        showMsg(escapeHtml(e.message), true);
    }
}

const DEMOS = new URL("examples/demos.json", document.baseURI);
let demos = null;  // examples/demos.json: the basic example, simple examples and the gallery's case studies

async function clickDemos() {
    if (demos === null) {
        try {
            demos = await (await fetch(DEMOS)).json();
        } catch (e) {
            showMsg("Couldn't load the list of demos: " + escapeHtml(e.message), true);
            return;
        }
        showDemos();
    }
    bootstrap.Modal.getOrCreateInstance(document.getElementById("demos")).show();
}

function showDemos() {
    const list = document.getElementById("demoList");
    document.getElementById("demoGallery").href = demos.gallery;
    for (const group of demos.groups) {
        const heading = document.createElement("h6");
        heading.className = "mt-2";
        heading.textContent = group.title;
        const items = document.createElement("div");
        items.className = "list-group mb-3";
        for (const demo of group.demos) {
            const button = document.createElement("button");
            button.type = "button";
            button.className = "list-group-item list-group-item-action";
            button.textContent = demo.title;
            button.onclick = () => openDemo(demo, group);
            items.appendChild(button);
        }
        list.append(heading, items);
    }
}

async function openDemo(demo, group) {
    bootstrap.Modal.getOrCreateInstance(document.getElementById("demos")).hide();
    if (!confirmReplaceBrowserProject()) return;
    const loaded = `Loaded the demo “${escapeHtml(demo.title)}”. Click Compile to see it.`;
    if (demo.example) {
        // the editor's own example: a new paper from it, as New does
        const text = await (await fetch(new URL(demo.example, DEMOS))).text();
        await setProject(await newBrowserProject("paper.md", text));
        showMsg(loaded, false, true);
    } else {
        // a group's base is relative to demos.json
        const base = group.base ? new URL(group.base, DEMOS) : demos.base;
        await openPaperURL(new URL(demo.paper, base).href, null, loaded, demo.files || []);
    }
}

async function clickOpenURL() {
    const url = prompt("URL of a paper (.md or .txt) or of a zip of its folder:");
    if (!url || !confirmReplaceBrowserProject()) return;
    await openPaperURL(url.trim());
}

async function clickOpenFolder() {
    try {
        const opened = await openFolderProject();
        if (opened) await setProject(opened);
    } catch (e) {
        showMsg("Couldn't open the folder: " + escapeHtml(e.message), true);
    }
}

async function clickReopenFolder() {
    try {
        const opened = await openFolderProject(reopenHandle);
        if (opened) {
            const meta = await idb("meta", "readonly", store => store.get("project"));
            opened.mdPath = meta ? meta.mdPath : null;
            await setProject(opened);
        }
    } catch (e) {
        showMsg("Couldn't open the folder: " + escapeHtml(e.message), true);
    }
}

async function onOpenFiles(input) {
    if (input.files.length === 0) return;
    if (!confirmReplaceBrowserProject()) return;
    await setProject(await openFilesProject(input.files));
    input.value = "";
}

async function clickNew() {
    if (!confirmReplaceBrowserProject()) return;
    await setProject(await newBrowserProject("paper.md", await (await fetch(STARTER)).text()));
}

// Whether replacing this browser paper would lose edits (a folder on disk is never replaced)
function hasEdits(paper) {
    if (!paper || paper.reopen || paper.mode !== "browser") return false;
    return paper.edited || (paper === project && ace.edit("editor").getValue() !== paper.readText(paper.mdPath));
}

// Opening another paper replaces the paper kept in this browser: ask, if it was edited
function confirmReplaceBrowserProject() {
    return !hasEdits(project) ||
        confirm("This replaces the paper kept in this browser, which you've edited, and its files. Download a zip of them first if you need them. Continue?");
}

// The paper is saved shortly after each edit (and before the page closes), one save at a time
const SAVE_DELAY = 500;
let saveTimer = null;
let saving = Promise.resolve();

function scheduleSave() {
    clearTimeout(saveTimer);
    saveTimer = setTimeout(savePaper, SAVE_DELAY);
}

// Save the editor's text as the paper, if it changed. Resolves once it's saved.
function savePaper() {
    clearTimeout(saveTimer);
    saveTimer = null;
    saving = saving.then(async () => {
        if (project === null) return;
        const text = ace.edit("editor").getValue();
        if (text === project.readText(project.mdPath)) return;
        try {
            await project.write(project.mdPath, text);
        } catch (e) {
            showMsg("Couldn't save the paper: " + escapeHtml(e.message), true);
        }
    });
    return saving;
}

// Ctrl/⌘-S: save now (it's saved anyway)
async function clickSave() {
    if (project === null) return;
    await savePaper();
    showMsg(project.mode === "browser" ? `Saved ${escapeHtml(project.mdPath)} in this browser` : `Saved ${escapeHtml(project.mdPath)}`);
}

function clickDownloadPaper() {
    download(new Blob([ace.edit("editor").getValue()], { type: "text/markdown" }),
             project.mdPath.slice(project.mdPath.lastIndexOf("/") + 1));
}

// The code compiling generates (output_code/lib.py, lib.h, lib.m), one file or all in a zip
function clickDownloadCode(file) {
    const code = new Map([...project.files].filter(([path]) => path.startsWith("output_code/"))
                                           .map(([path, bytes]) => [path.slice("output_code/".length), bytes]));
    if (code.size === 0) {
        showMsg("Compile the paper first: its code is generated by compiling.", true);
    } else if (file === undefined) {
        download(zipFiles(code), `${project.paperName}-code.zip`);
    } else if (code.has(file)) {
        download(new Blob([code.get(file)], { type: "text/plain" }), file);
    } else {
        showMsg(`The paper has no ${file}: it has no I❤️LA code.`, true);
    }
}

async function clickDownloadZip() {
    await savePaper();
    download(zipProject(project), `${project.paperName}.zip`);
}

function initDropTarget() {
    document.addEventListener("dragover", (event) => event.preventDefault());
    document.addEventListener("drop", async (event) => {
        event.preventDefault();
        const files = await droppedFiles(event.dataTransfer);
        if (files.length === 0 || !confirmReplaceBrowserProject()) return;
        await setProject(await openFilesProject(files));
    });
}

// The files (and files in folders) of a drop, each with its relative path
async function droppedFiles(dataTransfer) {
    const entries = [...dataTransfer.items].map(item => item.webkitGetAsEntry && item.webkitGetAsEntry()).filter(e => e);
    if (entries.length === 0) return [...dataTransfer.files];
    const files = [];
    const walk = async (entry, prefix) => {
        if (entry.isFile) {
            const file = await new Promise((resolve, reject) => entry.file(resolve, reject));
            file.relativePath = prefix + entry.name;
            files.push(file);
        } else {
            const reader = entry.createReader();
            let batch;
            do {
                batch = await new Promise((resolve, reject) => reader.readEntries(resolve, reject));
                for (const child of batch) await walk(child, `${prefix}${entry.name}/`);
            } while (batch.length > 0);
        }
    };
    for (const entry of entries) await walk(entry, "");
    // One dropped folder: its files are relative to it
    return files;
}

function removeIframe(){
    const output = document.getElementById('output');
    while (output.firstChild) {
        output.removeChild(output.firstChild);
    }
}

function showIframe(figures){
    const output = document.getElementById('output');
    const old = document.getElementById('res');
    // Keep the reader's place across compiles
    let scroll = 0;
    try { scroll = old ? old.contentWindow.scrollY : 0; } catch (e) { /* not loaded */ }
    const iframe = document.createElement('iframe');
    iframe.setAttribute("id", "res");
    iframe.onload = () => {
        if (scroll) iframe.contentWindow.scrollTo(0, scroll);
        if (figures && typeof iframe.contentWindow.handleFigure === "function") {
            iframe.contentWindow.handleFigure(figures);
        }
    };
    iframe.src = project.previewURL(`${curFileName}.html`) + `?v=${Date.now()}`;
    removeIframe();
    output.appendChild(iframe);
}

function reloadIframe(){
    const iframe = document.getElementById('res');
    if (iframe) iframe.contentWindow.location.reload();
}

function updateError(err) {
    showMsg(escapeHtml(err), true);
    activateBtnStatus();
}

async function compileFunction(){
    let editor = ace.edit("editor");
    const source = editor.getValue();
    latestSource = source;
    await savePaper();
    const data = await ask({ type: "compile", name: project.paperName, source });
    if (data.ret === 0) {
        curFileName = data.name;
        showMsg('Compile succeeded');
        showIframe(data.fig);
    } else {
        console.log(`data.expr is ${data.expr}`);
        console.log(`data.msg is ${data.msg}`);
        if (data.expr) scrollToText(data.expr);
        showMsg(escapeHtml(data.msg), true);
    }
}

function onEditEquation(raw_text){
    preEqCode = decodeURIComponent(escape(window.atob(raw_text)));
    console.log(`Received: ${raw_text}`);
    // document.getElementById("equation").innerHTML = raw_text;
    let equation = ace.edit("equation");
    equation.setValue(preEqCode, 1);
    $('#eqEditor').modal('show');
}

async function clickCompile(){
    if (!workerReady || compiling || project === null) return;
    hideMsg();
    compiling = true;
    document.getElementById("compile").disabled = true;
    setBtnTitle("Compiling", true);
    try {
        await compileFunction();
    } catch (error) {
        console.error(error);
        showMsg('Compile error!\n' + escapeHtml(error.message), true);
    } finally {
        compiling = false;
        activateBtnStatus();
    }
}

function onCancelUpdateEq() {
    clearEq();
}


function findLineNumber(editor, text) {
    let lines = editor.session.doc.getAllLines();
    let textLineNumber = [];
    for (let i = 0, l = lines.length; i < l; i++) {
        if (lines[i].indexOf(text) !== -1){
            textLineNumber.push(i);
        }
    }
    return textLineNumber
}

function scrollToText(text) {
    var Range = ace.require("ace/range").Range;
    let editor = ace.edit("editor");
    let line = findLineNumber(editor, text);
    editor.scrollToLine(line, true, true, null);
    var range = new Range(line[0], 0, line[0]+1, 0);
    console.log(`click line: ${line[0]}, `);
    editor.session.addMarker(range, "error", "text");
}

function removeMarkers() {
    let editor = ace.edit("editor");
    const prevMarkers = editor.session.getMarkers();
    if (prevMarkers) {
      const prevMarkersArr = Object.keys(prevMarkers);
      for (let item of prevMarkersArr) {
        editor.session.removeMarker(prevMarkers[item].id);
      }
    }
}

function clearEq() {
    let equation = ace.edit("equation");
    $('#eqEditor').modal('hide');
    equation.setValue('', 1);
}

function onUpdateEq() {
     let equation = ace.edit("equation");
     content = equation.getValue();
     let editor = ace.edit("editor");
     latestSource = latestSource.replace(preEqCode, content);
     // console.log(`source is ${source}`);
     $('#eqEditor').modal('hide');
     clearEq();
     editor.setValue(latestSource, 1);
     // same as manually clicking
     clickCompile();
}

async function onUpdatePython() {
    let python = ace.edit("python");
    const content = python.getValue();
    let editor = ace.edit("editor");
    latestSource = latestSource.replace(preTestCode, content);
    $('#testEditor').modal('hide');
    clearTest();
    editor.setValue(latestSource, 1);
    compiling = true;
    document.getElementById("compile").disabled = true;
    setBtnTitle("Running", true);
    try {
        await project.write(project.mdPath, latestSource);
        const result = await ask({ type: "figure", name: curFigure, source: content, paper: latestSource });
        if (result.ok) {
            reloadIframe();
        } else {
            console.log(`error is ${result.msg}`);
            document.getElementById("res").contentWindow.updateFigure(curFigure, result.msg, true);
        }
    } catch (error) {
        showMsg('Run error!\n' + escapeHtml(error.message), true);
    } finally {
        compiling = false;
        activateBtnStatus();
    }
}

function onCancelUpdatePython() {
    clearTest();
}

function clearTest() {
    let python = ace.edit("python");
    $('#testEditor').modal('hide');
    python.setValue('', 1);
}

function onClickFigure(ele, name){
    curFigure = name;
    // app.py saves each figure's code next to the paper (IMG_CODE is ".")
    const code = project.readText(`${name}.py`);
    if (code === undefined) return;
    let python = ace.edit("python");
    preTestCode = code;
    python.setValue(code, 1);
    $('#testEditor').modal('show');
}

// Copy a link to the editor with the paper's text in it, compressed, in the fragment (#z=), which
// isn't sent to the server: no limit on its length but the browser's, and the server doesn't see the paper
async function clickShare() {
    const url = new URL(window.location.href);
    url.search = "";
    url.hash = "z=" + await compressText(ace.edit("editor").getValue());
    const link = url.href;
    const others = project !== null && project.files.size > 1 ? " It has only the paper's text, not its other files." : "";
    try {
        await navigator.clipboard.writeText(link);
        showMsg(`Copied a link to this paper to the clipboard.${others}`);
    } catch (e) {
        prompt(`Copy this link to the paper.${others}`, link);
    }
}

// text, deflated, in base64url
async function compressText(text) {
    const stream = new Blob([encoder.encode(text)]).stream().pipeThrough(new CompressionStream("deflate-raw"));
    const bytes = new Uint8Array(await new Response(stream).arrayBuffer());
    let binary = "";
    for (let i = 0; i < bytes.length; i += 0x8000) {
        binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
    }
    return btoa(binary).replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/, "");
}

async function decompressText(base64url) {
    const binary = atob(base64url.replace(/-/g, "+").replace(/_/g, "/"));
    const bytes = Uint8Array.from(binary, c => c.charCodeAt(0));
    const stream = new Blob([bytes]).stream().pipeThrough(new DecompressionStream("deflate-raw"));
    return decoder.decode(await new Response(stream).arrayBuffer());
}

// msg is HTML: escape text from elsewhere (escapeHtml). A sticky notice stays until the next message or edit.
function showMsg(msg, error=false, sticky=false){
    if (msgTimeout){
        clearTimeout(msgTimeout);
    }
    msg = msg.replaceAll('\n', '<br>')
    let el = document.getElementById("msg");
    el.hidden = false;
    el.innerHTML = msg;

    // Alert types: https://getbootstrap.com/docs/4.0/components/alerts/
    // Edit class: https://stackoverflow.com/questions/195951/how-can-i-change-an-elements-class-with-javascript
    el.classList.remove('alert-primary');
    el.classList.remove('alert-danger');
    el.classList.remove('alert-warning');
    if(error) {
        el.classList.add('alert-danger');
    } else if (sticky) {
        el.classList.add('alert-warning');
    } else {
        // notice, auto hide
        el.classList.add('alert-primary');
        msgTimeout = setTimeout(hideMsg, 2000);
    }
    MathJax.typesetPromise([el]).then(() => {
      // the new content is has been typeset
    });
}

function hideMsg(){
    document.getElementById("msg").hidden = true; document.getElementById("msg").innerHTML = '';
}

function setBtnTitle(text, spin=false){
    document.getElementById("compile").innerHTML = `<i id="submit_icon" class="fa fa-refresh${spin ? " fa-spin" : ""}"></i> ` + text;
}

function initBtnStatus(){
    document.getElementById("compile").disabled = true;
    setBtnTitle("Loading H❤️rtDown…", true);
}

function activateBtnStatus(){
    if (!workerReady) {
        initBtnStatus();
        return;
    }
    document.getElementById("compile").disabled = false;
    setBtnTitle("Compile");
}

// A change in an editor (ace's change event). delta: the change
function onEditIhla(editor, delta){
    hideMsg();
    removeMarkers();
    substitute(editor, delta);
}

// Typing a shortcut and a space (\alpha ) replaces them with the symbol (α). Only what was just
// typed: the paper's own LaTeX ($\lambda \cdot x$) and loaded text are left alone.
function substitute(editor, delta){
    if (delta.action !== "insert" || delta.lines.length !== 1 || delta.lines[0] !== " ") return;
    const row = delta.end.row;
    const before = editor.session.getLine(row).slice(0, delta.end.column);
    const m = before.match(/\\([^\\\s]+) $/);
    if (!m || !(m[1] in unicode_dict)) return;
    // After ace finishes this change. Edits may come first (fast typing, another shortcut), so an
    // anchor follows the shortcut's position, and it's replaced only if it's still there.
    const start = editor.session.doc.createAnchor(row, delta.end.column - m[0].length);
    setTimeout(() => {
        const Range = ace.require("ace/range").Range;
        const at = start.getPosition();
        start.detach();
        const range = new Range(at.row, at.column, at.row, at.column + m[0].length);
        if (editor.session.getTextRange(range) !== m[0]) return;
        const cursor = editor.getCursorPosition();
        const end = editor.session.replace(range, unicode_dict[m[1]]);
        if (cursor.row === at.row && cursor.column === at.column + m[0].length) {
            editor.moveCursorToPosition(end);
            editor.clearSelection();
        }
    }, 0);
}

function captureHotKeys() {
    // https://stackoverflow.com/questions/93695/best-cross-browser-method-to-capture-ctrls-with-jquery
    window.addEventListener( 'keydown', function(event) {
        if (event.ctrlKey || event.metaKey) {
            switch (String.fromCharCode(event.which).toLowerCase()) {
            case 'r':
                event.preventDefault();
                clickCompile();
                break;
            case 's':
                event.preventDefault();
                clickSave();
                break;
            }
        }
    });
}
