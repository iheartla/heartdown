// Serves the open paper's folder to the preview iframe, so the generated HTML
// can load its images, figures and heartdown-resource/ by relative URLs, as it
// did from the old Python server. The files live in the editor page (see
// js/project.js), not here: a request for paper/<session>/<path> is passed to
// the editor page with that session id, which answers with the file.
//
// It also makes the browser check with the server that its copies of the
// editor's own files are current, as servers without Cache-Control (python3 -m
// http.server) or with a max-age (GitHub Pages) let it skip that. A page would
// otherwise mix two versions of the editor, such as a new editor.js with an old
// project.js.

const SCOPE = new URL(self.registration.scope).pathname;
const PREFIX = new URL("paper/", self.registration.scope).pathname;
const TIMEOUT = 10000;

const TYPES = {
    html: "text/html", htm: "text/html", css: "text/css", js: "text/javascript", mjs: "text/javascript",
    json: "application/json", svg: "image/svg+xml", png: "image/png", jpg: "image/jpeg", jpeg: "image/jpeg",
    gif: "image/gif", webp: "image/webp", bmp: "image/bmp", ico: "image/x-icon", pdf: "application/pdf",
    woff: "font/woff", woff2: "font/woff2", ttf: "font/ttf", otf: "font/otf", mp4: "video/mp4", webm: "video/webm",
    txt: "text/plain", md: "text/plain", py: "text/plain", bib: "text/plain", ihla: "text/plain", hlang: "text/plain",
    m: "text/plain", h: "text/plain", map: "application/json",
};

self.addEventListener("install", () => self.skipWaiting());
self.addEventListener("activate", (event) => event.waitUntil(self.clients.claim()));

self.addEventListener("fetch", (event) => {
    const url = new URL(event.request.url);
    if (url.origin !== self.location.origin || !url.pathname.startsWith(SCOPE)) return;
    if (url.pathname.startsWith(PREFIX)) {
        event.respondWith(serve(url));
    } else if (event.request.method === "GET") {
        // An unchanged file costs a 304
        event.respondWith(fetch(new Request(event.request, { cache: "no-cache" })));
    }
});

// Ask one window for the file; it answers only if it has this session
function ask(client, session, path) {
    return new Promise((resolve, reject) => {
        const channel = new MessageChannel();
        channel.port1.onmessage = (event) => event.data ? resolve(event.data) : reject();
        client.postMessage({ type: "heartdown-file", session, path }, [channel.port2]);
    });
}

async function serve(url) {
    const rest = url.pathname.slice(PREFIX.length);
    const slash = rest.indexOf("/");
    const session = rest.slice(0, slash);
    const path = decodeURIComponent(rest.slice(slash + 1));
    const windows = await self.clients.matchAll({ type: "window", includeUncontrolled: true });
    let answer;
    try {
        answer = await Promise.race([
            Promise.any(windows.map((client) => ask(client, session, path))),
            new Promise((_, reject) => setTimeout(reject, TIMEOUT)),
        ]);
    } catch (e) {
        return new Response("The H❤️rtDown editor for this paper is not open.", { status: 404 });
    }
    if (!answer.found) {
        return new Response(`Not found: ${path}`, { status: 404 });
    }
    const ext = path.slice(path.lastIndexOf(".") + 1).toLowerCase();
    return new Response(answer.bytes, {
        headers: {
            "Content-Type": (TYPES[ext] || "application/octet-stream") + (TYPES[ext]?.startsWith("text") ? "; charset=utf-8" : ""),
            "Cache-Control": "no-store",
        },
    });
}
