
import cgi, cgitb

form = cgi.FieldStorage()

bgcolor = form["BackGroundColor"]
print("Content-type:text/html")

body = "<head><title> BackGround Color Is modified"
body += "</title><style>* {font-family: system-ui, -apple-system, BlinkMacSystemFont,"
body += "'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;}"
body += "body { background-color: "
body += "Azure"
body += "; display: flex; justify-content: center; align-items: center; height: 100vh;"
body += "margin: 0; flex-direction: column;} h1 { font-size: 3em; color: black;"
body += "} </style> </head>"
body += "<body><h1>"
body += "Color Successefully modified"
body += "</h1></body></html>"
body += "<br><a href=\"/python/welcome.py\"><button> Go back to your profile</button></a>"

if bgcolor:
    print(f"Set-Cookie: bgcolor={bgcolor.value}")
print(f"Content-Length: {len(body)}\r\n\r\n")
print(f"{body}")
