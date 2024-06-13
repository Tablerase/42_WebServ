/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:16:35 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/13 12:11:24 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "parsing.hpp"
#include "ExternLibrary.hpp"

/* ======================== Constructor / Destructor ======================== */

Server::Server()
{
  #ifdef LOG
  std::cout << BLUB << "🗄️ Constructor Server by default" << RESET << "\n";
  #endif
  this->name_ = "_";
  this->port_ = 80;
  this->max_client_body_size_ = 1;
  // Default server values
  // name_ = "localhost";
  // port_ = 80;
  // location test;
  // test.path_ = "/";
  // test.redirect_ = false;
  // test.redirect_path_ = "";
  // test.root_ = "/var/www/html";
  // test.index_ = "index.html";
  // test.autoindex_ = false;
  // test.limit_except_.push_back("GET");
  // test.upload_path_ = "/var/www/html";
  // locations_.insert(std::pair<std::string, location>("/", test));
}

Server::~Server()
{
  #ifdef LOG
  std::cout << BLKB << "🗄️‍ Destructor Server by default" << RESET << "\n";
  #endif
}

/* ================================ Getters ================================= */

std::string const &Server::get_name() const {
  return this->name_;
}

int const &Server::get_port() const {
  return this->port_;
}

std::map<std::string, location> const &Server::get_locations() const {
  return this->locations_;
}

location const &Server::get_location(std::string const &path) const {
  return this->locations_.at(path);
}

/**
 * @brief Get the error pages map
 * @return map<int,string>: the error pages map
*/
map<int,string> const & Server::get_error_pages() const {
  return this->error_pages_;
}

/**
 * @brief Get the error page absolute path from the error code
 * @param error_code
 * @return string: the absolute path to the error page
 * @note If the error code is not found, return an empty string
*/
string const & Server::get_error_page(int const & error_code) const {
  string result;
  try {
    result = this->error_pages_.at(error_code);
    return result;
  }
  catch (out_of_range & oor) {
    result = "";
    return result;
  }
}

int const & Server::get_max_client_body_size() const {
  return this->max_client_body_size_;
}

/* ================================ Setters ================================= */

void Server::set_name(std::string const &name) {
  this->name_ = name;
}

void Server::set_port(int const &port) {
  if (port < 0 || port > 65535)
    throw std::invalid_argument("Port must be between 0 and 65535");
  this->port_ = port;
}

void Server::set_max_client_body_size(int const &size) {
  if (size < 0)
    throw invalid_argument("Max body size should be a positive number.");
  this->max_client_body_size_ = size;
}

/* =============================== Functions ================================ */

void Server::AddError_page(int const & error_code, string const & error_page) {
  if (is_valid_http_error_code(error_code) == false)
    throw std::invalid_argument("Invalid error code.");
  try {
    error_pages_.at(error_code);
    throw runtime_error("multiple error_page with the same error code.");
  }
  catch (out_of_range & oor) {}
  this->error_pages_.insert(make_pair(error_code, error_page));
}

void Server::AddLocation(string const & path, location const & loc) {
  try {
    this->locations_.at(path);
    throw runtime_error("multiple location blocks with the same path.");
  }
  catch (std::out_of_range & oor) {}
  this->locations_.insert(make_pair(path, loc));
}

/* =============================== Exceptions =============================== */



/* ================================= Output ================================= */

ostream &operator<<(ostream & os, const map<int,string> error_pages) {
  os << "Error pages :\n";
  if (error_pages.empty() == true)
    return os;
  for (map<int,string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
    os << REDB << it->first << RESET << " : " << it->second << endl;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const Server &obj){
  os
    << YELB << "🗄️ Server" << RESET << " "
    << BYEL << obj.get_name() << RESET
    << " listening to port: " << BBLU << obj.get_port() << RESET << "\n";
  
  os << endl;

  os << obj.get_error_pages() << endl;
  os << "Max client body size : " << obj.get_max_client_body_size() << "Mb" << endl;

  os << endl;

  for (map<string,location>::const_iterator it = obj.get_locations().begin(); it != obj.get_locations().end(); ++it)
    os << it->second << endl;
  return os;
}
