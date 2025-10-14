# WEBSERV EVALUATION TESTING GUIDE

This guide contains all command lines needed to thoroughly test a webserv implementation according to the 42 evaluation sheet.

---

## TABLE OF CONTENTS
1. [Pre-Evaluation Checks](#pre-evaluation-checks)
2. [Code Review - Critical I/O Multiplexing](#code-review---critical-io-multiplexing)
3. [Configuration File Tests](#configuration-file-tests)
4. [Basic HTTP Method Tests](#basic-http-method-tests)
5. [File Upload & Retrieval](#file-upload--retrieval)
6. [CGI Tests](#cgi-tests)
7. [Stress & Resilience Tests](#stress--resilience-tests)
8. [Telnet Manual Testing](#telnet-manual-testing)
9. [Status Code Verification](#status-code-verification)
10. [Bonus Tests](#bonus-tests)
11. [Evaluation Questions](#evaluation-questions)

---

## PRE-EVALUATION CHECKS

### Verify Git Repository
```bash
git log --oneline | head -5
```

### Check for Forbidden Functions
```bash
grep -r "execve" srcs/ includes/
```

### Compile and Check for Re-link Issues
```bash
make
make  # Should say "Nothing to be done"
```

### Check for Memory Leaks
```bash
valgrind --leak-check=full --track-fds=yes ./webserv [config_file]
```
*Run server, perform some requests, then Ctrl+C to check for leaks*

---

## CODE REVIEW - CRITICAL I/O MULTIPLEXING

**⚠️ THESE ARE AUTOMATIC FAIL (GRADE 0) IF WRONG ⚠️**

### Check Only ONE select/poll/epoll in Main Loop
```bash
grep -n "select\|poll\|epoll_wait" srcs/*.cpp
```

### Verify errno is NEVER Used After Socket Operations
```bash
grep -A 5 "read\|recv\|write\|send" srcs/*.cpp | grep errno
```
**If errno is found after socket operations → GRADE 0**

### Check All Socket Operations Check Return Values Properly
```bash
grep -A 2 "read\|recv\|write\|send" srcs/*.cpp
```
**Must check both -1 AND other values, not just -1 or 0**

### Manual Code Review Checklist
- [ ] Only ONE select/poll/epoll in the main loop
- [ ] Select monitors both READ and WRITE at the same time
- [ ] Only ONE read OR write per client per select call
- [ ] All read/recv/write/send check for errors and remove client on failure
- [ ] NO errno checking after socket operations
- [ ] NO file operations without going through select/poll

---

## CONFIGURATION FILE TESTS

**Assume config file is at `config/default.conf`**

### Multiple Servers with Different Ports
```bash
curl http://localhost:8080
curl http://localhost:8081
curl http://localhost:8082
```

### Multiple Servers with Different Hostnames
```bash
curl --resolve example.com:8080:127.0.0.1 http://example.com:8080
curl --resolve test.com:8080:127.0.0.1 http://test.com:8080
```

### Custom Error Pages (Test 404)
```bash
curl -i http://localhost:8080/nonexistent_page
```

### Client Body Size Limit - Test UNDER Limit
```bash
curl -X POST -H "Content-Type: text/plain" --data "small body" http://localhost:8080/upload
```

### Client Body Size Limit - Test OVER Limit (Should Get 413)
```bash
curl -X POST -H "Content-Type: text/plain" --data "$(python3 -c 'print("A"*100000)')" http://localhost:8080/upload
```

### Routes to Different Directories
```bash
curl http://localhost:8080/images/
curl http://localhost:8080/uploads/
```

### Default Index File for Directories
```bash
curl http://localhost:8080/
```
*Should serve index.html or configured default*

### Accepted Methods Per Route - Should Work
```bash
curl -X GET http://localhost:8080/files/test.txt
```

### Accepted Methods Per Route - Should Fail (405)
```bash
curl -X DELETE http://localhost:8080/protected/file.txt
```

---

## BASIC HTTP METHOD TESTS

### GET Requests
```bash
curl -i http://localhost:8080/
curl -i http://localhost:8080/index.html
curl -i http://localhost:8080/images/logo.png
```

### POST Requests - With File
```bash
echo "test data" > test.txt
curl -X POST -H "Content-Type: text/plain" --data-binary @test.txt http://localhost:8080/upload
```

### POST Requests - Form Data
```bash
curl -X POST -d "name=test&value=123" http://localhost:8080/form
```

### DELETE Requests
```bash
curl -X DELETE http://localhost:8080/uploads/test.txt
curl -i -X DELETE http://localhost:8080/uploads/test.txt
```

### UNKNOWN/Invalid Methods (Should NOT Crash)
```bash
curl -i -X INVALID http://localhost:8080/
curl -i -X HACK http://localhost:8080/
```

### UNKNOWN Method with Telnet
```bash
telnet localhost 8080
```
*Then type:* `BADMETHOD / HTTP/1.1` *[Enter][Enter]*

---

## FILE UPLOAD & RETRIEVAL

### Create Test File
```bash
echo "This is a test upload file" > upload_test.txt
```

### Upload File
```bash
curl -X POST -F "file=@upload_test.txt" http://localhost:8080/upload
```

### Retrieve Uploaded File
```bash
curl http://localhost:8080/uploads/upload_test.txt
```

### Upload Larger File (5MB)
```bash
dd if=/dev/urandom of=large_file.bin bs=1M count=5
curl -X POST -F "file=@large_file.bin" http://localhost:8080/upload
```

### Upload Multiple Files
```bash
curl -X POST -F "file1=@test1.txt" -F "file2=@test2.txt" http://localhost:8080/upload
```

---

## CGI TESTS

### Test Basic CGI (PHP)
*First create test.php with:* `<?php echo "Hello from CGI"; ?>`
```bash
curl http://localhost:8080/cgi-bin/test.php
```

### GET Method with Query String
```bash
curl "http://localhost:8080/cgi-bin/script.php?name=test&value=42"
```

### POST Method to CGI
```bash
curl -X POST -d "username=test&password=secret" http://localhost:8080/cgi-bin/login.php
```

### Python CGI (if supported)
```bash
curl http://localhost:8080/cgi-bin/test.py
```

### Check CGI Environment Variables
*Create env.php:* `<?php phpinfo(); ?>`
```bash
curl http://localhost:8080/cgi-bin/env.php | grep -i "path_info\|request_method"
```

### Test Chunked Request to CGI
```bash
curl -X POST -H "Transfer-Encoding: chunked" --data-binary @file.txt http://localhost:8080/cgi-bin/process.php
```

---

## STRESS & RESILIENCE TESTS

### Concurrent Connections (Using Siege)
*Install siege:* `brew install siege`
```bash
siege -c 50 -t 30s http://localhost:8080/
```

### Rapid Requests (100 Concurrent)
```bash
for i in {1..100}; do curl http://localhost:8080/ & done
```

### Slow Client (Connection Should Timeout)
```bash
telnet localhost 8080
```
*Type nothing for 60+ seconds - server should handle timeout gracefully*

### Incomplete Requests
```bash
echo -e "GET / HTTP/1.1\r\n" | nc localhost 8080
```
*Wait 30 seconds - server should timeout properly*

### Malformed Requests (Should NOT Crash)
```bash
echo -e "INVALID REQUEST\r\n\r\n" | nc localhost 8080
echo -e "GET\r\n\r\n" | nc localhost 8080
echo -e "GET / HTTP/9.9\r\n\r\n" | nc localhost 8080
```

### Very Long URL
```bash
curl -i "http://localhost:8080/$(python3 -c 'print("a"*10000)')"
```

### Very Long Headers
```bash
curl -i -H "X-Custom-Header: $(python3 -c 'print("A"*10000)')" http://localhost:8080/
```

### Multiple Requests on Same Connection (Keep-Alive)
```bash
(echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n") | nc localhost 8080
```

---

## TELNET MANUAL TESTING

### Connect to Server
```bash
telnet localhost 8080
```

### Basic GET Request
```
GET / HTTP/1.1
Host: localhost

[Press Enter twice]
```

### GET with Headers
```
GET /index.html HTTP/1.1
Host: localhost
User-Agent: telnet-test
Accept: text/html

[Press Enter twice]
```

### POST Request
```
POST /upload HTTP/1.1
Host: localhost
Content-Type: text/plain
Content-Length: 11

Hello World
```

### Invalid Request
```
GARBAGE
[Press Enter twice]
```

---

## STATUS CODE VERIFICATION

### 200 OK
```bash
curl -i http://localhost:8080/existing_file.html | head -1
```

### 201 Created (After POST)
```bash
curl -i -X POST -d "data" http://localhost:8080/upload | head -1
```

### 204 No Content (After DELETE)
```bash
curl -i -X DELETE http://localhost:8080/file.txt | head -1
```

### 301/302 Redirect (If Configured)
```bash
curl -i http://localhost:8080/old_page | head -1
```

### 400 Bad Request
```bash
echo -e "BAD\r\n\r\n" | nc localhost 8080 | head -1
```

### 403 Forbidden
```bash
curl -i http://localhost:8080/forbidden_directory/ | head -1
```

### 404 Not Found
```bash
curl -i http://localhost:8080/doesnotexist.html | head -1
```

### 405 Method Not Allowed
```bash
curl -i -X DELETE http://localhost:8080/protected_route | head -1
```

### 413 Payload Too Large
```bash
curl -i -X POST --data "$(python3 -c 'print("X"*100000)')" http://localhost:8080/ | head -1
```

### 500 Internal Server Error
```bash
curl -i http://localhost:8080/cgi-bin/broken_script.php | head -1
```

### 501 Not Implemented
```bash
curl -i -X TRACE http://localhost:8080/ | head -1
```

### 505 HTTP Version Not Supported
```bash
echo -e "GET / HTTP/2.0\r\n\r\n" | nc localhost 8080 | head -1
```

---

## BONUS TESTS

### Cookies and Session Management - Login
```bash
curl -i http://localhost:8080/login
```
*Check for Set-Cookie header*

### Cookies and Session Management - Use Cookie
```bash
curl -i -b "session=abc123" http://localhost:8080/profile
```

### Multiple CGI Types - PHP
```bash
curl http://localhost:8080/cgi-bin/test.php
```

### Multiple CGI Types - Python
```bash
curl http://localhost:8080/cgi-bin/test.py
```

### Multiple CGI Types - Perl
```bash
curl http://localhost:8080/cgi-bin/test.pl
```

---

## EVALUATION QUESTIONS

Ask these questions while running tests:

1. **"Explain how your select/poll implementation works"**
2. **"Show me the code path from select to read/write"**
3. **"Why did you choose select vs poll vs epoll?"**
4. **"How do you handle partial reads/writes?"**
5. **"What happens when a client disconnects mid-request?"**
6. **"How do you handle chunked encoding?"**
7. **"Explain your CGI implementation - how does PATH_INFO work?"**
8. **"How does your server handle the request/response cycle?"**
9. **"What happens if select returns an error?"**
10. **"How do you determine when a request is complete?"**

---

## CRITICAL REMINDERS

- ⚠️ **errno checking after socket operations = AUTOMATIC FAIL**
- ⚠️ **Multiple select/poll calls = AUTOMATIC FAIL**
- ⚠️ **File operations without select/poll = AUTOMATIC FAIL**
- ⚠️ **Not checking both read AND write in select = AUTOMATIC FAIL**
- ✅ **All HTTP status codes must be accurate**
- ✅ **Server must NEVER crash or hang**
- ✅ **Compare behavior with NGINX when uncertain**

---

## NOTES

- Replace `localhost:8080` with the actual host:port from their config
- Create test files (test.txt, upload_test.txt, etc.) before running upload tests
- For CGI tests, ensure php-cgi or python is installed and configured
- Check that the server remains running after each test
- Verify no memory leaks throughout the evaluation

Good luck with your evaluation!
