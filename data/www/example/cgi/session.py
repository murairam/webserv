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


def render_html(session_id, session_data, cookie_header, message, query_string):
    created = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(session_data.get("created", 0)))
    last_visit = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(session_data.get("last_visit", time.time())))
    counter = session_data.get("counter", 0)
    visit_count = session_data.get("visit_count", 0)
    username = session_data.get("user_name", "Guest")
    last_action = session_data.get("last_action", "Session initialised")
    session_age = int(time.time() - session_data.get("created", time.time()))
    minutes_left = max(0, (SESSION_TIMEOUT - session_age) // 60)

    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🔐 Session Management Demo</title>
    <style>
        :root {
            color-scheme: light dark;
            --bg: #111827;
            --bg-alt: #1f2937;
            --panel: rgba(17, 24, 39, 0.82);
            --accent: #38bdf8;
            --accent-strong: #0ea5e9;
            --text: #e5e7eb;
            --muted: #9ca3af;
            --success: #10b981;
            --warning: #f59e0b;
            --danger: #ef4444;
            --purple: #a78bfa;
            --shadow: 0 30px 60px rgba(15, 23, 42, 0.32);
            font-family: "Inter", "Segoe UI", system-ui, -apple-system, sans-serif;
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            background: radial-gradient(circle at top, #1f2937 0%, #0f172a 50%, #030712 100%);
            color: var(--text);
            display: flex;
            justify-content: center;
            padding: clamp(1.5rem, 3vw + 1rem, 4rem);
        }

        main {
            width: min(1100px, 100%);
            backdrop-filter: blur(18px);
            background: var(--panel);
            border-radius: 28px;
            padding: clamp(1.75rem, 2.5vw + 1rem, 3rem);
            box-shadow: var(--shadow);
        }

        header {
            text-align: center;
            margin-bottom: 2rem;
        }

        header h1 {
            margin: 0;
            font-size: clamp(2rem, 4vw, 2.75rem);
            letter-spacing: -0.02em;
        }

        header p {
            margin: 0.75rem auto 1rem auto;
            max-width: 680px;
            color: var(--muted);
            font-size: 1.05rem;
        }

        .nav-link {
            display: inline-flex;
            align-items: center;
            gap: 0.4rem;
            padding: 0.55rem 1.1rem;
            border-radius: 999px;
            border: 1px solid rgba(148, 163, 184, 0.32);
            color: var(--text);
            text-decoration: none;
            transition: border 0.2s ease, transform 0.2s ease;
            margin-top: 1rem;
        }

        .nav-link:hover {
            border-color: var(--accent);
            transform: translateY(-2px);
        }

        section {
            margin-top: 2rem;
            padding-top: 2rem;
            border-top: 1px solid rgba(148, 163, 184, 0.16);
        }

        section:first-of-type {
            border-top: none;
            padding-top: 0;
            margin-top: 0;
        }

        h2 {
            margin: 0 0 1rem 0;
            font-size: clamp(1.5rem, 2.5vw, 1.85rem);
            color: var(--accent);
        }

        h3 {
            margin: 1.5rem 0 0.75rem 0;
            font-size: 1.25rem;
        }

        .info-box {
            background: rgba(15, 23, 42, 0.6);
            border: 1px solid rgba(148, 163, 184, 0.18);
            border-radius: 16px;
            padding: 1.25rem;
            margin: 1rem 0;
        }

        .info-box.success {
            border-color: rgba(16, 185, 129, 0.3);
            background: rgba(16, 185, 129, 0.05);
        }

        .info-box.warning {
            border-color: rgba(245, 158, 11, 0.3);
            background: rgba(245, 158, 11, 0.05);
        }

        .info-box.purple {
            border-color: rgba(167, 139, 250, 0.3);
            background: rgba(167, 139, 250, 0.05);
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 1rem;
            margin: 1.5rem 0;
        }

        .stat-card {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.12);
            border-radius: 14px;
            padding: 1.25rem;
            text-align: center;
        }

        .stat-value {
            font-size: 2rem;
            font-weight: 700;
            color: var(--accent);
            margin: 0.5rem 0;
        }

        .stat-label {
            color: var(--muted);
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }

        .action-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 1rem;
            margin: 1.5rem 0;
        }

        .action-button {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 0.5rem;
            padding: 1.25rem;
            border-radius: 16px;
            text-decoration: none;
            transition: all 0.2s ease;
            border: 2px solid transparent;
        }

        .action-button.primary {
            background: linear-gradient(135deg, rgba(56, 189, 248, 0.15), rgba(14, 165, 233, 0.15));
            border-color: rgba(56, 189, 248, 0.3);
        }

        .action-button.success {
            background: linear-gradient(135deg, rgba(16, 185, 129, 0.15), rgba(5, 150, 105, 0.15));
            border-color: rgba(16, 185, 129, 0.3);
        }

        .action-button.warning {
            background: linear-gradient(135deg, rgba(245, 158, 11, 0.15), rgba(217, 119, 6, 0.15));
            border-color: rgba(245, 158, 11, 0.3);
        }

        .action-button.danger {
            background: linear-gradient(135deg, rgba(239, 68, 68, 0.15), rgba(220, 38, 38, 0.15));
            border-color: rgba(239, 68, 68, 0.3);
        }

        .action-button:hover {
            transform: translateY(-3px);
            box-shadow: 0 15px 30px rgba(56, 189, 248, 0.2);
        }

        .action-icon {
            font-size: 2rem;
        }

        .action-title {
            font-weight: 600;
            color: var(--text);
            font-size: 1rem;
        }

        .action-desc {
            font-size: 0.85rem;
            color: var(--muted);
            text-align: center;
        }

        form {
            display: flex;
            gap: 0.75rem;
            align-items: flex-end;
            flex-wrap: wrap;
            margin: 1rem 0;
        }

        .form-group {
            flex: 1;
            min-width: 200px;
        }

        label {
            display: block;
            font-weight: 600;
            color: rgba(226, 232, 240, 0.85);
            margin-bottom: 0.4rem;
            font-size: 0.9rem;
        }

        input[type="text"] {
            background: rgba(15, 23, 42, 0.6);
            color: var(--text);
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 12px;
            padding: 0.7rem 0.9rem;
            font-size: 0.95rem;
            width: 100%;
        }

        input[type="text"]:focus {
            outline: none;
            border-color: var(--accent);
        }

        button {
            border: none;
            border-radius: 999px;
            padding: 0.7rem 1.4rem;
            font-weight: 600;
            letter-spacing: 0.01em;
            cursor: pointer;
            transition: transform 0.18s ease, box-shadow 0.18s ease;
            background: linear-gradient(120deg, var(--accent), var(--accent-strong));
            color: #04111f;
        }

        button:hover {
            transform: translateY(-1px);
            box-shadow: 0 10px 25px rgba(14, 165, 233, 0.3);
        }

        .technical-info {
            background: rgba(15, 23, 42, 0.8);
            border: 1px solid rgba(148, 163, 184, 0.12);
            border-radius: 14px;
            padding: 1rem;
            font-family: "JetBrains Mono", "Fira Mono", ui-monospace, monospace;
            font-size: 0.85rem;
            overflow-x: auto;
        }

        .technical-info dt {
            color: var(--accent);
            font-weight: 600;
            margin-top: 0.5rem;
        }

        .technical-info dt:first-child {
            margin-top: 0;
        }

        .technical-info dd {
            margin: 0.25rem 0 0 0;
            color: var(--muted);
            word-break: break-all;
        }

        .flow-diagram {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.12);
            border-radius: 14px;
            padding: 1.5rem;
            margin: 1.5rem 0;
            font-family: "JetBrains Mono", "Fira Mono", ui-monospace, monospace;
            font-size: 0.9rem;
            line-height: 1.8;
        }

        .flow-step {
            margin: 0.5rem 0;
            padding-left: 1.5rem;
            position: relative;
        }

        .flow-step::before {
            content: "→";
            position: absolute;
            left: 0;
            color: var(--accent);
            font-weight: bold;
        }

        code {
            background: rgba(56, 189, 248, 0.16);
            border-radius: 0.4rem;
            padding: 0.15rem 0.4rem;
            color: #bae6fd;
            font-family: "JetBrains Mono", "Fira Mono", ui-monospace, monospace;
            font-size: 0.9em;
        }

        ul {
            line-height: 1.7;
            color: var(--muted);
        }

        ul li {
            margin: 0.5rem 0;
        }

        @media (max-width: 640px) {
            body {
                padding: 1rem;
            }

            main {
                padding: 1.4rem;
                border-radius: 18px;
            }

            .stats-grid {
                grid-template-columns: 1fr;
            }

            .action-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
<main>
    <header>
        <h1>🔐 Session Management Demo</h1>
        <p>Server-side session state backed by cookies and file storage, demonstrating persistent user data across HTTP requests.</p>
        <a class="nav-link" href="/">← Back to main playground</a>
    </header>""")

    if message:
        print(f"""
    <div class="info-box success">
        <strong>✓ Action completed:</strong> {message}
    </div>""")

    print(f"""
    <section>
        <h2>Your Session State</h2>
        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-label">User</div>
                <div class="stat-value">{username}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Counter</div>
                <div class="stat-value">{counter}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Visits</div>
                <div class="stat-value">{visit_count}</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Session TTL</div>
                <div class="stat-value">{minutes_left}m</div>
            </div>
        </div>

        <div class="info-box purple">
            <strong>Last action:</strong> {last_action}<br>
            <strong>Session created:</strong> {created}<br>
            <strong>Last visit:</strong> {last_visit}<br>
            <strong>Session ID:</strong> <code>{session_id[:24]}...</code>
        </div>
    </section>

    <section>
        <h2>Session Actions</h2>

        <h3>Login as Different User</h3>
        <form method="GET" action="/cgi/session.py">
            <input type="hidden" name="action" value="login">
            <div class="form-group">
                <label for="username">Username</label>
                <input type="text" id="username" name="name" placeholder="Enter your name" required>
            </div>
            <button type="submit">🔑 Login</button>
        </form>

        <h3>Quick Actions</h3>
        <div class="action-grid">
            <a href="/cgi/session.py?action=increment" class="action-button success">
                <div class="action-icon">➕</div>
                <div class="action-title">Increment</div>
                <div class="action-desc">Add 1 to counter</div>
            </a>

            <a href="/cgi/session.py?action=logout" class="action-button warning">
                <div class="action-icon">🚪</div>
                <div class="action-title">Logout</div>
                <div class="action-desc">Reset to Guest</div>
            </a>

            <a href="/cgi/session.py?action=reset" class="action-button danger">
                <div class="action-icon">🔄</div>
                <div class="action-title">Reset Session</div>
                <div class="action-desc">Clear all data</div>
            </a>

            <a href="/cgi/session.py" class="action-button primary">
                <div class="action-icon">↻</div>
                <div class="action-title">Refresh</div>
                <div class="action-desc">Reload this page</div>
            </a>
        </div>
    </section>

    <section>
        <h2>How Sessions Work With Our Server</h2>

        <div class="info-box">
            <p><strong>Sessions extend cookies to store data server-side.</strong> While cookies store data in the browser (limited, visible, insecure), sessions use cookies to store only a <strong>session ID</strong>, with the actual data stored securely on the server.</p>
        </div>

        <h3>The Session Flow</h3>
        <div class="flow-diagram">
            <div class="flow-step">Browser requests /cgi/session.py (no cookies yet)</div>
            <div class="flow-step">C++ Server executes Python CGI</div>
            <div class="flow-step">CGI checks HTTP_COOKIE for SESSIONID</div>
            <div class="flow-step">No session? Generate new ID: sha256(time+pid+random)</div>
            <div class="flow-step">Create session file: ./sessions/sess_abc123.json</div>
            <div class="flow-step">Store data: {{"user": "Guest", "counter": 0, "created": 1234567890}}</div>
            <div class="flow-step">CGI outputs: Set-Cookie: SESSIONID=abc123; HttpOnly; SameSite=Lax</div>
            <div class="flow-step">Server forwards Set-Cookie to browser</div>
            <div class="flow-step">Browser stores session cookie</div>
            <div class="flow-step">Next request: Browser sends Cookie: SESSIONID=abc123</div>
            <div class="flow-step">Server passes to CGI via HTTP_COOKIE</div>
            <div class="flow-step">CGI loads ./sessions/sess_abc123.json</div>
            <div class="flow-step">CGI updates data (increment counter, etc.)</div>
            <div class="flow-step">CGI saves updated JSON back to file</div>
            <div class="flow-step">Session persists until timeout ({SESSION_TIMEOUT}s = {SESSION_TIMEOUT//60} minutes)</div>
        </div>

        <h3>Why Sessions > Cookies Alone</h3>
        <div class="info-box">
            <table style="width: 100%; border-collapse: collapse;">
                <thead>
                    <tr style="border-bottom: 1px solid rgba(148, 163, 184, 0.2);">
                        <th style="text-align: left; padding: 0.5rem; color: var(--accent);">Feature</th>
                        <th style="text-align: left; padding: 0.5rem; color: var(--accent);">Cookies Only</th>
                        <th style="text-align: left; padding: 0.5rem; color: var(--accent);">Cookie + Session</th>
                    </tr>
                </thead>
                <tbody>
                    <tr style="border-bottom: 1px solid rgba(148, 163, 184, 0.1);">
                        <td style="padding: 0.5rem;">Storage location</td>
                        <td style="padding: 0.5rem; color: var(--muted);">Browser</td>
                        <td style="padding: 0.5rem; color: var(--success);">Server</td>
                    </tr>
                    <tr style="border-bottom: 1px solid rgba(148, 163, 184, 0.1);">
                        <td style="padding: 0.5rem;">Data size limit</td>
                        <td style="padding: 0.5rem; color: var(--muted);">4KB per cookie</td>
                        <td style="padding: 0.5rem; color: var(--success);">Unlimited (disk)</td>
                    </tr>
                    <tr style="border-bottom: 1px solid rgba(148, 163, 184, 0.1);">
                        <td style="padding: 0.5rem;">Security</td>
                        <td style="padding: 0.5rem; color: var(--danger);">Visible to user</td>
                        <td style="padding: 0.5rem; color: var(--success);">Hidden on server</td>
                    </tr>
                    <tr style="border-bottom: 1px solid rgba(148, 163, 184, 0.1);">
                        <td style="padding: 0.5rem;">Bandwidth</td>
                        <td style="padding: 0.5rem; color: var(--danger);">Sent with every request</td>
                        <td style="padding: 0.5rem; color: var(--success);">Only ID sent (~32 bytes)</td>
                    </tr>
                    <tr>
                        <td style="padding: 0.5rem;">Data types</td>
                        <td style="padding: 0.5rem; color: var(--muted);">Strings only</td>
                        <td style="padding: 0.5rem; color: var(--success);">Complex objects (JSON)</td>
                    </tr>
                </tbody>
            </table>
        </div>

        <h3>Implementation Architecture</h3>
        <div class="info-box">
            <h4>Server Layer (C++)</h4>
            <ul>
                <li>Parses incoming <code>Cookie:</code> header containing SESSIONID</li>
                <li>Passes cookies to CGI via <code>HTTP_COOKIE</code> environment variable</li>
                <li>Forwards <code>Set-Cookie:</code> headers from CGI back to browser</li>
                <li><strong>Server is session-agnostic</strong> - doesn't know or care about session logic</li>
            </ul>
        </div>

        <div class="info-box">
            <h4>Application Layer (Python CGI)</h4>
            <ul>
                <li><strong>Session ID generation:</strong> <code>sha256(timestamp + PID + 16 random bytes)</code></li>
                <li><strong>Storage:</strong> JSON files in <code>./cgi/sessions/</code> directory</li>
                <li><strong>Validation:</strong> Checks if session file exists and hasn't expired</li>
                <li><strong>Timeout:</strong> Sessions expire after {SESSION_TIMEOUT}s ({SESSION_TIMEOUT//60} minutes) of inactivity</li>
                <li><strong>Cookie attributes:</strong> <code>HttpOnly</code> (no JS access), <code>SameSite=Lax</code> (CSRF protection)</li>
            </ul>
        </div>

        <div class="info-box warning">
            <h4>🔐 Production Considerations</h4>
            <p>This demo uses file-based storage for simplicity. Real applications should use:</p>
            <ul>
                <li><strong>Database storage</strong> (Redis, PostgreSQL) for scalability</li>
                <li><strong>Secure cookies</strong> with <code>Secure</code> flag (HTTPS only)</li>
                <li><strong>CSRF tokens</strong> for state-changing operations</li>
                <li><strong>Session regeneration</strong> after login to prevent fixation attacks</li>
                <li><strong>Proper cleanup</strong> with scheduled jobs to delete expired sessions</li>
            </ul>
        </div>

        <h3>File Storage Format</h3>
        <div class="info-box">
            <p>Each session is stored as a JSON file at <code>./cgi/sessions/sess_{{session_id}}.json</code>:</p>
            <pre style="background: #1a202c; color: #f7fafc; padding: 1rem; border-radius: 8px; overflow: auto; margin-top: 0.5rem;">{{
    "created": {session_data.get("created", 0)},
    "last_visit": {session_data.get("last_visit", 0)},
    "visit_count": {visit_count},
    "counter": {counter},
    "user_name": "{username}",
    "last_action": "{last_action}"
}}</pre>
        </div>
    </section>

    <section>
        <h2>Technical Details</h2>
        <dl class="technical-info">
            <dt>HTTP_COOKIE (from browser):</dt>
            <dd>{cookie_header if cookie_header else '(empty - no cookies sent)'}</dd>

            <dt>Session ID:</dt>
            <dd>{session_id}</dd>

            <dt>Session file path:</dt>
            <dd>{session_path(session_id)}</dd>

            <dt>QUERY_STRING:</dt>
            <dd>{query_string if query_string else '(empty - no action taken)'}</dd>

            <dt>REQUEST_METHOD:</dt>
            <dd>{os.environ.get('REQUEST_METHOD', 'N/A')}</dd>

            <dt>SCRIPT_NAME:</dt>
            <dd>{os.environ.get('SCRIPT_NAME', 'N/A')}</dd>

            <dt>SERVER_SOFTWARE:</dt>
            <dd>{os.environ.get('SERVER_SOFTWARE', 'N/A')}</dd>

            <dt>Session timeout:</dt>
            <dd>{SESSION_TIMEOUT} seconds ({SESSION_TIMEOUT//60} minutes)</dd>

            <dt>Time remaining:</dt>
            <dd>{minutes_left} minutes</dd>
        </dl>
    </section>

    <section>
        <h2>Try These Actions</h2>
        <div class="info-box">
            <ul>
                <li>Login with different usernames and see the session persist your identity</li>
                <li>Increment the counter multiple times - it persists even after closing the tab (within {SESSION_TIMEOUT//60} minutes)</li>
                <li>Open this page in multiple tabs - they all share the same session</li>
                <li>Check <code>./data/www/example/cgi/sessions/</code> to see your session file on disk</li>
                <li>Wait {SESSION_TIMEOUT//60} minutes and the session expires automatically</li>
                <li>Compare with simple cookies at <a href="/cgi/cookies.py" style="color: var(--accent);">/cgi/cookies.py</a> to see the difference</li>
                <li>Use the JSON API: add <code>?format=json</code> to get session data as JSON instead of HTML</li>
            </ul>
        </div>
    </section>

    <footer style="margin-top: 3rem; text-align: center; color: var(--muted); font-size: 0.9rem;">
        <p>Session management by Mari & Yang - Part of the 42 Webserv Project</p>
        <p>C++98 HTTP Server with Python CGI • SHA-256 session IDs • File-based persistence • Full HTTP/1.1 support</p>
    </footer>
</main>
</body>
</html>""")


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
        render_html(session_id, session, cookie_header, message, query_string)


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
