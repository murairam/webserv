#!/usr/bin/perl

print "Content-Type: text/html; charset=UTF-8\n\n";

print <<EOF;
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Perl CGI Test</title>
</head>
<body>
<h1>🐪 Perl CGI Works!</h1>
<p><strong>Method:</strong> $ENV{'REQUEST_METHOD'}</p>
<p><strong>Query String:</strong> $ENV{'QUERY_STRING'}</p>
<p><strong>Script Name:</strong> $ENV{'SCRIPT_NAME'}</p>
<p><strong>Server Software:</strong> $ENV{'SERVER_SOFTWARE'}</p>

<h2>Environment Variables:</h2>
<ul>
EOF

# Print some key environment variables
foreach my $key (sort keys %ENV) {
    if ($key =~ /^(HTTP_|SERVER_|REQUEST_|SCRIPT_|QUERY_|GATEWAY_)/) {
        print "<li><strong>$key:</strong> $ENV{$key}</li>\n";
    }
}

print <<EOF;
</ul>

<h2>Query Parameters:</h2>
EOF

if ($ENV{'QUERY_STRING'}) {
    my @pairs = split(/&/, $ENV{'QUERY_STRING'});
    print "<ul>\n";
    foreach my $pair (@pairs) {
        my ($name, $value) = split(/=/, $pair);
        $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        print "<li><strong>$name:</strong> $value</li>\n";
    }
    print "</ul>\n";
} else {
    print "<p>No query parameters</p>\n";
}

print <<EOF;

<p><em>Perl version: $]</em></p>
</body>
</html>
EOF
