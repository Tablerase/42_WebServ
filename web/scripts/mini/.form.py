
#!usr/bin/python3

import cgi, cgitb

form = cgi.FieldStorage()

print("Content-type:text/html")

content = ("<html>")
content += ("<head>")
content += ("<title> MY FIRST CGI FILE </title>")
content += ("</head>")
content += ("<body>")
content += ("<h3> This is HTML's Body Section </h3>")
content += ("UserName is : ")
if "username" in form:
    username = form["username"].value
    content += (username)
else:
    content += ("Unknown")
content += ("\nEmail is : ")
if "emailaddress" in form:
    emailaddress = form["emailaddress"].value
    content += (emailaddress)
else:
    content += ("Unknown")
content += ("</body>")
content += ("</html>")

print(f"Content-Length: {len(content)}\r\n\r\n")
print(f"{content}")
