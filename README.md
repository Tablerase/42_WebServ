# Web Server

## Description

This project is a HTTP server (Mini NGINX) that can handle multiple clients at the same time. It is able to serve static files and execute CGI scripts. It handles GET, POST and DELETE requests.

## Installation

clone the project, move into it with a terminal, then use:

```bash
make
```

## Usage

### Run

```bash
./webserv [config_file]
```

Example:
```bash
./webserv config/basic.conf
```

### Stop

```bash
Ctrl + C
```

### Use

#### Web Browser
- Open a web browser and go to http://localhost:1313/ to see the default page.
  - Config port is `1313` in the example config file. Modify the port number according to the config file.

#### `curl`
- You can also use `curl` to send requests to the server.
  - `-H` flag is used to add headers to the request.
  - `-X` flag is used to specify the request method.

```bash
curl -X GET http://localhost:1313/
```

## Documentation

- [Data Structures and Project Architecture - Web Server - Figjam](https://www.figma.com/files/team/1364356230083559376/project/240515569/WebServ?fuid=1364356226035135583)
  - [Local Saved Figjam](./document/figjam/)

## Config file

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

## Demonstration with example config file

1. Run the server with the example config file.
    ```bash
    make
    ./webserv config/basic.conf
    ```
2. Open a web browser and go to http://localhost:1313/ to see the default page (with directory listing).
3. Check cgi script by going to http://localhost:1313/ruby/
4. Check the error page by going to http://localhost:1313/some_invalid_path
5. Send a file using `curl`.
    ```bash
    touch test.txt
    curl -X POST -F "file=@./test.txt" http://localhost:1313/
    ```
6. Check the uploaded file by going to http://localhost:1313/upload/
7. Delete the uploaded file using `curl`.
    ```bash
    curl -X DELETE http://localhost:1313/upload/test.txt
    ```
8. Check the deleted file by going to http://localhost:1313/upload/
9. Check cookie by going to http://localhost:1313/python/
