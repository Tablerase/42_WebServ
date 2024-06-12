#include "Server.hpp"
#include <ExternLibrary.hpp>

void parse_virtual_server_field(Server & virtual_server, string & line, already_seen_server & as) {
  pair<string, vector<string> > new_field = parse_server_line(line);
  vector<string> server_fields = server_fields_vector();
  int num_key = number_keyword(server_fields, new_field.first);
  switch (num_key)
  {
  case FIELD_LISTEN:
    return initListenServer(virtual_server, as, new_field.second.front());
  case FIELD_SERVER_NAME:
    return initServerNameServer(virtual_server, as, new_field.second.front());
  case FIELD_ERROR_PAGE:
    return initErrorPageServer(virtual_server, as, new_field.second);
  case FIELD_CLIENT_MAX_BODY_SIZE:
    return initClientMaxBodySize(virtual_server, as, new_field.second.front());
  default:
    break;
  }
}

pair<string, vector<string> > parse_server_line(string & line) {
  vector<string> server_fields = server_fields_vector();

  // ######################### TO DO ######################### //
  //                                                           //
  // check for fields that were not set and set default values //
  //                                                           //
  // ######################### TO DO ######################### //

  if (line.find(";") == string::npos)
    throw runtime_error("missing ';' at end of line.");
  string word = get_word(line);
  if (word == "")
    return empty_pair();
  if (word == line) {
    string error_message = "'" + word + "' field not correctly formated.";
    throw runtime_error(error_message);
  }
  int num_key = number_keyword(server_fields, word);
  if (num_key == -1) {
    string error_message = "'" + word + "' is not a valid name field.";
    throw runtime_error(error_message);
  }
  switch (num_key)
  {
  case FIELD_LISTEN:
    return get_method_line(server_fields[FIELD_LISTEN], line, 1); // 1 arg : [port_number]
  case FIELD_SERVER_NAME:
    return get_method_line(server_fields[FIELD_SERVER_NAME], line, 1); // 1 arg : [path]
  case FIELD_ERROR_PAGE:
    return get_method_line(server_fields[FIELD_ERROR_PAGE], line, 2); // 2 args : [error_code] [path]
  case FIELD_CLIENT_MAX_BODY_SIZE:
    return get_method_line(server_fields[FIELD_CLIENT_MAX_BODY_SIZE], line, 1); // 1 arg : [xx in Mb]
default:
    return empty_pair();
  }
}

bool is_virtual_server_correctly_set(ifstream & config_file) {
string line;
size_t pos = 0;
while (1) {
    getline(config_file, line);
    if (line.find_first_not_of("server{ \t\n\v\f\r") != string::npos)
    return false;
    if (line.find_first_not_of(" \t\n\v\f\r") != string::npos) {
    if (line.find("server") != 0 || line.find_last_of("server") != 5)
        return false;
    pos = line.find("{");
    if (pos == string::npos && line.find_first_not_of(" \t\n\v\f\r", 6) == string::npos)
        return found_close_bracket_on_next_line(config_file);
    if (line.find_first_not_of(" \t\n\v\f\r", pos + 1) != string::npos)
        return false;
    return true;
    }
}
return true;
}

vector<string> server_fields_vector() {
  vector<string> server_fields(4);
  server_fields[0] = "listen";
  server_fields[1] = "server_name";
  server_fields[2] = "error_page";
  server_fields[3] = "client_max_body_size";
  return server_fields;
}
