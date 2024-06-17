/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:16:35 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/17 14:56:30 by rcutte           ###   ########.fr       */
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

/**
 * @brief Get the max client body size
 * @return int: the max client body size
 * @note The max client body size is in Mb by default it's 1Mb
*/
size_t const & Server::get_max_client_body_size() const {
  return this->max_client_body_size_;
}

std::map<std::string, location> const &Server::get_locations() const {
  return this->locations_;
}

/**
 * @brief Get the location from the path (no search)
 * @param path: the path
 * @return location: the location struct
 * @note If the path is not found, return the default("/") location
*/
location const *Server::get_location(std::string const &path) const {
  try {
    return &(this->locations_.at(path));
  }
  catch (std::out_of_range &oor) {
		string old_cpy = path;
		if (old_cpy.find_last_of("/") == old_cpy.size() - 1 &&
				old_cpy.size() > 1) {
			old_cpy.erase(old_cpy.size() - 1, old_cpy.npos);
		}
		string new_path = old_cpy.substr(0, old_cpy.find_last_of("/") + 1);
		cout << "New Path : " << new_path << endl;
    return get_location(new_path);
  }
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
string const Server::get_error_page(int const & error_code) const {
  string result;
  try {
    return this->error_pages_.at(error_code);
  }
  catch (out_of_range & oor) {
    return result;
  }
}

/**
 * @brief Check if the path is a redirect
 * @param path: the path
 * @return bool: true if the path is a redirect, false otherwise
*/
bool Server::is_redirect(string const & path) const {
  try {
    return this->locations_.at(path).redirect_;
  }
  catch (out_of_range & oor) {
    return false;
  }
}

/**
 * @brief Get the redirect absolute path from the path
 * @param path: the path
 * @return string: the redirect absolute path
 * @note If the path is not found, return an empty string
*/
string const Server::get_redirect_path(string const & path) const {
  try {
    return this->locations_.at(path).redirect_path_;
  }
  catch (out_of_range & oor) {
    return "";
  }
}

/**
 * @brief Get the root absolute path from the path
 * @param path: the path
 * @return string: the root absolute path
 * @note If the path is not found, return an empty string
*/
string const Server::get_root(string const & path) const {
  try {
    return this->locations_.at(path).root_;
  }
  catch (out_of_range & oor) {
    return "";
  }
}

/**
 * @brief Get the index file from the path
 * @param path: the path
 * @return string: the index absolute path of file
 * @note If the path is not found, return an empty string
*/
string const Server::get_index(string const & path) const {
  try {
    string root = this->locations_.at(path).root_;
    string index = this->locations_.at(path).index_;
    string result = root + "/" + index;
    // remove double slashes
    if (result.find("//") != string::npos)
      return result.substr(0, result.find("//")) + result.substr(result.find("//") + 1);
    return result;
  }
  catch (out_of_range & oor) {
    return "";
  }
}

/**
 * @brief Get the autoindex/directory listing value from the path
 * @param path: the path
 * @return bool: the autoindex value (True if the directory listing is allowed, False otherwise)
 * @note If the path is not found, return false
*/
bool Server::is_autoindex(string const & path) const {
  try {
    return this->locations_.at(path).autoindex_;
  }
  catch (out_of_range & oor) {
    return false;
  }
}

/**
 * @brief Check if the method is allowed on the path
 * @param path: the path
 * @param method: the method
 * @return bool: true if the method is allowed, false otherwise
*/
bool Server::is_allowed_method(string const & path, string const & method) const {
  try {
    vector<string> limit_except = this->locations_.at(path).limit_except_;
    for (vector<string>::iterator it = limit_except.begin(); it != limit_except.end(); ++it) {
      if (*it == method)
        return true;
    }
  }
  catch (out_of_range & oor) {
    return false;
  }
  return false;
}

/**
 * @brief Get the upload path from the path
 * @param path: the path
 * @return string: the upload absolute path
 * @note If the path is not found, return an empty string
*/
string const Server::get_upload_path(string const & path) const {
  try {
    return this->locations_.at(path).upload_path_;
  }
  catch (out_of_range & oor) {
    return "";
  }
}

/**
 * @brief Get the limit except config (allowed methods) from the path
 * @param path: the path
 * @return vector<string>: the limit except vector
 * @note If the path is not found, return an empty vector
*/
vector<string> Server::get_limit_except(string const & path) const {
  try {
    return this->locations_.at(path).limit_except_;
  }
  catch (out_of_range & oor) {
    return vector<string>();
  }
}

/**
 * @brief Check if the path and the extension are in a cgi
 * @param path: the path
 * @param extension: the extension (.php, .py, ...)
 * @return bool: true if the path and extension are in a cgi, false otherwise
*/
bool Server::is_cgi_extension(string const & path, string const & extension) const {
  try {
    map<string,string> cgi = this->locations_.at(path).cgi_;
    for (map<string,string>::iterator it = cgi.begin(); it != cgi.end(); ++it) {
      if (it->first == extension)
        return true;
    }
  }
  catch (out_of_range & oor) {
    return false;
  }
  return false;
}

/**
 * @brief Get the cgi path from the path and the extension
 * @param path: the path
 * @param extension: the extension (.php, .py, ...)
 * @return string: the cgi absolute path
 * @note If the path and extension are not found, return an empty string
*/
string const Server::get_cgi_path(string const & path, string const & extension) const {
  try {
    map<string,string> cgi = this->locations_.at(path).cgi_;
    for (map<string,string>::iterator it = cgi.begin(); it != cgi.end(); ++it) {
      if (it->first == extension)
        return it->second;
    }
  }
  catch (out_of_range & oor) {
    return "";
  }
  return "";
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

/**
 * @brief Add an error page to the server
 * @param error_code: the error code
 * @param error_page: the absolute path to the error page
 * @note If the error code is already in the map, throw an exception
*/
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

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& map) {
    os << "{";
    for (typename std::map<K, V>::const_iterator it = map.begin(); it != map.end(); ++it) {
        if (it != map.begin()) os << ", ";
        os << it->first << ": " << it->second;
    }
    os << "}";
    return os;
}
