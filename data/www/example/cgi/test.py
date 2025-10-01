#!/usr/bin/python3

import os
import sys

print("Content-Type: text/html; charset=UTF-8")
print()

print("<!DOCTYPE html>")
print("<html><head><meta charset='UTF-8'><title>Python CGI</title></head><body>")
print("<h1>🐍 Python CGI Works!</h1>")
print("<p><strong>Method:</strong> " + os.environ.get('REQUEST_METHOD', 'N/A') + "</p>")
print("<p><strong>Query:</strong> " + os.environ.get('QUERY_STRING', 'N/A') + "</p>")
print("<p><strong>Path Info:</strong> " + os.environ.get('PATH_INFO', 'N/A') + "</p>")

if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = os.environ.get('CONTENT_LENGTH', '0')
    if content_length.isdigit() and int(content_length) > 0:
        post_data = sys.stdin.read(int(content_length))
        print("<h2>POST Data Received:</h2>")
        print("<pre>" + post_data + "</pre>")

print("</body></html>")
