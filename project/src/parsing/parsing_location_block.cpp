#include "Server.hpp"
#include "parsing.hpp"
#include "ExternLibrary.hpp"

void parse_location_block(Server & virtual_server, ifstream & config_file, string & location_path) {
  string line;
  string missing_method;
  already_seen_location as;
  pair<string, location> new_location_block;
  pair<string, vector<string> > new_field;

  new_location_block.first = location_path;
  new_location_block.second = bzero_location(location_path);
  as = bzero_alreadyseenlocation();
  getline(config_file, line);
  while (config_file.eof() == false && is_close_bracket(line) == false) {
    if (line.find_first_not_of(" \t\n\v\f\r") != string::npos) {
      new_field = parse_line_inside_location(line);
      if (new_field != empty_pair())
        add_field_to_location(new_location_block.second, as, new_field);
    }
    getline(config_file, line);
  }
  if (config_file.eof() == true && is_close_bracket(line) == false)
    throw runtime_error("unclosed bracket.");
  missing_method = mandatory_fields_missing(as);
  if (missing_method != "") {
    string error_message = "'" + missing_method + "' field is missing in location block";
    throw runtime_error(error_message);
  }
  virtual_server.AddLocation(new_location_block.first, new_location_block.second);
}

vector<string> location_fields_vector() {
  vector<string> location_fields(7);
  location_fields[0] = "allowed_methods";
  location_fields[1] = "return";
  location_fields[2] = "index";
  location_fields[3] = "root";
  location_fields[4] = "autoindex";
  location_fields[5] = "upload_file_path";
  location_fields[6] = "cgi";
  return location_fields;
}

string check_for_location_field(string & line, ifstream & config_file) {
  string word;
  size_t start = line.find_first_not_of(" \t\n\v\f\r");
  if (start == string::npos)
    return "";
  size_t end = line.find_first_of(" \t\n\v\f\r", start);
  if (end == string::npos)
    word = line.substr(start);
  else
    word = line.substr(start, end - start);
  if (word == "location") {
    if (end == string::npos)
      throw runtime_error("'location' field not correctly formated.");
    start = line.find_first_not_of(" \t\n\v\f\r", end);
    if (start == string::npos)
      throw runtime_error("'location' field not correctly formated.");
    end = line.find_first_of("{ \t\n\v\f\r", start);
    if (end == string::npos) {
      if (found_close_bracket_on_next_line(config_file) == false)
        throw runtime_error("'location' field not correctly formated.");
      return line.substr(start);
    }
    word = line.substr(start, end - start);
    size_t pos = line.find("{", end);
    if (pos == string::npos || line.find_last_not_of(" \t\n\v\f\r") != pos)
      throw runtime_error("'location' field not correctly formated.");
    return word;
  }
  return "";
}

pair<string,vector<string> > parse_line_inside_location(string & line) {
  vector<string> location_fields = location_fields_vector();
  if (line.find(";") == string::npos)
    throw runtime_error("missing ';' at end of line.");
  string word = get_word(line);
  if (word == "")
    return empty_pair();
  if (word == line) {
    string error_message = "'" + word + "' field not correctly formated.";
    throw runtime_error(error_message);
  }
  int num_key = number_keyword(location_fields, word);
  if (num_key == -1) {
    string error_message = "'" + word + "' is not a valid name field.";
    throw runtime_error(error_message);
  }
  switch (num_key)
  {
  case FIELD_ALLOWED_METHODS:
    return get_method_line(location_fields[FIELD_ALLOWED_METHODS], line, 1); // 1 arg : GET or GET|POST ...
  case FIELD_RETURN:
    return get_method_line(location_fields[FIELD_RETURN], line, 1); // 1 arg : [path]
  case FIELD_INDEX:
    return get_method_line(location_fields[FIELD_INDEX], line, 1); // 1 arg : [file name]
  case FIELD_ROOT:
    return get_method_line(location_fields[FIELD_ROOT], line, 1); // 1 arg : [path]
  case FIELD_AUTOINDEX:
    return get_method_line(location_fields[FIELD_AUTOINDEX], line, 1); // 1 arg [on|off]
  case FIELD_UPLOAD_FILE_PATH:
    return get_method_line(location_fields[FIELD_UPLOAD_FILE_PATH], line, 1); // 1 arg : [path]
  case FIELD_CGI:
    return get_method_line(location_fields[FIELD_CGI], line, 2); // 2 args : [.extension] [path_to_bin]
  default:
    return empty_pair();
  }
}

void add_field_to_location(location & location_block, already_seen_location & as, pair<string, vector<string> > & new_field) {
  vector<string> location_fields = location_fields_vector();
  int num_key = number_keyword(location_fields, new_field.first);
  switch (num_key)
  {
  case FIELD_ALLOWED_METHODS:
    return initLimitExcept(location_block, as, new_field.second.front());
  case FIELD_RETURN:
    return initRedirectionLocation(location_block, as, new_field.second.front());
  case FIELD_INDEX:
    return initIndexLocation(location_block, as, new_field.second.front());
  case FIELD_ROOT:
    return initRootLocation(location_block, as, new_field.second.front());
  case FIELD_AUTOINDEX:
    return initAutoindexLocation(location_block, as, new_field.second.front());
  case FIELD_UPLOAD_FILE_PATH:
    return initUploadPathLocation(location_block, as, new_field.second.front());
  case FIELD_CGI:
    return initCgiLocation(location_block, as, new_field.second);
  default:
    break;
  }
}
