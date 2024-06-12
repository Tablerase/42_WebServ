/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 16:37:44 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/12 11:45:11 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "parsing.hpp"

bool  CheckInput(int argc, char** argv){
  try {
    if (argc != 2)
      throw std::invalid_argument("Usage: ./webserv [config_file]");
    if (std::string(argv[1]).rfind(".conf") != std::string(argv[1]).size() - 5)
      throw std::invalid_argument("Config file must be a .conf file");
    return true;
  } catch (std::exception &e) {
    std::cerr
      << REDB << " ERROR " << RESET << " "
      << RED << e.what() << RESET << std::endl;
    return false;
  }
}

int main(int argc, char** argv){
  if (CheckInput(argc, argv) == false ) { return EXIT_FAILURE; };

  // Parsing
  vector<PortListener> listeners = ParseConfig(argv[1]);
  for (vector<PortListener>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
    cout << *it;
  }

  // Event loop

  // Server server;
  // std::cout
  //   << server
  //   << server.get_location("/")
  //   << std::endl;
}

/*
[ TO CHECK ] set default values
test everything
check for missing fields that are mandatory (location is ?)
empty server valid ?
[ OK ] patch unclosed brackets
[ OK ] patch empty line
[ OK ] cgi field
[ OK ] not allow same port on same server_name
*/