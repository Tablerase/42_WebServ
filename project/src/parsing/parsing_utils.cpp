#include "Server.hpp"
#include <ExternLibrary.hpp>

pair< string, vector<string> > empty_pair() {
  string fst;
  vector<string> snd;
  return make_pair(fst, snd);
}

string get_word(const string & line, const int start_pos) {
  string word;
  size_t start = line.find_first_not_of(" \t\n\v\f\r", start_pos);
  if (start == string::npos)
    return "";
  size_t end = line.find_first_of(" \t\n\v\f\r", start);
  if (end == string::npos)
    word = line.substr(start);
  else
    word = line.substr(start, end - start);
  return word;
}

int number_keyword(vector<string> & valid_fields, string & word) {
  int i = 0;
  for (vector<string>::iterator it = valid_fields.begin(); it != valid_fields.end(); ++it) {
    if (word == *it)
      return i;
    i++;
  }
  return -1;
}

void check_if_semicolon_valid(string & line, string & field_name) {
  size_t pos = line.find(";");
  if (pos != line.rfind(";")) {
    string error_message = "'" + field_name + "' field not correctly formated.";
    throw runtime_error(error_message);
  }
  if (line.find_first_not_of(" \t\n\v\f\r", pos + 1) != string::npos) {
    string error_message = "'" + field_name + "' field not correctly formated.";
    throw runtime_error(error_message);
  }
}

int string_to_int(const string & argument) {
  char *pEnd;
  long num = strtol(argument.c_str(), &pEnd, 10);
  if (pEnd != &argument.c_str()[argument.length()])
    return -1;
  return num;
}

int string_to_int_error_code(const string & string_error_code) {
  const char *cstr_code = string_error_code.c_str();
  char *pEnd;
  if (string_error_code.length() > 3)
    return -1;
  if (cstr_code[0] != '4')
    return -1;
  if ((string_error_code >= "400" && string_error_code <= "418")
    || (string_error_code >= "421" && string_error_code <= "426")
    || (string_error_code == "428" || string_error_code == "429"
    || string_error_code == "431" || string_error_code == "451"))
    return static_cast<int>(strtol(cstr_code, &pEnd, 10));
  return -1;
}

location bzero_location(string & path) {
  location l;
  l.path_ = path;
  l.redirect_ = false;
  l.redirect_path_ = "";
  l.root_ = "";
  l.index_ = "";
  l.autoindex_ = false;
  l.limit_except_.push_back("GET");
  l.upload_path_ = "";
  return l;
}

string mandatory_fields_missing(already_seen_location asl) {
  if (asl.root_ == false)
    return "root";
  if (asl.index_ == false)
    return "index";
  return "";
}

string get_word(const string & line) {
  string word;
  size_t start = line.find_first_not_of(" \t\n\v\f\r");
  if (start == string::npos)
    return "";
  size_t end = line.find_first_of(" \t\n\v\f\r", start);
  if (end == string::npos)
    word = line.substr(start);
  else
    word = line.substr(start, end - start);
  return word;
}
