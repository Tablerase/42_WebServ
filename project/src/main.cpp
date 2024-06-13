/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 16:37:44 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/11 18:31:00 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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

int main(int argc, char** argv, char **envp){
  if (CheckInput(argc, argv) == false ) { return EXIT_FAILURE; };

  // Parsing
  // vector<PortListener> listeners = ParseConfig(argv[1]);

  // Event loop

  Server server;
  std::cout
    << server 
    << server.get_location("/")
    << std::endl;
  (void)envp;
}