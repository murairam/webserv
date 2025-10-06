#!/usr/bin/python3
"""Cookie-backed session management demo CGI."""

import hashlib
import json
import os
import sys
import time
from urllib.parse import parse_qs

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SESSION_DIR = os.path.join(BASE_DIR, "sessions")
SESSION_TIMEOUT = 1800  # seconds


def ensure_session_dir():
    """Ensure the session directory exists."""
    if not os.path.isdir(SESSION_DIR):
        os.makedirs(SESSION_DIR)


def generate_session_id():
    seed = f"{time.time()}:{os.getpid()}:{os.urandom(16)}"
    return hashlib.sha256(seed.encode("utf-8")).hexdigest()


def session_path(session_id):
    return os.path.join(SESSION_DIR, f"sess_{session_id}.json")


def load_session(session_id):
    if not session_id:
        return None
    path = session_path(session_id)
    if not os.path.exists(path):
        return None
    try:
        with open(path, "r", encoding="utf-8") as handle:
            data = json.load(handle)
    except Exception:
        return None
    if time.time() - data.get("created", 0) > SESSION_TIMEOUT:
        try:
            os.remove(path)
        except OSError:
            pass
        return None
    return data


def save_session(session_id, data):
    ensure_session_dir()
    path = session_path(session_id)
    data_to_store = dict(data)
    try:
        with open(path, "w", encoding="utf-8") as handle:
            json.dump(data_to_store, handle)
    except Exception:
        return False
    return True


def parse_cookies(header):
    cookies = {}
    if not header:
        return cookies
    for chunk in header.split(";"):
        if "=" not in chunk:
            continue
        name, value = chunk.split("=", 1)
        cookies[name.strip()] = value.strip()
    return cookies


def emit_headers(session_id, extra_headers=None, content_type="text/html; charset=utf-8"):
    print(f"Content-Type: {content_type}")
    cookie_directives = [f"SESSIONID={session_id}", "Path=/", f"Max-Age={SESSION_TIMEOUT}", "HttpOnly", "SameSite=Lax"]
    print("Set-Cookie: " + "; ".join(cookie_directives))
    if extra_headers:
        for header_name, header_value in extra_headers.items():
            print(f"{header_name}: {header_value}")
    print()


def render_html(session_id, session_data, cookie_header, message):
    created = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(session_data.get("created", 0)))
    last_visit = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(session_data.get("last_visit", time.time())))
    counter = session_data.get("counter", 0)
    visit_count = session_data.get("visit_count", 0)
    username = session_data.get("user_name", "Guest")
    last_action = session_data.get("last_action", "Session initialised")

    print("<!DOCTYPE html>")
    print("<html lang=\"en\">")
    print("<head>")
    print("    <meta charset=\"UTF-8\">")
    print("    <title>Cookie Session Demo</title>")
    print("    <style>")
    print("        body { font-family: 'Segoe UI', Arial, sans-serif; background:#f5f7fb; margin:0; padding:2rem; }")
    print("        .card { max-width:800px; margin:0 auto; background:#fff; border-radius:12px; padding:2rem; box-shadow:0 18px 40px rgba(31,45,61,0.12); }")
    print("        h1 { margin-top:0; color:#2d3748; }")
    print("        .pill { display:inline-block; padding:0.25rem 0.75rem; border-radius:999px; background:#ebf8ff; color:#3182ce; font-size:0.875rem; }")
    print("        dl { display:grid; grid-template-columns: max-content auto; gap:0.5rem 1.5rem; }")
    print("        dt { font-weight:600; color:#4a5568; }")
    print("        dd { margin:0; color:#2d3748; }")
    print("        .actions a { display:inline-block; margin:0.5rem 0.25rem; padding:0.5rem 1rem; border-radius:8px; background:#4c51bf; color:#fff; text-decoration:none; transition:background 0.2s ease; }")
    print("        .actions a:hover { background:#434190; }")
    print("        .message { background:#f0fff4; border:1px solid #9ae6b4; color:#276749; padding:0.75rem 1rem; border-radius:8px; margin-bottom:1rem; }")
    print("        pre { background:#1a202c; color:#f7fafc; padding:1rem; border-radius:8px; overflow:auto; }")
    print("    </style>")
    print("</head>")
    print("<body>")
    print("    <div class=\"card\">")
    print("        <span class=\"pill\">Cookie-backed server session</span>")
    print("        <h1>Session Playground</h1>")
    if message:
        print(f"        <div class=\"message\">{message}</div>")
    print("        <dl>")
    print(f"            <dt>Session ID</dt><dd>{session_id[:16]}…</dd>")
    print(f"            <dt>User</dt><dd>{username}</dd>")
    print(f"            <dt>Visit count</dt><dd>{visit_count}</dd>")
    print(f"            <dt>Counter value</dt><dd>{counter}</dd>")
    print(f"            <dt>Last action</dt><dd>{last_action}</dd>")
    print(f"            <dt>Created at</dt><dd>{created}</dd>")
    print(f"            <dt>Last visit</dt><dd>{last_visit}</dd>")
    print("        </dl>")
    print("        <div class=\"actions\">")
    print("            <a href=\"/cgi/session.py?action=increment\">Increment counter</a>")
    print("            <a href=\"/cgi/session.py?action=login&name=Traveler\">Login as Traveler</a>")
    print("            <a href=\"/cgi/session.py?action=logout\">Logout</a>")
    print("            <a href=\"/cgi/session.py?action=reset\">Reset session</a>")
    print("        </div>")
    print("        <h2>Raw cookie header</h2>")
    cookie_display = cookie_header if cookie_header else "No cookies sent"
    print(f"        <pre>{cookie_display}</pre>")
    print("        <p>Tip: interact with <code>/session-demo.html</code> for a JavaScript driven UI.</p>")
    print("    </div>")
    print("</body>")
    print("</html>")


def render_json(session_id, session_data, message):
    payload = {
        "session_id": session_id,
        "user_name": session_data.get("user_name", "Guest"),
        "visit_count": session_data.get("visit_count", 0),
        "counter": session_data.get("counter", 0),
        "last_action": session_data.get("last_action", "Session initialised"),
        "created": session_data.get("created", time.time()),
        "last_visit": session_data.get("last_visit", time.time()),
        "message": message,
    }
    print(json.dumps(payload))


def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    cookies = parse_cookies(cookie_header)
    session_id = cookies.get("SESSIONID")
    session = load_session(session_id)

    if session is None:
        session_id = generate_session_id()
        session = {
            "created": time.time(),
            "last_visit": time.time(),
            "visit_count": 0,
            "counter": 0,
            "user_name": "Guest",
            "last_action": "Session created",
        }

    query_string = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query_string)
    action = params.get("action", [""])[0]
    format_hint = params.get("format", ["html"])[0].lower()
    message = ""

    if action == "login":
        name = params.get("name", ["Guest"])[0] or "Guest"
        session["user_name"] = name
        session["last_action"] = f"Logged in as {name}"
        message = session["last_action"]
    elif action == "logout":
        session["user_name"] = "Guest"
        session["last_action"] = "Logged out"
        message = session["last_action"]
    elif action == "increment":
        session["counter"] = session.get("counter", 0) + 1
        session["last_action"] = "Incremented counter"
        message = session["last_action"]
    elif action == "reset":
        session = {
            "created": time.time(),
            "last_visit": time.time(),
            "visit_count": 0,
            "counter": 0,
            "user_name": "Guest",
            "last_action": "Session reset",
        }
        message = "Session cleared"
    else:
        message = "Session refreshed"

    session["visit_count"] = session.get("visit_count", 0) + 1
    session["last_visit"] = time.time()

    save_session(session_id, session)

    if format_hint == "json":
        emit_headers(session_id, content_type="application/json; charset=utf-8")
        render_json(session_id, session, message)
    else:
        emit_headers(session_id)
        render_html(session_id, session, cookie_header, message)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:  # pragma: no cover - fallback for CGI debugging
        print("Status: 500 Internal Server Error")
        print("Content-Type: text/plain; charset=utf-8")
        print()
        print("CGI script failed:")
        print(str(exc))
        sys.exit(1)
