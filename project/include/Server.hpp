/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:12:31 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/11 18:00:07 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
# define SERVER_H_

# define FIELD_ALLOWED_METHODS 0
# define FIELD_RETURN 1
# define FIELD_INDEX 2
# define FIELD_ROOT 3
# define FIELD_AUTOINDEX 4
# define FIELD_UPLOAD_FILE_PATH 5
# define FIELD_CGI 6

# define FIELD_LISTEN 0
# define FIELD_SERVER_NAME 1
# define FIELD_ERROR_PAGE 2
# define FIELD_CLIENT_MAX_BODY_SIZE 3

# include <iostream>
# include <fstream>
# include <iomanip>
# include <stdexcept>
# include <algorithm>
# include <string>
# include <vector>
# include <list>
# include <map>

# include "color.h"
# include <PortListener.hpp>

using namespace std;

struct already_seen_location {
  bool redirect_;
  bool root_;
  bool index_;
  bool autoindex_;
  bool limit_except_;
  bool upload_path_;
  bool cgi_;
};

struct access_seen {
  bool get_;
  bool post_;
  bool delete_;
};

struct already_seen_server {
  bool        name_;
  bool        port_;
  bool        max_client_body_size_;
  map<int,string> error_pages_;
};

struct location {
  // Path to the location
  std::string path_;

  // Redirections
    // redirection false by default
  bool        redirect_;
  std::string redirect_path_;

  // Path infos

  std::string                         root_;
  std::string                         index_;
    // autoindex - directory listing false by default
  bool                                autoindex_;
    // vector(method) - GET by default (Limited to: GET, POST, DELETE)
  std::vector<std::string>            limit_except_;
  std::string                         upload_path_;
};

class Server
{
private:
  /* Core values */

  std::string name_;
  int         port_;

  /* Additional values */

  int                                 max_client_body_size_;
    // map(error_code, error_page_absolute_path)
  std::map<int, std::string>          error_pages_;
    // map(extension, absolue_path_to_bin)
  std::map<std::string, std::string>  cgi_;

  /* Locations values */
    // map(path, location struct)
  std::map<std::string, location> locations_;

public:
  Server();
  ~Server();

  // Getters

  std::string const &get_name() const;
  int const         &get_port() const;
  std::map<std::string, location> const &get_locations() const;
  location const    &get_location(std::string const &path) const;

  // Setters

  void set_name(std::string const &name);
  void set_port(int const &port);
  void set_max_client_body_size(int const &port);

  // Functions

  void AddLocation(std::string const &path, location const &loc);
  void AddError_page(int const &error_code, std::string const &error_page);
  void AddCgi(std::string const &extension, std::string const &path_to_bin);
};

std::ostream &operator<<(std::ostream &os, const Server &obj);

std::ostream &operator<<(std::ostream &os, const location &obj);

// ===================== PARSING.CPP =====================

pair<string, vector<string> > get_method_line(
    string & field_name, string & line, size_t nb_args);
Server parse_virtual_server(ifstream & config_file);
vector<PortListener> read_config_file(ifstream & config_file);
vector<PortListener> ParseConfig(string file_name);

// ===================== PARSING_SERVER_FIELDS.CPP =====================

void parse_virtual_server_field(
    Server & virtual_server, string & line, already_seen_server & as);
pair<string, vector<string> > parse_server_line(string & line);
bool is_virtual_server_correctly_set(ifstream & config_file);
vector<string> server_fields_vector();

// ===================== PARSING_CREATE_SERVER_FIELDS.CPP =====================

void initListenServer(
    Server & server, already_seen_server & as, const string & argument);
void initServerNameServer(
    Server & server, already_seen_server & as, const string & argument);
void initErrorPageServer(
    Server & server, already_seen_server & as, const vector<string> & arguments);
void initClientMaxBodySize(
    Server & server, already_seen_server & as, string & argument);

// ===================== PARSING_LOCATION_BLOCK.CPP =====================

void parse_location_block(
    Server & virtual_server, ifstream & config_file, string & location_path);
vector<string> location_fields_vector();
string check_for_location_field(string & line, ifstream & config_file);
pair<string,vector<string> > parse_line_inside_location(string & line);
void add_field_to_location(
    location & location_block, already_seen_location & as,
    pair<string, vector<string> > & new_field);

// ===================== PARSING_CREATE_LOCATION_BLOCK.CPP =====================

void initLimitExcept(
    location & location_block, already_seen_location & as, string & argument);
vector<string> parse_limit_except(string & raw_args);
void initRedirectionLocation(
    location & location_block, already_seen_location & as, string & argument);
void initRootLocation(
    location & location_block, already_seen_location & as, string & argument);
void initIndexLocation(
    location & location_block, already_seen_location & as, string & argument);
void initAutoindexLocation(
    location & location_block, already_seen_location & as, string & argument);
void initUploadPathLocation(
    location & location_block, already_seen_location & as, string & argument);
void initCgiLocation(
    location & location_block, already_seen_location & as, vector<string> & arguments);

// ===================== PARSING_BOOLEAN_UTILS.CPP =====================

bool found_close_bracket_on_next_line(ifstream & config_file);
bool is_close_bracket(string & line);
bool is_valid_limit_except_componant(string & current_method, access_seen & as);
bool is_allowed_cgi_extension(string & extension);
bool is_valid_http_error_code(int error_code);

// ===================== PARSING_ALREADYSEEN.CPP =====================

already_seen_location bzero_alreadyseenlocation();
already_seen_server   bzero_alreadyseenserver();

// ===================== PARSING_UTILS.CPP =====================

pair< string, vector<string> > empty_pair();
string get_word(const string & line, const int start_pos);
int number_keyword(vector<string> & valid_fields, string & word);
void check_if_semicolon_valid(string & line, string & field_name);
int string_to_int(const string & argument);
int string_to_int_error_code(const string & string_error_code);
location bzero_location(string & path);
string mandatory_fields_missing(already_seen_location asl);
string get_word(const string & line);

#endif