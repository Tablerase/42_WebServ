/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PortListener.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 13:25:40 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/18 14:34:45 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PORTLISTENER_HPP
# define PORTLISTENER_HPP

#include "EventLoop.hpp"
#include <map>
#include <sstream>
#include "Server.hpp"
#include <string>

#define TIMEOUT 3
#define MAX_CONNECTIONS 4000

class Server;
class Client;
class EventLoop;

using namespace std;

class PortListener {
	
	public :
		PortListener( void );
		~PortListener( void );

		int						getSocketFd( void ) const;
		const string&	getListeningPort( void ) const;
		map<string, Server> &	getServerMap( void );
		void					setMainEventLoop(EventLoop *ptr);
		Server*				getServer(const string& name);
		void					getTimeout();

		void	initSocket( void ); // Could be moved in the constructor but it depends
		// how you'll implement it.
		void	manageEvent( int fd);

		// Added for parsing
        void                    addServerToMap(Server & new_server);
		void					printServerMap() const;
		void					setDefaultServer(const string & default_server);
		const string &          getDefaultServer() const;
		// end for parsing

	private :
		void					_acceptConnection( void );
		void					_writeMinimalAnswer( int fd, string status, string info, string body );
		void					_sendMinimalAnswer( int fd, string Answer);
		const string*	_thisNeedToSendAnswer(int fd);
		void					_closeConnection(int fd);

		map<string, Server>	_serverMap;
		string								_defaultServer;
		map<int, Client *>		_clientMap;
		map<int, string>			_immediateResponse;
		int										_socketFd;
		EventLoop*						_mainEventLoop;
		string								_listeningPort;
		struct addrinfo*			_address;
		struct addrinfo*			_fullList;
};

ostream & operator<<(ostream & os, PortListener & listener);

#endif
