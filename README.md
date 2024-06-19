# Documentation

- [Data Structures and Project Architecture - Web Server - Figjam](https://www.figma.com/files/team/1364356230083559376/project/240515569/WebServ?fuid=1364356226035135583)
  - [Local Saved Figjam](./document/figjam/)

# Config file

Block server :
-

 - **server_name** [name of the server]
 - **listen** [port number]
 - **client_max_body_size** [size in Mb]
 - **error_page** [error code] [path to error file]

Inside a server block, we can have zero to many location blocks.

Location block [matching path] :
-

 - **root** [path] -> appends this path after the entry path
 - **index** [file name] -> default page if no file specified in the uri
 - **autoindex** [on|off] -> enables/disables the listing of directory
 - **allowed_methods** [GET|POST|DELETE] -> defines the allowed methods
 - **cgi** [.extension] [path to executable]
 - **upload_file_path** [directory path] -> location of the files to download