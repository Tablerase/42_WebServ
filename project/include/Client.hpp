/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 16:23:15 by purmerinos        #+#    #+#             */
/*   Updated: 2024/06/14 20:10:13 by rcutte           ###   ########.fr       */
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
struct location;

typedef enum e_status {
	IDLE = 0,
	READING = 1,
	WRITING = 2,
}						t_status;

typedef struct s_requestLine {
	string	method;
	// string	uri;
	double	protocol;
	string	filePath;
	string	absolutePath;
	string	cgiQuery;
}							t_requestLine;

class Client {
	
	public :

		Client(int fd, PortListener& owner, EventLoop& eventLoop);
		~Client( void );

		void	manageNewEvent( void );
		bool	isCLientTimeout( void );

		class CloseMeException : public exception {
			public :
				const char* what() const throw() {
					return ("Client manager signaled that connection should be closed");}
		};
		class ChildIsExiting : public exception {
			public :
				const char* what() const throw() {
					return ("Children Wants To CleanExit !");}
		};

	private :

		void				_readRequest( void );
		void				_sendAnswer( void );
		void				_parseRequest( void );
		void				_parseRequestLine( const string& requestLine );
		void				_parseMethod( const string& method);
		void 				_parseUri( const string& uri);
		void				_parseProtocol( const string& protocol);
		void				_checkForReferer( void );
		void				_buildAbsolutePath(const string& locPath);
		void				_manageDeleteRequest( void );
		void				_manageGetRequest( void );
		void				_managePostRequest( void );
		void				_processClassicGetRequest(string& extension );
		void				_processClassicPostRequest( void );
		bool				_checkExtensionMatch(const string& extension);
		void				_listDirectory( void );
		void				_buildNoBodyResponse(string status, string info, string body, bool isFatal);
		void				_noBodyResponseDriver( const int status, const string& optionnalBody, const bool isFatal);
		bool				_loadCustomStatusPage(string path);
		void				_fillResponse( string status, bool shouldClose );
		void				_generateContentExtension(string& path);
		void				_statFile(const char* path);
		inline bool	_indexFileExist( const string path);
		inline bool	_requestIsDir( const string path);
		
		// ClientParseHeader.cpp
		void	_parseHeader( void );
		void	_checkContentLength(void);
		void	_checkForChunkedRequest( void );
		void	_parseChunkedRequest( string RequestPart );
		void	_checkHeaderValidity( pair<string, string> newHeader);

		//ClientCgi.cpp

		void	_cgiInit( void );
		void	_manageBodyForCgi( void );
		void	_manageCgiOutfile( void );
		void	_killCgi( void );
		void	_checkCgiStatus( void );
		void	_readOutfile( void );
		void	_childrenRoutine( void );
		void	_buildEnv( void );

		// Variables for interaction with outside of the objects.
		PortListener&		_owner;
		EventLoop&			_mainEventLoop;
		const	int				_connectionEntry;
		time_t					_lastInteractionTime;
		Server*					_configServer;
		const	location*	_locationBlockForTheRequest;

		// Concerning request parsing
		t_status						_status;
		t_requestLine				_requestLine;
		char								_buffer[BUFFER_SIZE];
		int							_singleReadBytes;
		size_t							_bytesReadFromBody;
		string							_header;
		string							_body;
		map<string, string>	_headerFields;
		bool								_headerIsFullyRed;
		bool								_bodyIsPresent;
		bool								_bodyIsFullyRed;
		bool								_requestIsChunked;
		size_t							_contentLength;

		// Cgi
		string					_cgiInfilePath;
		string					_cgiOutFilePath;
		string					_cgiScriptName;
		string					_cgiScriptPath;
		string					_cgiBinPath;
		pid_t						_cgiScriptPid;
		bool						_cgiIsRunning;
		bool						_requestIsHandledByCgi;
		string					_infileSize;
		vector<string>	_env;
		vector<char*> 	_cEnv;
		vector<string>	_arg;
		vector<char*> 	_cArg;

		// COncerning response status
		stringstream				_response;
		stringstream				_bodyStream;
		map<string, string>	_responseHeader;
		bool								_responseIsReady;
		bool								_connectionShouldBeClosed;
		string	_copyForDebug;
};

#endif
