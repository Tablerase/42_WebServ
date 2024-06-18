/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 16:37:44 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/18 14:26:42 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"
#include "Server.hpp"
#include "parsing.hpp"

#include "test.hpp"

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
  if (listeners.empty() == true)
    return EXIT_FAILURE;
  for (vector<PortListener>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
    cout << *it;
  }
	int isChild = 0;
	// try {
		EventLoop mainLoop(listeners);
		isChild = mainLoop.loopForEvent();
	// } catch (exception& e) {
	// 	cerr << "Could Not Start Server because of " << e.what() << endl;
	// }
	return (isChild);
}
