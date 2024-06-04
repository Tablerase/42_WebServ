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
#include <string>
#include <unordered_map>

class Server;
class Client;

class PortListener {
	
	public :
		PortListener( void );
		~PortListener( void );

		int						getSocketFd( void ) const;
		const string&	getListeningPort( void ) const;
		void					setMainEventLoop(EventLoop *ptr);

		void	initSocket( void ); // Could be moved in the constructor but it depends
		// how you'll implement it.
		void	manageEvent( int fd);


	private :
		void	_acceptConnection( void );

		unordered_map<string, Server *>	_serverMap;
		map<int, Client *>							_clientMap;
		int															_socketFd;
		EventLoop*											_mainEventLoop;
		string													_listeningPort;
		struct addrinfo*								_address;
};

#endif
