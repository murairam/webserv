#!/bin/bash

# Ultra-minimal shell login - same pattern as working test.sh
exec 1>&1

{
    echo "Content-Type: text/html; charset=UTF-8"

    # Handle actions first, then determine user
    if [[ "$QUERY_STRING" == *"action=logout"* ]]; then
        echo "Set-Cookie: username=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT"
        USER="Guest"
    elif [[ "$QUERY_STRING" == *"action=login"* ]]; then
        NEW_USER=$(echo "$QUERY_STRING" | sed -n 's/.*name=\([^&]*\).*/\1/p' | sed 's/%20/ /g')
        if [[ -n "$NEW_USER" ]]; then
            echo "Set-Cookie: username=$NEW_USER; Path=/"
            USER="$NEW_USER"
        else
            USER="Guest"
        fi
    else
        # No action, check existing cookie
        if [[ "$HTTP_COOKIE" == *"username="* ]]; then
            USER=$(echo "$HTTP_COOKIE" | sed -n 's/.*username=\([^;]*\).*/\1/p')
        else
            USER="Guest"
        fi
    fi

    echo ""
    echo "<!DOCTYPE html>"
    echo "<html><head><meta charset='UTF-8'><title>Shell Login</title></head><body>"
    echo "<h1>🐚 Shell Login Demo</h1>"
    echo "<p><strong>User:</strong> $USER</p>"

    if [[ "$USER" == "Guest" ]]; then
        echo "<form method='GET'><input name='action' type='hidden' value='login'><input name='name' placeholder='Name' required><button>Login</button></form>"
    else
        echo "<form method='GET'><input name='action' type='hidden' value='logout'><button>Logout</button></form>"
    fi

    echo "<p>Cookie: ${HTTP_COOKIE:-None}</p>"
    echo "</body></html>"
} | cat

sync
