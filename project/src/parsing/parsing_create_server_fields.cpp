#include "Server.hpp"
#include <ExternLibrary.hpp>

void initListenServer(Server & server, already_seen_server & as, const string & argument) {
  if (as.port_ == true)
    throw runtime_error("multiple definition of listen in server block.");
  as.port_ = true;
  server.set_port(string_to_int(argument));
  return ;
}

void initServerNameServer(Server & server, already_seen_server & as, const string & argument) {
  if (as.name_ == true)
    throw runtime_error("multiple definition of server_name in server block.");
  as.name_ = true;
  server.set_name(argument);
  return ;
}

void initErrorPageServer(Server & server, already_seen_server & as, const vector<string> & arguments) {
  if (arguments.size() != 2)
    throw runtime_error("'error_page' field wrong number of arguments.");
  int error_code = string_to_int_error_code(arguments.front());
  if (error_code == -1)
    throw runtime_error("'error_page' field invalid error_code.");
  try {
    as.error_pages_.at(error_code);
    throw runtime_error("multiple definition of the same error_page in server block.");
  }
  catch (out_of_range & oer) {}
  as.error_pages_.insert(make_pair(error_code ,arguments.back()));
  server.AddError_page(error_code, arguments.back());
  return ;
}

void initClientMaxBodySize(Server & server, already_seen_server & as, string & argument) {
  if (as.max_client_body_size_ == true)
    throw runtime_error("multiple definition of client max body size in server block.");
  as.max_client_body_size_ = true;
  int size = string_to_int(argument);
  server.set_max_client_body_size(size);
  return ;
}
