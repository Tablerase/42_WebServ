/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:35:08 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/14 19:18:38 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test.hpp"

void TestPath(Server &server, std::string path){
  SeparatorMsg("Testing path: " + path);
  cout
    << "Server root: " << WHTB << server.get_root(path) << RESET << endl
    << "Server index: " << WHTB << server.get_index(path) << RESET << endl
    << "Server autoindex: " << WHTB << server.is_autoindex(path) << RESET << endl
    << "Server upload path: " << WHTB << server.get_upload_path(path) << RESET << endl
    << "Server limit_except: " << WHTB << server.get_limit_except(path) << RESET << endl
    << "Server allow GET: " << WHTB << server.is_allowed_method(path, "GET") << RESET << endl
    << "Server extension cgi: " << WHTB << server.is_cgi_extension(path,".php") << RESET << endl
    << "Server ext bin cgi: " << WHTB << server.get_cgi_path(path,".php") << RESET << endl
    << endl;
}

void TestFunctions(Server &server){
  cout
    << boolalpha
    << "Server name: " << WHTB << server.get_name() << RESET << endl
    << "Server port: " << WHTB << server.get_port() << RESET << endl
    << "Server max_client_body_size: " << WHTB << server.get_max_client_body_size() << RESET << endl
    << "Server error page found (404): " << WHTB << server.get_error_page(404) << RESET << endl
    << "Server error page not found (500): " << WHTB << server.get_error_page(500) << RESET << endl
    ;
  SeparatorLine();
  TestPath(server, "/");
}

void TestServer(){
  { // Basic server
    Server server;
    server.AddError_page(404, "./web/errors/404.html");
    location loc;
    loc.path_ = "/";
    loc.limit_except_.push_back("GET");
    loc.limit_except_.push_back("POST");
    loc.redirect_ = false;
    loc.root_ = "./web/pages/";
    loc.index_ = "index.html";
    loc.autoindex_ = false;
    loc.upload_path_ = "./web/upload/";
    loc.cgi_[".php"] = "/usr/bin/php-cgi";
    server.AddLocation(loc.path_, loc);

    try
    {
      TestFunctions(server);
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
  }
}
