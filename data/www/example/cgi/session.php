<?php
header("Content-Type: text/html; charset=UTF-8");

// Start session (creates session cookie automatically)
session_start();

// Handle actions
$action = $_GET['action'] ?? '';

if ($action === 'login' && !empty($_GET['username'])) {
    $_SESSION['username'] = $_GET['username'];
    $_SESSION['login_time'] = time();
} elseif ($action === 'logout') {
    session_destroy();
    session_start(); // Start fresh session
} elseif ($action === 'set_counter') {
    $_SESSION['counter'] = ($_SESSION['counter'] ?? 0) + 1;
}

// Set some demonstration cookies
if ($action === 'set_cookie') {
    setcookie('demo_cookie', 'Hello from PHP!', time() + 3600, '/');
    setcookie('timestamp', time(), time() + 3600, '/');
}

$username = $_SESSION['username'] ?? 'Guest';
$counter = $_SESSION['counter'] ?? 0;
$login_time = $_SESSION['login_time'] ?? null;
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>🐘 PHP Sessions & Cookies</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .session-info { background: #e7f3ff; padding: 15px; border-radius: 5px; margin: 15px 0; }
        .cookie-info { background: #fff2e7; padding: 15px; border-radius: 5px; margin: 15px 0; }
        .actions { background: #e7ffe7; padding: 15px; border-radius: 5px; margin: 15px 0; }
        button { background: #007bff; color: white; border: none; padding: 8px 16px; margin: 5px; border-radius: 4px; cursor: pointer; }
        button.logout { background: #dc3545; }
        button.cookie { background: #fd7e14; }
        input { padding: 8px; margin: 5px; border: 1px solid #ddd; border-radius: 4px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🐘 PHP Session & Cookie Demo</h1>

        <div class="session-info">
            <h3>📊 Session Information</h3>
            <p><strong>Session ID:</strong> <?= session_id() ?></p>
            <p><strong>Username:</strong> <?= htmlspecialchars($username) ?></p>
            <p><strong>Counter:</strong> <?= $counter ?></p>
            <?php if ($login_time): ?>
                <p><strong>Login Time:</strong> <?= date('Y-m-d H:i:s', $login_time) ?></p>
            <?php endif; ?>
            <p><strong>Session Data:</strong></p>
            <pre><?= htmlspecialchars(print_r($_SESSION, true)) ?></pre>
        </div>

        <div class="cookie-info">
            <h3>🍪 Cookie Information</h3>
            <?php if (!empty($_COOKIE)): ?>
                <?php foreach ($_COOKIE as $name => $value): ?>
                    <p><strong><?= htmlspecialchars($name) ?>:</strong> <?= htmlspecialchars($value) ?></p>
                <?php endforeach; ?>
            <?php else: ?>
                <p><em>No cookies found</em></p>
            <?php endif; ?>
        </div>

        <div class="actions">
            <h3>🎮 Actions</h3>

            <form method="GET" style="display: inline-block; margin: 10px;">
                <h4>Login:</h4>
                <input type="hidden" name="action" value="login">
                <input type="text" name="username" placeholder="Enter username" required>
                <button type="submit">🔑 Login</button>
            </form>

            <form method="GET" style="display: inline-block; margin: 10px;">
                <input type="hidden" name="action" value="logout">
                <button type="submit" class="logout">🚪 Logout</button>
            </form>

            <form method="GET" style="display: inline-block; margin: 10px;">
                <input type="hidden" name="action" value="set_counter">
                <button type="submit">➕ Increment Counter</button>
            </form>

            <form method="GET" style="display: inline-block; margin: 10px;">
                <input type="hidden" name="action" value="set_cookie">
                <button type="submit" class="cookie">🍪 Set Demo Cookie</button>
            </form>

            <form method="GET" style="display: inline-block; margin: 10px;">
                <button type="submit">🔄 Refresh</button>
            </form>
        </div>

        <div class="session-info">
            <h3>🔧 Technical Details</h3>
            <p><strong>PHP Session ID:</strong> <?= session_id() ?></p>
            <p><strong>Session Save Path:</strong> <?= session_save_path() ?></p>
            <p><strong>Session Cookie Name:</strong> <?= session_name() ?></p>
            <p><strong>HTTP_COOKIE:</strong> <?= $_SERVER['HTTP_COOKIE'] ?? 'No cookies' ?></p>
            <p><strong>Query String:</strong> <?= $_SERVER['QUERY_STRING'] ?? 'No parameters' ?></p>
        </div>

        <p><em>This demonstrates PHP's built-in session and cookie handling!</em></p>
    </div>
</body>
</html>
