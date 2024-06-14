#include "Server.hpp"
#include <ExternLibrary.hpp>

already_seen_location bzero_alreadyseenlocation() {
  already_seen_location l;
  l.redirect_ = false;
  l.root_ = false;
  l.index_ = false;
  l.autoindex_ = false;
  l.limit_except_ = false;
  l.upload_path_ = false;
  l.cgi_.insert(make_pair(".py", false));
  l.cgi_.insert(make_pair(".rb", false));
  return l;
}

already_seen_server bzero_alreadyseenserver() {
  already_seen_server l;
  l.name_ = false;
  l.port_ = false;
  l.max_client_body_size_ = false;
  (void)l.error_pages_; // init in default constructor
  return l;
}
