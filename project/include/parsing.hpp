/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 11:24:09 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/12 11:37:07 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_H_
# define PARSING_H_

# include "Server.hpp"

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
bool is_empty_server(already_seen_server & as);

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