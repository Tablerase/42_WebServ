/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:16:35 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/11 15:55:52 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ======================== Constructor / Destructor ======================== */

Server::Server()
{
  #ifdef LOG
  std::cout << BLUB << "🗄️ Constructor Server by default" << RESET << "\n";
  #endif
  // Default server values
  name_ = "localhost";
  port_ = 80;
  root_ = "";
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

/* ================================ Setters ================================= */

void Server::set_name(std::string const &name) {
  this->name_ = name;
}

void Server::set_port(int const &port) {
  if (port < 0 || port > 65535)
    throw std::invalid_argument("Port must be between 0 and 65535");
  this->port_ = port;
}

/* =============================== Functions ================================ */



/* =============================== Exceptions =============================== */



/* ================================= Output ================================= */

std::ostream &operator<<(std::ostream &os, const Server &obj){
  os
    << YELB << "🗄️ Server" << RESET << " "
    << BYEL << obj.get_name() << RESET
    << " listening to port: " << BBLU << obj.get_port() << RESET << "\n";
  return os;
}