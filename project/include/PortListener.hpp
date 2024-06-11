/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PortListener.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 13:25:40 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/04 13:25:41 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PORTLISTENER_HPP

# define PORTLISTENER_HPP
#include <EventLoop.hpp>
#include <map>
#include <sstream>
#include <string>

#define TIMEOUT 5

class Server;
class Client; 

class PortListener {
	
	public :
		PortListener( void );
		~PortListener( void );

		int						getSocketFd( void ) const;
		const string&	getListeningPort( void ) const;
		void					setMainEventLoop(EventLoop *ptr);
		Server*				getServer(const string& name) const;
		void					getTimeout();

		void	initSocket( void ); // Could be moved in the constructor but it depends
		// how you'll implement it.
		void	manageEvent( int fd);

	private :
		void					_acceptConnection( void );
		void					_writeMinimalAnswer( int fd, string status, string info, string body );
		void					_sendMinimalAnswer( int fd, string Answer);
		const string*	_thisNeedToSendAnswer(int fd);
		void					_closeConnection(int fd);

		map<string, Server *>	_serverMap;
		string								_defaultServer;
		map<int, Client *>		_clientMap;
		map<int, string>			_immediateResponse;
		int										_socketFd;
		EventLoop*						_mainEventLoop;
		string								_listeningPort;
		struct addrinfo*			_address;
};

#endif
