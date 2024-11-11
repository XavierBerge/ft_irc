/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:04 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 20:44:56 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <map>
#include <vector>
#include <poll.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <sstream>
#include <ctime>
#include <algorithm> 

#define MAX_CLIENTS 10
#define RED "\033[1;31m"
#define RESET "\033[0m"

class Client;

class Channel;

class Server 
{
	private:
		int server_fd;  // Socket du serveur
		std::map<int, Client*> clients;  // Mapping des descripteurs de fichiers à chaque client
		std::vector<struct pollfd> poll_fds;  // Liste des sockets surveillés par poll()
		std::map<std::string, Channel*> channels;
		std::string password;
		std::string hostname;
		std::string servername;
		static bool Signal;
		typedef void (Server::*CommandHandler)(int, const std::string&);
		std::map<std::string, CommandHandler> commandMap;

		void add_poll_fd(int fd, short events); 
		void handle_client_data(int client_fd); // parsing data from client
		// clean exit methods
		void remove_poll_fd(int fd);
		void close_connection(int client_fd);
		// authentification methods
		void handle_authentication(int client_fd, const std::string& command);
		void handle_new_connection();
		void send_welcome_messages(Client  *client);
		
		void initializeCommandMap(); // Init for command parsing
		

	public:
		Server(int port, const std::string& password);
		~Server();
		void run(); // main loop
		static void	SignalHandler(int signum); // signals
		std::string getHostname() const; // getter


		// Commands
		void handleCommand(int client_fd, const std::string& command);
		void handleJoin(int client_fd, const std::string& command);
		void handlePrivmsg(int client_fd, const std::string& command);
		void handleKick(int client_fd, const std::string& command);
		void handleInvite(int client_fd, const std::string& command);
		void handleTopic(int client_fd, const std::string& command);
		void handleNick(int client_fd, const std::string& command);
		void handleMode(int client_fd, const std::string& command);
		void handlePart(int client_fd, const std::string& command);
		void handleQuit(int client_fd, const std::string& command);
		void handlePing(int client_fd, const std::string& command);

		// Mode options
		void handleModePlusO(int client_fd, const std::string& command);
		void handleModePlusI(int client_fd, const std::string& command);
		void handleModePlusL(int client_fd, const std::string& command);
		void handleModePlusK(int client_fd, const std::string& command);
		void handleModePlusT(int client_fd, const std::string& command);
		void handleModeMinusO(int client_fd, const std::string& command);
		void handleModeMinusI(int client_fd, const std::string& command);
		void handleModeMinusL(int client_fd, const std::string& command);
		void handleModeMinusK(int client_fd, const std::string& command);
		void handleModeMinusT(int client_fd, const std::string& command);
		void handleNicknameModeI(int client_fd, const std::string& channel);

		void sendChannelTopic(int client_fd, Channel* channel); 
		
		void removeClientFromAllChannels(int client_fd);


};

#endif
