import cgi, cgitb, os
from http.cookies import SimpleCookie

cookieString = os.environ.get('HTTP_COOKIE')
serverName = os.environ.get('SERVER_NAME')
body = "<!DOCTYPE html>"


if cookieString:
    cookie = SimpleCookie()
    cookie.load(cookieString)
    for key, morse1 in cookie.items():
        print(f"{key} is {morse1}")
    if 'bgcolor' in cookie and 'user' in cookie:
        name = cookie['user'].value
        bgcolor = cookie['bgcolor'].value
        body += "<head><title>"
        body += name
        body += "</title><style>* {font-family: system-ui, -apple-system, BlinkMacSystemFont,"
        body += "'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;}"
        body += "body { background-color: "
        body += bgcolor
        body += "; display: flex; justify-content: center; align-items: center; height: 100vh;"
        body += "margin: 0; flex-direction: column;} h1 { font-size: 3em; color: white;"
        body += "} </style> </head>"
        body += "<body><h1>"
        body += name
        body += "</h1><h2>Welcome to your panel !</h2></body></html>"
        body += "<br><a href=\"/python/ColorModifier.html\"><button> Change Your Profile Color</button></a>"
        body += "<br><a href=\"/python/LogOut.py\"><button> Log Out </button></a>"
else:
    body += "<head><title> Unknown User"
    body += "</title><style>* {font-family: system-ui, -apple-system, BlinkMacSystemFont,"
    body += "'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;}"
    body += "body { background-color: "
    body += "LightGoldenRodYellow"
    body += "; display: flex; justify-content: center; align-items: center; height: 100vh;"
    body += "margin: 0; flex-direction: column;} h1 { font-size: 3em; color: black;"
    body += "} </style> </head>"
    body += "<body><h1>"
    body += "Oops ! You're not connected yet"
    body += "</h1></body></html>"
    body += "<br><a href=\"/python/connectionForm.html\"><button> Create a new profile</button></a>"

print("Content-Type: Text/html")
print(f"Server: {serverName}")
print(f"Content-Length: {len(body)}\r\n")
print(f"{body}")
