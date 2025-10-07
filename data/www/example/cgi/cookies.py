#!/usr/bin/python3

import os
import time
from urllib.parse import parse_qs

def parse_cookies(cookie_header):
    """Parse cookies from HTTP header"""
    cookies = {}
    if cookie_header:
        for cookie in cookie_header.split(';'):
            if '=' in cookie:
                name, value = cookie.strip().split('=', 1)
                cookies[name] = value
    return cookies

def main():
    # Parse existing cookies
    cookie_header = os.environ.get('HTTP_COOKIE', '')
    cookies = parse_cookies(cookie_header)

    # Parse query parameters
    query_string = os.environ.get('QUERY_STRING', '')
    params = parse_qs(query_string)

    # Handle cookie operations
    action = params.get('action', [''])[0]
    new_cookies = []

    if action == 'set' and 'name' in params and 'value' in params:
        cookie_name = params['name'][0]
        cookie_value = params['value'][0]
        new_cookies.append(f"{cookie_name}={cookie_value}; Path=/")
    elif action == 'delete' and 'name' in params:
        cookie_name = params['name'][0]
        new_cookies.append(f"{cookie_name}=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT")
    elif action == 'visit':
        visit_count = int(cookies.get('visit_count', '0')) + 1
        new_cookies.append(f"visit_count={visit_count}; Path=/")
        new_cookies.append(f"last_visit={int(time.time())}; Path=/")

    # Output headers
    print("Content-Type: text/html; charset=UTF-8")
    for cookie in new_cookies:
        print(f"Set-Cookie: {cookie}")
    print()  # Empty line to end headers

    # Output HTML
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🍪 Cookie Management Demo</title>
    <style>
        :root {{
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
            --shadow: 0 30px 60px rgba(15, 23, 42, 0.32);
            font-family: "Inter", "Segoe UI", system-ui, -apple-system, sans-serif;
        }}

        * {{
            box-sizing: border-box;
        }}

        body {{
            margin: 0;
            min-height: 100vh;
            background: radial-gradient(circle at top, #1f2937 0%, #0f172a 50%, #030712 100%);
            color: var(--text);
            display: flex;
            justify-content: center;
            padding: clamp(1.5rem, 3vw + 1rem, 4rem);
        }}

        main {{
            width: min(1100px, 100%);
            backdrop-filter: blur(18px);
            background: var(--panel);
            border-radius: 28px;
            padding: clamp(1.75rem, 2.5vw + 1rem, 3rem);
            box-shadow: var(--shadow);
        }}

        header {{
            text-align: center;
            margin-bottom: 2rem;
        }}

        header h1 {{
            margin: 0;
            font-size: clamp(2rem, 4vw, 2.75rem);
            letter-spacing: -0.02em;
        }}

        header p {{
            margin: 0.75rem auto 1rem auto;
            max-width: 680px;
            color: var(--muted);
            font-size: 1.05rem;
        }}

        .nav-link {{
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
        }}

        .nav-link:hover {{
            border-color: var(--accent);
            transform: translateY(-2px);
        }}

        section {{
            margin-top: 2rem;
            padding-top: 2rem;
            border-top: 1px solid rgba(148, 163, 184, 0.16);
        }}

        section:first-of-type {{
            border-top: none;
            padding-top: 0;
            margin-top: 0;
        }}

        h2 {{
            margin: 0 0 1rem 0;
            font-size: clamp(1.5rem, 2.5vw, 1.85rem);
            color: var(--accent);
        }}

        h3 {{
            margin: 1.5rem 0 0.75rem 0;
            font-size: 1.25rem;
        }}

        .info-box {{
            background: rgba(15, 23, 42, 0.6);
            border: 1px solid rgba(148, 163, 184, 0.18);
            border-radius: 16px;
            padding: 1.25rem;
            margin: 1rem 0;
        }}

        .info-box.success {{
            border-color: rgba(16, 185, 129, 0.3);
            background: rgba(16, 185, 129, 0.05);
        }}

        .info-box.warning {{
            border-color: rgba(245, 158, 11, 0.3);
            background: rgba(245, 158, 11, 0.05);
        }}

        .cookie-list {{
            display: grid;
            gap: 0.75rem;
            margin: 1rem 0;
        }}

        .cookie-item {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.12);
            border-radius: 12px;
            padding: 1rem;
        }}

        .cookie-item strong {{
            color: var(--accent);
        }}

        .empty-state {{
            text-align: center;
            color: var(--muted);
            padding: 2rem;
            font-style: italic;
        }}

        form {{
            display: grid;
            gap: 0.75rem;
            margin: 1rem 0;
        }}

        .form-row {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 0.75rem;
        }}

        input[type="text"] {{
            background: rgba(15, 23, 42, 0.6);
            color: var(--text);
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 12px;
            padding: 0.7rem 0.9rem;
            font-size: 0.95rem;
            width: 100%;
        }}

        input[type="text"]:focus {{
            outline: none;
            border-color: var(--accent);
        }}

        button {{
            border: none;
            border-radius: 999px;
            padding: 0.7rem 1.4rem;
            font-weight: 600;
            letter-spacing: 0.01em;
            cursor: pointer;
            transition: transform 0.18s ease, box-shadow 0.18s ease;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 0.4rem;
        }}

        button.primary {{
            background: linear-gradient(120deg, var(--accent), var(--accent-strong));
            color: #04111f;
        }}

        button.success {{
            background: linear-gradient(120deg, #10b981, #059669);
            color: #04111f;
        }}

        button.danger {{
            background: linear-gradient(120deg, #ef4444, #dc2626);
            color: white;
        }}

        button:hover {{
            transform: translateY(-1px);
            box-shadow: 0 10px 25px rgba(14, 165, 233, 0.3);
        }}

        .technical-info {{
            background: rgba(15, 23, 42, 0.8);
            border: 1px solid rgba(148, 163, 184, 0.12);
            border-radius: 14px;
            padding: 1rem;
            font-family: "JetBrains Mono", "Fira Mono", ui-monospace, monospace;
            font-size: 0.85rem;
            overflow-x: auto;
        }}

        .technical-info dt {{
            color: var(--accent);
            font-weight: 600;
            margin-top: 0.5rem;
        }}

        .technical-info dt:first-child {{
            margin-top: 0;
        }}

        .technical-info dd {{
            margin: 0.25rem 0 0 0;
            color: var(--muted);
            word-break: break-all;
        }}

        .flow-diagram {{
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.12);
            border-radius: 14px;
            padding: 1.5rem;
            margin: 1.5rem 0;
            font-family: "JetBrains Mono", "Fira Mono", ui-monospace, monospace;
            font-size: 0.9rem;
            line-height: 1.8;
        }}

        .flow-step {{
            margin: 0.5rem 0;
            padding-left: 1.5rem;
            position: relative;
        }}

        .flow-step::before {{
            content: "→";
            position: absolute;
            left: 0;
            color: var(--accent);
            font-weight: bold;
        }}

        code {{
            background: rgba(56, 189, 248, 0.16);
            border-radius: 0.4rem;
            padding: 0.15rem 0.4rem;
            color: #bae6fd;
            font-family: "JetBrains Mono", "Fira Mono", ui-monospace, monospace;
            font-size: 0.9em;
        }}

        ol, ul {{
            line-height: 1.7;
            color: var(--muted);
        }}

        ol li, ul li {{
            margin: 0.5rem 0;
        }}

        @media (max-width: 640px) {{
            body {{
                padding: 1rem;
            }}

            main {{
                padding: 1.4rem;
                border-radius: 18px;
            }}

            .form-row {{
                grid-template-columns: 1fr;
            }}
        }}
    </style>
</head>
<body>
<main>
    <header>
        <h1>🍪 Cookie Management Demo</h1>
        <p>Interactive demonstration of HTTP cookie handling through our C++98 webserver and Python CGI scripts.</p>
        <a class="nav-link" href="/">← Back to main playground</a>
    </header>

    <section>
        <h2>Current Cookies in Your Browser</h2>
        <div class="cookie-list">""")

    if cookies:
        for name, value in cookies.items():
            print(f"""
            <div class="cookie-item">
                <div>
                    <strong>{name}:</strong> <span>{value}</span>
                </div>
                <a href="?action=delete&name={name}">
                    <button class="danger" type="button">Delete</button>
                </a>
            </div>""")

        # Special handling for visit tracking
        if 'visit_count' in cookies:
            last_visit = cookies.get('last_visit', 'Unknown')
            if last_visit.isdigit():
                last_visit = time.ctime(int(last_visit))
            print(f"""
        </div>
        <div class="info-box success">
            <strong>📈 Visit Tracking Active:</strong> You've visited this page <strong>{cookies['visit_count']}</strong> times.
            Last visit: {last_visit}
        </div>""")
        else:
            print("</div>")
    else:
        print("""
            <div class="empty-state">
                <p>No cookies are currently set. Use the forms below to create some!</p>
            </div>
        </div>""")

    print(f"""
    </section>

    <section>
        <h2>Cookie Actions</h2>

        <h3>Set a Custom Cookie</h3>
        <form method="GET">
            <input type="hidden" name="action" value="set">
            <div class="form-row">
                <input type="text" name="name" placeholder="Cookie name (e.g., username)" required>
                <input type="text" name="value" placeholder="Cookie value (e.g., alice)" required>
            </div>
            <button class="primary" type="submit">🍪 Set Cookie</button>
        </form>

        <h3>Track Your Visits</h3>
        <form method="GET">
            <input type="hidden" name="action" value="visit">
            <button class="success" type="submit">📊 Increment Visit Counter</button>
        </form>

        <h3>Refresh Page</h3>
        <form method="GET">
            <button class="primary" type="submit">🔄 Reload & View Cookies</button>
        </form>
    </section>

    <section>
        <h2>How Cookies Work With Our Server</h2>

        <div class="info-box">
            <p><strong>Cookies are small pieces of data that websites store in your browser.</strong> They enable features like login sessions, shopping carts, and user preferences by maintaining state across multiple HTTP requests.</p>
        </div>

        <h3>The Complete Cookie Flow</h3>
        <div class="flow-diagram">
            <div class="flow-step">Browser makes request to /cgi/cookies.py</div>
            <div class="flow-step">Browser includes: Cookie: name1=value1; name2=value2</div>
            <div class="flow-step">C++ Server receives HTTP request</div>
            <div class="flow-step">Server parses Cookie: header in GetRequest.cpp</div>
            <div class="flow-step">Server stores cookies in _cookie map</div>
            <div class="flow-step">Server sets HTTP_COOKIE environment variable</div>
            <div class="flow-step">Server executes Python CGI script via fork/exec</div>
            <div class="flow-step">CGI reads os.environ.get('HTTP_COOKIE')</div>
            <div class="flow-step">CGI parses cookie string into dictionary</div>
            <div class="flow-step">CGI processes your action (set/delete/visit)</div>
            <div class="flow-step">CGI outputs: Set-Cookie: name=value; Path=/</div>
            <div class="flow-step">Server forwards Set-Cookie header to browser</div>
            <div class="flow-step">Browser stores the cookie for future requests</div>
        </div>

        <h3>Implementation Details</h3>

        <div class="info-box">
            <h4>1. Server-Side (C++)</h4>
            <p>Our C++ HTTP server handles cookies at the protocol level:</p>
            <ul>
                <li><strong>Parsing incoming cookies:</strong> GetRequest.cpp, PostRequest.cpp, and DeleteRequest.cpp all parse the <code>Cookie:</code> header from browser requests</li>
                <li><strong>Storing cookies:</strong> Parsed cookies are stored in a <code>std::map&lt;std::string, std::string&gt; _cookie</code></li>
                <li><strong>CGI environment:</strong> HttpRequest.cpp converts cookies to the <code>HTTP_COOKIE</code> environment variable for CGI scripts</li>
                <li><strong>Forwarding responses:</strong> CgiHandler.cpp reads CGI output and forwards <code>Set-Cookie:</code> headers back to the browser unchanged</li>
            </ul>
        </div>

        <div class="info-box">
            <h4>2. Application-Side (Python CGI)</h4>
            <p>This CGI script implements the business logic for cookie operations:</p>
            <ul>
                <li><strong>Reading cookies:</strong> <code>os.environ.get('HTTP_COOKIE')</code> retrieves the cookie string from the server</li>
                <li><strong>Parsing:</strong> Split by semicolon, then by equals sign to create a dictionary</li>
                <li><strong>Setting cookies:</strong> Output <code>Set-Cookie: name=value; Path=/</code> header before the blank line</li>
                <li><strong>Deleting cookies:</strong> Set expiration date to the past: <code>Expires=Thu, 01 Jan 1970 00:00:00 GMT</code></li>
                <li><strong>Cookie attributes:</strong> <code>Path=/</code> makes the cookie available site-wide</li>
            </ul>
        </div>

        <div class="info-box warning">
            <h4>🔍 Key Architectural Decision</h4>
            <p><strong>Why does the CGI generate cookies instead of the C++ server?</strong></p>
            <p>This follows the standard separation of concerns in web architecture:</p>
            <ol>
                <li><strong>Server layer (C++):</strong> Handles the HTTP protocol - parsing requests, routing, executing CGI, forwarding responses</li>
                <li><strong>Application layer (CGI):</strong> Handles business logic - when to set cookies, what values to use, session management</li>
            </ol>
            <p>This is exactly how Apache, Nginx, and all other web servers work. The server is a generic HTTP handler; applications make decisions about cookies, sessions, and state management.</p>
        </div>

        <h3>Cookie Security & Attributes</h3>
        <div class="info-box">
            <p>Real production cookies use additional security attributes:</p>
            <ul>
                <li><code>HttpOnly</code> - Prevents JavaScript access (XSS protection)</li>
                <li><code>Secure</code> - Only sent over HTTPS</li>
                <li><code>SameSite</code> - Controls cross-site request behavior (CSRF protection)</li>
                <li><code>Max-Age</code> or <code>Expires</code> - Controls cookie lifetime</li>
                <li><code>Domain</code> - Controls which domains can receive the cookie</li>
                <li><code>Path</code> - Controls which URL paths can receive the cookie</li>
            </ul>
        </div>
    </section>

    <section>
        <h2>Technical Information</h2>
        <dl class="technical-info">
            <dt>HTTP_COOKIE (from browser):</dt>
            <dd>{cookie_header if cookie_header else '(empty - no cookies sent)'}</dd>

            <dt>QUERY_STRING (your action):</dt>
            <dd>{query_string if query_string else '(empty - no action taken)'}</dd>

            <dt>Action Performed:</dt>
            <dd>{action if action else 'None - just viewing cookies'}</dd>

            <dt>REQUEST_METHOD:</dt>
            <dd>{os.environ.get('REQUEST_METHOD', 'N/A')}</dd>

            <dt>SCRIPT_NAME:</dt>
            <dd>{os.environ.get('SCRIPT_NAME', 'N/A')}</dd>

            <dt>SERVER_SOFTWARE:</dt>
            <dd>{os.environ.get('SERVER_SOFTWARE', 'N/A')}</dd>
        </dl>
    </section>

    <section>
        <h2>Try These Actions</h2>
        <div class="info-box">
            <ul>
                <li>Set a cookie named "username" with your name - it will persist across page refreshes</li>
                <li>Use "Track Visit" to see a counter increment each time you visit</li>
                <li>Delete any cookie and see it disappear from your browser</li>
                <li>Open your browser's Developer Tools (F12) → Application/Storage → Cookies to see them directly</li>
                <li>Compare with the session demo at <a href="/cgi/session.py" style="color: var(--accent);">/cgi/session.py</a> which uses cookies for server-side session management</li>
            </ul>
        </div>
    </section>

    <footer style="margin-top: 3rem; text-align: center; color: var(--muted); font-size: 0.9rem;">
        <p>Cookie implementation by Mari & Yang - Part of the 42 Webserv Project</p>
        <p>C++98 HTTP Server with Python CGI • No external libraries • Full HTTP/1.1 support</p>
    </footer>
</main>
</body>
</html>""")

if __name__ == "__main__":
    main()
