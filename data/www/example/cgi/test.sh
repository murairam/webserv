#!/bin/bash

# Force output to be line-buffered and sync immediately
exec 1>&1

{
    echo "Content-Type: text/html; charset=UTF-8"
    echo ""
    echo "<!DOCTYPE html>"
    echo "<html>"
    echo "<head><meta charset='UTF-8'><title>Shell CGI</title></head>"
    echo "<body>"
    echo "<h1>🐚 Shell CGI Works!</h1>"
    echo "<p><strong>Method:</strong> ${REQUEST_METHOD:-N/A}</p>"
    echo "<p><strong>Query:</strong> ${QUERY_STRING:-N/A}</p>"
    echo "<p><strong>Path Info:</strong> ${PATH_INFO:-N/A}</p>"
    echo "<p><strong>Script:</strong> ${SCRIPT_NAME:-N/A}</p>"
    echo "<p><strong>Server:</strong> ${SERVER_SOFTWARE:-N/A}</p>"
    echo "</body>"
    echo "</html>"
} | cat

# Ensure everything is flushed
sync
