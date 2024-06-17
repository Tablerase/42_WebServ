
print("Content-type:text/html")

body = "<head><title> Bye Bye !"
body += "</title><style>* {font-family: system-ui, -apple-system, BlinkMacSystemFont,"
body += "'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;}"
body += "body { background-color: "
body += "CornflowerBlue"
body += "; display: flex; justify-content: center; align-items: center; height: 100vh;"
body += "margin: 0; flex-direction: column;} h1 { font-size: 3em; color: black;"
body += "} </style> </head>"
body += "<body><h1>"
body += "See Ya !"
body += "</h1></body></html>"
body += "<br><a href=\"/python/welcome.py\"><button> Go back to main page</button></a>"

print(f"Set-Cookie: user=blbkvvv;Expires=Thu, 01-Jan-1970 00:00:10 GMT;")
print(f"Set-Cookie: bgcolor=spspspsp;Expires=Thu, 01-Jan-1970 00:00:10 GMT;")
print(f"Content-Length: {len(body)}\r\n\r\n")
print(f"{body}")
