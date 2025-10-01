#!/usr/bin/ruby

puts "Content-Type: text/html; charset=UTF-8"
puts ""

puts "<!DOCTYPE html>"
puts "<html><head><meta charset='UTF-8'><title>Ruby CGI</title></head><body>"
puts "<h1>💎 Ruby CGI Works!</h1>"
puts "<p><strong>Method:</strong> #{ENV['REQUEST_METHOD']}</p>"
puts "<p><strong>Query:</strong> #{ENV['QUERY_STRING']}</p>"
puts "<p><strong>Path Info:</strong> #{ENV['PATH_INFO']}</p>"
puts "<p><strong>Time:</strong> #{Time.now}</p>"
puts "</body></html>"
