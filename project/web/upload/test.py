#!/usr/bin/python3
import os

print ("Content-type: text/html");
response_body = "<!DOCTYPE html><body><p>Env var</p><ul>";
for name, value in os.environ.items():
    response_body += "<li>";
    response_body += ("{0}: {1}".format(name, value));
    response_body += "</li>"
response_body += "</ul></body>"
contentLength = len(response_body);
print(f"Content-length: {contentLength}\r\n\r\n");
print (f"{response_body}");
