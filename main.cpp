#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>

#define LISTEN_BACKLOG 50

// INADDR_ANY
// INADDR_LOOPBACK

struct sockaddr_in in_addr;
sockaddr* peer_ip;
socklen_t* peer_size;

std::vector<int> CLIENTS;

int sendall( const char *MESSAGE, int actual_client )
{
	for ( auto client : CLIENTS) {
		if ( actual_client != client) {
			send(client, MESSAGE, strlen(MESSAGE), 0);
		};
	};
	return 0;
}

// TODO: Make a function to remove a client from the list

void conn_handler( int client_socket )
{
	CLIENTS.push_back(client_socket);

	const char *HELLO_MESSAGE 	= "\nTchat v1.0\nINFO: Type :help to see all the commands\r\n";
	const char *BYE_MESSAGE		= "INFO: User disconnected \r\n";
	const char *HELP_MESSAGE	= "HELP Commands avalible:\n    :exit EXITS\n    :help DISPLAYS THE HELP\r\n";

	bool TERMINATE_RECEIVING = false;

	int hello_message = send(client_socket, HELLO_MESSAGE, strlen(HELLO_MESSAGE), 0);

	if ( hello_message == strlen(HELLO_MESSAGE) ) { 
		std::cout << "INFO: Sent hello message" << std::endl;
	};
	
	while ( TERMINATE_RECEIVING != true ) {
		
		char buff[2048];
		int bytes = recv(client_socket, buff, 2048, 0);
		buff[bytes] = '\0';

		// TODO: HANDLE HERE THE CHANNELS AND SENDERS

		if ( buff[0] == ':' ) {

			if ( strcoll(buff, ":exit") == 13 ) {
				TERMINATE_RECEIVING = true;
				sendall(BYE_MESSAGE, client_socket);
			};

			if ( strcoll(buff, ":help") == 13 ) {
				send(client_socket, HELP_MESSAGE, strlen(HELP_MESSAGE), 0);
			};

		} else {

			sendall(buff, client_socket);

		};

	};
	
	close(client_socket);
}

int main()
{
	auto PERMISIONS = getuid();

	if ( PERMISIONS != 0 ) {
		std::cout << "ERROR: Run as root" << std::endl;
		return 1;
	};

	in_addr.sin_family = AF_INET;
	in_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	in_addr.sin_port = htons(1010);
	
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	if ( tcp_socket == -1 ) {
		std::cout << "ERROR: could not create a socket" << std::endl;
		exit(1);
	} 
	
	int binding = bind(tcp_socket, (sockaddr*)&in_addr, sizeof(in_addr));

	if ( binding == -1 ) {
		std::cout << "ERROR: Could not bind to port" << std::endl;
		exit(1);
	};

	std::cout << "INFO: Listening on port 1010" << std::endl;

	listen(tcp_socket, LISTEN_BACKLOG);

	while ( true )
	{

		int new_socket = accept(tcp_socket, peer_ip, peer_size);

		if ( new_socket ) {
			std::cout << "INFO: Connection accepted" << std::endl;
		};

		std::thread handling_thread(conn_handler, new_socket);
		handling_thread.detach();

	};
};
