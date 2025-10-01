<?php

header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>PHP CGI</title></head>
<body>
<h1>🐘 PHP CGI Works!</h1>
<p><strong>Method:</strong> <?php echo $_SERVER['REQUEST_METHOD'] ?? 'N/A'; ?></p>
<p><strong>Query:</strong> <?php echo $_SERVER['QUERY_STRING'] ?? 'N/A'; ?></p>
<p><strong>Path Info:</strong> <?php echo $_SERVER['PATH_INFO'] ?? 'N/A'; ?></p>

<?php if (($_SERVER['REQUEST_METHOD'] ?? '') === 'POST' && !empty($_POST)): ?>
<h2>POST Data Received:</h2>
<pre><?php print_r($_POST); ?></pre>
<?php endif; ?>

</body>
</html>
