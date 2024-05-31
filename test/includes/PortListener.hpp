/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PortListener.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/30 17:44:40 by purmerinos        #+#    #+#             */
/*   Updated: 2024/05/30 17:44:41 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP


# define SERVER_HPP

#include <string>
#include <sys/socket.h>
#include <vector>

using namespace std;
class PortListener {
	public :

		PortListener(const char* port, const string& index);
		~PortListener(void);

		int		getPortFd(void) const;
		int		getSocketFd( void );
		void	readRequest(int socketFd );
		void	writeRequest(int socketFd );
		int		matchFd( int fd ) const;
		int		hasPendingRequest( void ) const;

	private :
		
		int								_portFd;
		struct addrinfo*	_res;
		struct addrinfo*	_address;
		vector<int>				_clientFds;
		string						_index;
		string						_request;
		string						_response;
};

#endif
