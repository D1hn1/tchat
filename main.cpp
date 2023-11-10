#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>

#define LISTEN_BACKLOG 50
#define IP INADDR_LOOPBACK
#define PORT 1010

// INADDR_ANY
// INADDR_LOOPBACK

struct sockaddr_in in_addr;
sockaddr* peer_ip;
socklen_t* peer_size;

struct client {

	int socket;
	int channel;
	char* username;

};

std::vector<client> CLIENTS;

int sendall( const char *MESSAGE, int actual_client, int channel )
{
	for ( auto client : CLIENTS) {
		if ( actual_client != client.socket ) {
			if ( client.channel == channel ) {
				send(client.socket, MESSAGE, strlen(MESSAGE), 0);
			};
		};
	};
	return 0;
}

int remcli( int actual_client ) {
	
	int array_size = CLIENTS.size();

	for ( int x = 0; x < array_size; x++ ) {
		if ( CLIENTS[x].socket == actual_client ) {
			CLIENTS.erase(CLIENTS.begin()+x);
		};
	};
	
	return 0;
}

void conn_handler( int client_socket )
{
	
	client new_client;
	new_client.channel = 1;
	new_client.socket = client_socket;
	new_client.username = (char *) malloc(sizeof(char *)*10);
	
	strcpy(new_client.username, "User");
	CLIENTS.push_back(new_client);

	const char *HELP_MESSAGE	= "\nHELP Commands avalible:\n    :showch SHOW YOUR CURRENT CHANNEL\n    :channel CHANGE YOUR CHANNEL\n    :whoami SEE WHO YOU ARE\n    :sendto SEND A MSG TO A USER\n    :listusers LIST CONNECTED USERS\n    :name CHANGE YOUR NAME\n    :exit EXITS\n    :help DISPLAYS THE HELP\r\n";
	const char *HELLO_MESSAGE 	= "\nTchat v1.1\nINFO: Type :help to see all the commands\r\n";
	const char *CHANN_CHANGE	= "INFO: Changed channel successfully\r\n";
	const char *CHANGE_NAME		= "INFO: Changed name successfully\r\n";
	const char *BAD_COMMAND		= "INFO: That is not a command\r\n";
	const char *BYE_MESSAGE		= "INFO: User disconnected \r\n";
	const char *NOT_IMPLEMENTED	= "INFO: Not implemented\r\n";

	int hello_message = send(new_client.socket, HELLO_MESSAGE, strlen(HELLO_MESSAGE), 0);

	if ( hello_message == strlen(HELLO_MESSAGE) ) { 
		std::cout << "INFO: Sent hello message" << std::endl;
	};

	bool TERMINATE_RECEIVING = false;
	
	while ( TERMINATE_RECEIVING != true ) {
		
		char buff[2048];
		int bytes = recv(new_client.socket, buff, 2048, 0);
		buff[bytes] = '\0';

		std::string query = buff;

		if ( buff[0] == ':' ) {

			if ( query.find("exit") == 1 ) {
				remcli(new_client.socket);
				TERMINATE_RECEIVING = true;
				sendall(BYE_MESSAGE, new_client.socket, new_client.channel);
				std::cout << "INFO: " << new_client.username << " disconnected" << std::endl;
				free(new_client.username);

			} else if ( query.find("help") == 1 ) {
				send(new_client.socket, HELP_MESSAGE, strlen(HELP_MESSAGE), 0);

			} else if ( query.find("name") == 1 ) {
				char *name_comm = strtok(buff, " ");
				char *name_name = strtok(NULL, " ");
				
				if ( name_name ) {
					name_name[strcspn(name_name, "\n")-1] = '\0';
					strcpy(new_client.username, name_name);
					send(new_client.socket, CHANGE_NAME, strlen(CHANGE_NAME), 0);
				};

			} else if ( query.find("showch") == 1 ) {
				std::string str_channel= std::to_string(new_client.channel);
				const char *str_final_channel = str_channel.c_str();
				send(new_client.socket, str_final_channel, strlen(str_final_channel), 0);
				send(new_client.socket, "\n", 2, 0);

			} else if ( query.find("channel") == 1 ) {
				char *channel_comm = strtok(buff, " ");
				char *channel_num = strtok(NULL, " ");
				
				if ( channel_num ) {
					new_client.channel = atoi(channel_num);
					send(new_client.socket, CHANN_CHANGE, strlen(CHANN_CHANGE), 0);
				};

			} else if ( query.find("sendto") == 1 ) {
				char *sendto_comm = strtok(buff, " ");
				char *sendto_user = strtok(NULL, " ");
				char *sendto_message = strtok(NULL, " ");
				std::string final_sendto_message = "";

				while ( sendto_message != NULL ) {
					final_sendto_message.insert(final_sendto_message.size(), sendto_message);
					final_sendto_message.insert(final_sendto_message.size(), " ");
					sendto_message = strtok(NULL, " ");
				};

				std::string SENDTO_MESSAGE_AND_USERNAME = "PRIVATE " + (std::string)new_client.username + ": " + final_sendto_message + "\r";
				send(atoi(sendto_user), SENDTO_MESSAGE_AND_USERNAME.c_str(), strlen(SENDTO_MESSAGE_AND_USERNAME.c_str()), 0);

			} else if ( query.find("whoami") == 1 ) {
				send(new_client.socket, new_client.username, strlen(new_client.username), 0);
				send(new_client.socket, "\n", 2, 0);

			} else if ( query.find("listusers") == 1 ) {

				const char *listusers_banner = "\nID USERNAME CHANNEL\n\n";
				send(new_client.socket, listusers_banner, strlen(listusers_banner), 0);

				for ( auto user : CLIENTS ) {
					
					std::string listusers_socket = std::to_string(user.socket);
					std::string listusers_channel = std::to_string(user.channel);

					send(new_client.socket, listusers_socket.c_str(), strlen(listusers_socket.c_str()), 0);
					send(new_client.socket, "	", strlen("	"), 0);
					send(new_client.socket, user.username, strlen(user.username), 0);
					send(new_client.socket, "	", strlen("	"), 0);
					send(new_client.socket, listusers_channel.c_str(), strlen(listusers_channel.c_str()), 0);

					send(new_client.socket, "\n", 2, 0);

				};

				send(new_client.socket, "\n", 2, 0);

			} else {
				send(new_client.socket, BAD_COMMAND, strlen(BAD_COMMAND), 0);

			};

		} else {
			
			std::string MESSAGE_AND_USERNAME = (std::string)new_client.username + ": " + (std::string)buff + "\r";
			sendall(MESSAGE_AND_USERNAME.c_str(), new_client.socket, new_client.channel);

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

	int HTPORT = htons(PORT);
	int HTIP = htonl(IP);

	in_addr.sin_family = AF_INET;
	in_addr.sin_addr.s_addr = HTIP;
	in_addr.sin_port = HTPORT;
	
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	if ( tcp_socket == -1 ) {
		std::cout << "ERROR: could not create a socket" << std::endl;
		exit(1);
	} 
	
	int binding = bind(tcp_socket, (sockaddr*)&in_addr, sizeof(in_addr));

	if ( binding == -1 ) {
		std::cout << "ERROR: Could not bind to port: " << PORT << std::endl;
		exit(1);
	};

	std::cout << "INFO: Listening on port: " << PORT << std::endl;

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
