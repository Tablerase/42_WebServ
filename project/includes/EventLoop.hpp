/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 19:20:18 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/03 19:20:19 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTLOOP_HPP

# define EVENTLOOP_HPP

# include <ExternLibrary.hpp>
# include <map>
# include <vector>

# define MAX_EVENTS 1
# define WAIT_TIMEOUT 1000

using namespace std;

class PortListener;
class EventLoop {
	public :

		EventLoop(vector<PortListener>& PortVector);
		~EventLoop( void );

		void					loopForEvent( void );
		int						getEpollFd( void ) const;
		void					addFdOfInterest(int fd, PortListener* owner);

	private :
		
		EventLoop( void );

		PortListener*	_getOwner(int fd);
		static void		_sigIntCatcher( int signal );

		map<const int, PortListener *>	_fdMap;
		epoll_event											_eventManager;
		int															_epollFd;
		static bool											_serverIsRunning;
};

#endif