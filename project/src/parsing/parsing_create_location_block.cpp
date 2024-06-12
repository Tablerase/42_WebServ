#include "Server.hpp"
#include <ExternLibrary.hpp>

void initLimitExcept(location & location_block, already_seen_location & as, string & argument) {
  if (as.limit_except_ == true)
    throw runtime_error("multiple definition of allowed methods in location block.");
  as.limit_except_ = true;
  location_block.limit_except_ = parse_limit_except(argument);
  return ;
}

vector<string> parse_limit_except(string & raw_args) {
  access_seen as = {false, false, false};
  string method;
  vector<string> formated_args;
  size_t start = 0;
  size_t end = raw_args.find("|");
  while (end != string::npos) {
    method = raw_args.substr(start, end - start);
    if (is_valid_limit_except_componant(method, as) == false)
      throw runtime_error("'allowed_methods' invalid argument.");
    formated_args.push_back(method);
    start = end + 1;
    end = raw_args.find("|", start);
  }
  method = raw_args.substr(start);
  if (is_valid_limit_except_componant(method, as) == false)
    throw runtime_error("'allowed_methods' invalid argument.");
  formated_args.push_back(method);
  return formated_args;
}

void initRedirectionLocation(location & location_block, already_seen_location & as, string & argument) {
  if (as.redirect_ == true)
    throw runtime_error("multiple definition of redirection in location block.");
  as.redirect_ = true;
  location_block.redirect_ = true;
  location_block.redirect_path_ = argument;
  return ;
}

void initRootLocation(location & location_block, already_seen_location & as, string & argument) {
  if (as.root_ == true)
    throw runtime_error("multiple definition of root in location block.");
  as.root_ = true;
  location_block.root_ = argument;
  return ;
}

void initIndexLocation(location & location_block, already_seen_location & as, string & argument) {
  if (as.index_ == true)
    throw runtime_error("multiple definition of index in location block.");
  as.index_ = true;
  location_block.index_ = argument;
  return ;
}

void initAutoindexLocation(location & location_block, already_seen_location & as, string & argument) {
  if (as.autoindex_ == true)
    throw runtime_error("multiple definition of autoindex in location block.");
  as.autoindex_ = true;
  if (argument == "off")
    location_block.autoindex_ = 0;
  else if (argument == "on")
    location_block.autoindex_ = 1;
  else
    throw runtime_error("'autoindex' field invalid argument [on|off]");
  return ;
}

void initUploadPathLocation(location & location_block, already_seen_location & as, string & argument) {
  if (as.upload_path_ == true)
    throw runtime_error("multiple definition of upload path in location block.");
  as.upload_path_ = true;
  location_block.upload_path_ = argument;
}

void initCgiLocation(location & location_block, already_seen_location & as, vector<string> & arguments) {
  if (arguments.size() != 2)
    throw runtime_error("'cgi' field wrong number of arguments.");
  cout << "cgi not handeled yet." << endl;
  (void)location_block;
  (void)as;
  (void)arguments;
  return ;
}
