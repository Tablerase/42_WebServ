#include "Server.hpp"
#include <ExternLibrary.hpp>

bool found_close_bracket_on_next_line(ifstream & config_file) {
  string line;
  getline(config_file, line);
  if (line.find("{") != 0)
    return false;
  if (line.find_first_not_of(" \t\n\v\f\r", 1) != string::npos)
    return false;
  return true;
}

bool is_close_bracket(string & line) {
  size_t pos = line.find("}");
  if (pos == string::npos)
    return false;
  if (line.find_first_not_of("} \t\n\v\f\r") != string::npos)
    throw runtime_error("bracket not properly closed.");
  if (pos != line.find_last_of("}"))
    throw runtime_error("bracket not properly closed.");
  return true;
}

bool is_valid_limit_except_componant(string & current_method, access_seen & as) {
  if (current_method == "GET") {
    if (as.get_ == true)
      return false;
    as.get_ = true;
    return true;
  }
  if (current_method == "POST") {
    if (as.post_ == true)
      return false;
    as.post_ = true;
    return true;
  }
  if (current_method == "DELETE") {
    if (as.delete_ == true)
      return false;
    as.delete_ = true;
    return true;
  }
  return false;
}

bool is_allowed_cgi_extension(string & extension) {
  return (extension == ".py" || extension == ".rb"); // for ex
}

bool is_valid_http_error_code(int error_code) {
  if (error_code >= 400 && error_code <= 418)
    return true;
  if (error_code >= 421 && error_code <= 426)
    return true;
  if (error_code == 428 || error_code == 429 || error_code == 431 || error_code == 451)
    return true;
  return false;
}

bool is_empty_server(already_seen_server & as) {
  for (map<int,bool>::const_iterator it = as.error_pages_.begin(); it != as.error_pages_.end(); ++it) {
    if (it->second == true)
      return false;
  }
  if (as.max_client_body_size_ == false
    && as.name_ == false && as.port_ == false)
    return true;
  return false;
}