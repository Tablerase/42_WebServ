
#!usr/bin/python3

import cgi, cgitb

form = cgi.FieldStorage()

username = form["username"].value
emailaddress = form["emailaddress"].value

print("Content-type:text/html")

content = ("<html>")
content += ("<head>")
content += ("<title> MY FIRST CGI FILE </title>")
content += ("</head>")
content += ("<body>")
content += ("<h3> This is HTML's Body Section </h3>")
content += ("UserName is : ")
content += (username)
content += ("\nEmail is : ")
content += (emailaddress)
content += ("</body>")
content += ("</html>")

print(f"Content-Length: {len(content)}\r\n\r\n")
print(f"{content}")
