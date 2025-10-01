#!/usr/bin/python3

import os
import sys
import time
import hashlib
import json
from urllib.parse import parse_qs

# Simple session storage (in production, use a database)
SESSION_DIR = "./sessions"
SESSION_TIMEOUT = 3600  # 1 hour

def ensure_session_dir():
    if not os.path.exists(SESSION_DIR):
        os.makedirs(SESSION_DIR)

def generate_session_id():
    """Generate a unique session ID"""
    timestamp = str(time.time())
    random_data = str(os.urandom(16))
    return hashlib.md5((timestamp + random_data).encode()).hexdigest()

def get_session_file(session_id):
    """Get session file path"""
    return os.path.join(SESSION_DIR, f"sess_{session_id}.json")

def load_session(session_id):
    """Load session data from file"""
    if not session_id:
        return None

    session_file = get_session_file(session_id)
    if not os.path.exists(session_file):
        return None

    try:
        with open(session_file, 'r') as f:
            session_data = json.load(f)

        # Check if session has expired
        if time.time() - session_data.get('created', 0) > SESSION_TIMEOUT:
            os.remove(session_file)
            return None

        return session_data
    except:
        return None

def save_session(session_id, data):
    """Save session data to file"""
    ensure_session_dir()
    session_file = get_session_file(session_id)
    with open(session_file, 'w') as f:
        json.dump(data, f)

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
    # Parse cookies from HTTP_COOKIE environment variable
    cookie_header = os.environ.get('HTTP_COOKIE', '')
    cookies = parse_cookies(cookie_header)

    # Get existing session or create new one
    session_id = cookies.get('SESSIONID')
    session_data = load_session(session_id)

    # Parse query parameters
    query_string = os.environ.get('QUERY_STRING', '')
    params = parse_qs(query_string)

    # Handle actions
    action = params.get('action', [''])[0]

    if not session_data:
        # Create new session
        session_id = generate_session_id()
        session_data = {
            'created': time.time(),
            'visit_count': 0,
            'user_name': 'Guest',
            'last_action': 'Session created'
        }

    # Update session based on action
    session_data['visit_count'] += 1
    session_data['last_visit'] = time.time()

    if action == 'login' and 'name' in params:
        session_data['user_name'] = params['name'][0]
        session_data['last_action'] = f"Logged in as {params['name'][0]}"
    elif action == 'logout':
        session_data['user_name'] = 'Guest'
        session_data['last_action'] = 'Logged out'
    elif action == 'increment':
        session_data['counter'] = session_data.get('counter', 0) + 1
        session_data['last_action'] = 'Incremented counter'

    # Save session
    save_session(session_id, session_data)

    # Output HTTP headers
    print("Content-Type: text/html; charset=UTF-8")
    print(f"Set-Cookie: SESSIONID={session_id}; Path=/; Max-Age={SESSION_TIMEOUT}")
    print()  # Empty line to end headers

    # Output HTML
    print(f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>🍪 Session Management Demo</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }}
        .container {{ background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .session-info {{ background: #e8f4fd; padding: 15px; border-radius: 5px; margin: 15px 0; }}
        .actions {{ background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 15px 0; }}
        button {{ background: #007bff; color: white; border: none; padding: 8px 16px; margin: 5px; border-radius: 4px; cursor: pointer; }}
        button:hover {{ background: #0056b3; }}
        input {{ padding: 8px; margin: 5px; border: 1px solid #ddd; border-radius: 4px; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🍪 Cookie & Session Management Demo</h1>

        <div class="session-info">
            <h3>📊 Session Information</h3>
            <p><strong>Session ID:</strong> {session_id[:16]}...</p>
            <p><strong>User:</strong> {session_data['user_name']}</p>
            <p><strong>Visit Count:</strong> {session_data['visit_count']}</p>
            <p><strong>Counter:</strong> {session_data.get('counter', 0)}</p>
            <p><strong>Last Action:</strong> {session_data['last_action']}</p>
            <p><strong>Session Created:</strong> {time.ctime(session_data['created'])}</p>
        </div>

        <div class="actions">
            <h3>🎮 Actions</h3>

            <form method="GET" style="display: inline;">
                <input type="hidden" name="action" value="login">
                <input type="text" name="name" placeholder="Enter your name" required>
                <button type="submit">🔑 Login</button>
            </form>

            <form method="GET" style="display: inline;">
                <input type="hidden" name="action" value="logout">
                <button type="submit">🚪 Logout</button>
            </form>

            <form method="GET" style="display: inline;">
                <input type="hidden" name="action" value="increment">
                <button type="submit">➕ Increment Counter</button>
            </form>

            <form method="GET" style="display: inline;">
                <button type="submit">🔄 Refresh Page</button>
            </form>
        </div>

        <div class="session-info">
            <h3>🔧 Technical Details</h3>
            <p><strong>HTTP_COOKIE:</strong> {cookie_header or 'No cookies'}</p>
            <p><strong>QUERY_STRING:</strong> {query_string or 'No query parameters'}</p>
            <p><strong>REQUEST_METHOD:</strong> {os.environ.get('REQUEST_METHOD', 'N/A')}</p>
        </div>

        <p><em>This demonstrates cookie-based session management with your webserver!</em></p>
    </div>
</body>
</html>""")

if __name__ == "__main__":
    main()
