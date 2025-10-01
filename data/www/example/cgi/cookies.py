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
<html>
<head>
    <meta charset="UTF-8">
    <title>🍪 Cookie Demo</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }}
        .container {{ background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .cookie-list {{ background: #fff3cd; padding: 15px; border-radius: 5px; margin: 15px 0; }}
        .actions {{ background: #d4edda; padding: 15px; border-radius: 5px; margin: 15px 0; }}
        button {{ background: #28a745; color: white; border: none; padding: 8px 16px; margin: 5px; border-radius: 4px; cursor: pointer; }}
        button.delete {{ background: #dc3545; }}
        input {{ padding: 8px; margin: 5px; border: 1px solid #ddd; border-radius: 4px; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🍪 Cookie Management Demo</h1>

        <div class="cookie-list">
            <h3>📋 Current Cookies</h3>""")

    if cookies:
        for name, value in cookies.items():
            print(f"""
            <p><strong>{name}:</strong> {value}
                <a href="?action=delete&name={name}">
                    <button class="delete" type="button">❌ Delete</button>
                </a>
            </p>""")
    else:
        print("<p><em>No cookies set</em></p>")

    # Special handling for visit tracking
    if 'visit_count' in cookies:
        last_visit = cookies.get('last_visit', 'Unknown')
        if last_visit.isdigit():
            last_visit = time.ctime(int(last_visit))
        print(f"<p><strong>📈 Visit Tracking:</strong> You've visited {cookies['visit_count']} times. Last visit: {last_visit}</p>")

    print(f"""
        </div>

        <div class="actions">
            <h3>🎮 Cookie Actions</h3>

            <form method="GET" style="margin: 10px 0;">
                <h4>Set Cookie:</h4>
                <input type="hidden" name="action" value="set">
                <input type="text" name="name" placeholder="Cookie name" required>
                <input type="text" name="value" placeholder="Cookie value" required>
                <button type="submit">🍪 Set Cookie</button>
            </form>

            <form method="GET" style="margin: 10px 0;">
                <input type="hidden" name="action" value="visit">
                <button type="submit">📊 Track Visit</button>
            </form>

            <form method="GET" style="margin: 10px 0;">
                <button type="submit">🔄 Refresh</button>
            </form>
        </div>

        <div class="cookie-list">
            <h3>🔧 Technical Info</h3>
            <p><strong>HTTP_COOKIE:</strong> {cookie_header or 'No cookies'}</p>
            <p><strong>QUERY_STRING:</strong> {query_string or 'No parameters'}</p>
            <p><strong>Action Result:</strong> {action or 'None'}</p>
        </div>

        <p><em>This demonstrates basic cookie operations with your webserver!</em></p>
    </div>
</body>
</html>""")

if __name__ == "__main__":
    main()
