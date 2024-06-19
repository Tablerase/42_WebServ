#include "Server.hpp"
#include "parsing.hpp"
#include "ExternLibrary.hpp"

pair<string, vector<string> > get_method_line(string & field_name, string & line, size_t nb_args) {
  pair<string, vector<string> > myField;
  myField.first = field_name;
  check_if_semicolon_valid(line, field_name);
  size_t start = line.find(field_name);
  start += field_name.length();
  size_t end = start;
  while (end != string::npos) {
    start = line.find_first_not_of(" \t\n\v\f\r;", end);
    if (start == string::npos)
      break ;
    end = line.find_first_of(" \t\n\v\f\r;", start + 1);
    myField.second.push_back(line.substr(start, end - start));
  }
  if (myField.second.empty() == true) {
      string error_message = "'" + field_name + "' missing argument.";
      throw runtime_error(error_message);
  }
  if (myField.second.size() != nb_args){
      string error_message = "'" + field_name + "' wrong number of arguments.";
      throw runtime_error(error_message);
  }
  return myField;
}

location default_location() {
  location loc;
  loc.autoindex_ = false;
  (void)loc.cgi_;
  loc.index_ = "";
  loc.limit_except_.push_back("GET");
  loc.path_ = "/";
  loc.redirect_ = false;
  loc.redirect_path_ = "";
  loc.root_ = "";
  loc.upload_path_ = "";
  return loc;
}

Server parse_virtual_server(ifstream & config_file) {
  string line = "";
  string location_path;
  Server virtual_server;
  already_seen_server as;
  as = bzero_alreadyseenserver();

  while (config_file.eof() == false && is_close_bracket(line) == false) {
    location_path = check_for_location_field(line, config_file);
    if (location_path != "")
      parse_location_block(virtual_server, config_file, location_path);
    else if (line.find_first_not_of(" \t\n\v\f\r") != string::npos)
      parse_virtual_server_field(virtual_server, line, as);
    getline(config_file, line);
  }
  if (config_file.eof() == true && is_close_bracket(line) == false)
    throw runtime_error("unclosed bracket.");
  if (is_empty_server(as) == true && virtual_server.get_locations().empty() == true)
    throw runtime_error("empty server.");
  if (virtual_server.get_locations().empty() == true)
    virtual_server.AddLocation("/", default_location());
  return virtual_server;
}

void add_virtual_server_to_listeners(vector<PortListener> & listeners, Server & server) {
    for (vector<PortListener>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
        if (string_to_int(it->getListeningPort()) == server.get_port()) {
            it->addServerToMap(server);
            return ;
        }
    }
    listeners.push_back(PortListener());
    listeners.back().addServerToMap(server);
    listeners.back().setDefaultServer(server.get_name());
    return ;
}

vector<PortListener> read_config_file(ifstream & config_file) {
  vector<PortListener> listeners;

  while (config_file.eof() == false) {
    if (is_virtual_server_correctly_set(config_file) == false)
      throw runtime_error("'server' field not correctly formated.");
    if (config_file.eof() == true)
      break ;
    Server virtual_server = parse_virtual_server(config_file);
    add_virtual_server_to_listeners(listeners, virtual_server);
  }
  if (listeners.empty() == true)
    throw runtime_error("empty configuration file.");
  return listeners;
}

vector<PortListener> ParseConfig(string file_name) {
  try {
    ifstream config_file(file_name.c_str());
    if (config_file.is_open() == false)
      throw std::runtime_error("Failed to open the configuration file.");
    return read_config_file(config_file);
  }
  catch (exception &e) {
    std::cerr
      << REDB << " ERROR " << RESET << " "
      << RED << e.what() << RESET << std::endl;
  }
  vector<PortListener> empty_vect;
  return empty_vect;
}
