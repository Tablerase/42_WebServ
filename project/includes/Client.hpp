/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: purmerinos <purmerinos@protonmail.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 16:23:15 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/04 16:23:15 by purmerinos       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP

# define CLIENT_HPP

#include <cstddef>
#include <ctime>
#include <sstream>
#include <string>
# define BUFFER_SIZE 32000
# define MAX_URI_SIZE 8192
# define MAX_HEADER_SIZE 16384
#	include <PortListener.hpp>

class Server;

typedef enum e_status {
	IDLE = 0,
	READING = 1,
	WRITING = 2,
}						t_status;

typedef struct s_requestLine {
	string	method;
	string	uri;
	double	protocol;
	string	filePath;
	string	cgiQuery;
}							t_requestLine;

class Client {
	
	public :

		Client(int fd, PortListener& owner, EventLoop& eventLoop);
		~Client( void );

		void		manageNewEvent( void );
		time_t	getLastInteractionTime( void ) const;

		class CloseMeException : public exception {
			public :
				const char* what() const throw() {
					return ("Client manager signaled that connection should be closed");}
		};

	private :

		void	_readRequest( void );
		void	_sendAnswer( void );
		void	_parseRequest( void );
		void	_parseHeader( void );
		void	_parseRequestLine( const string& requestLine );
		void	_parseMethod( const string& method);
		void 	_parseUri( const string& uri);
		void	_parseProtocol( const string& protocol);
		void	_checkContentLength(void);
		void	_checkForChunkedRequest( void );
		void	_parseChunkedRequest( string RequestPart );
		void	_manageDeleteRequest( void );
		void	_manageGetRequest( void );
		void	_managePostRequest( void );
		void	_processClassicGetRequest(string& extension );
		void	_processClassicPostRequest( void );
		bool	_checkExtensionMatch(const string& extension);
		void	_listDirectory( void );
		void	_buildNoBodyResponse(int status, string info, string body, bool isFatal);
		bool	_loadCustomStatusPage(string path);
		void	_fillResponse( string status, bool shouldClose );
		void	_generateContentExtension(string& path);

		// Variables for interaction with outside of the objects.
		Server*				_configServer;
		EventLoop&		_mainEventLoop;
		PortListener&	_owner;
		const	int			_connectionEntry;
		time_t				_lastInteractionTime;

		// Concerning request parsing
		t_status						_status;
		t_requestLine				_requestLine;
		char								_buffer[BUFFER_SIZE];
		size_t							_singleReadBytes;
		size_t							_bytesReadFromBody;
		string							_header;
		string							_body;
		map<string, string>	_headerFields;
		bool								_headerIsFullyRed;
		bool								_bodyIsPresent;
		bool								_bodyIsFullyRed;
		bool								_requestIsChunked;
		int									_contentLength;

		// COncerning response status
		stringstream				_response;
		stringstream				_bodyStream;
		map<string, string>	_responseHeader;
		bool								_responseIsReady;
		bool								_connectionShouldBeClosed;
};

#endif
