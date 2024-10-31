#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <map>
#include <vector>
#include <poll.h>

class Client;

class Server 
{
	private:
		int server_fd;  // Socket du serveur
		std::map<int, Client*> clients;  // Mapping des descripteurs de fichiers à chaque client
		std::vector<struct pollfd> poll_fds;  // Liste des sockets surveillés par poll()
		std::string password;
		std::string hostname;
		std::string servername;
		static bool Signal;

		// Méthodes privées
		void add_poll_fd(int fd, short events);
		void remove_poll_fd(int fd);
		void handle_new_connection();
		void handle_client_data(int client_fd);
		void close_connection(int client_fd);
		void handle_authentication(int client_fd, const std::string& command);
		void send_welcome_messages(Client  *client);

	public:
		Server(int port, const std::string& password);
		~Server();
		void run();
		static void	SignalHandler(int signum);
};

#endif
