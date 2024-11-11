/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:00 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 20:47:33 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

bool Server::Signal = false;

void Server::initializeCommandMap() 
{
    commandMap["JOIN"] = &Server::handleJoin;
    commandMap["PRIVMSG"] = &Server::handlePrivmsg;
    commandMap["KICK"] = &Server::handleKick;
    commandMap["INVITE"] = &Server::handleInvite;
    commandMap["TOPIC"] = &Server::handleTopic;
    commandMap["NICK"] = &Server::handleNick;
    commandMap["MODE"] = &Server::handleMode;
    commandMap["PART"] = &Server::handlePart;
    commandMap["QUIT"] = &Server::handleQuit;
    commandMap["PING"] = &Server::handlePing;
}

Server::Server(int port, const std::string &password) : password(password), hostname("client.host"), servername("irc.tower.com")
{
    // AF_INET = IPv4, SOCK_STREAM = TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
		std::cerr << RED; 
        perror("socket");
		std::cerr << RESET;
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    // SO_REUSEADDR option allows restarting the server directly after it has been closed before its address and port are fully released
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
		std::cerr << RED; 
        perror("setsockopt");
		std::cerr << RESET;
        exit(EXIT_FAILURE);
    }
    // Set socket to non-blocking mode = Program will not block on read, write, send, recv if they are impossible
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
    {
		std::cerr << RED; 
        perror("fcntl");
		std::cerr << RESET;
        exit(EXIT_FAILURE);
    }
    // IP address configuration
    struct sockaddr_in server_addr; // struct for client IP address information
    memset(&server_addr, 0, sizeof(server_addr)); // initialize everything to 0 to avoid random values
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // any local IP address
    server_addr.sin_port = htons(port); // listening port converted to network byte order

    // Bind the socket to IP address and port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
		std::cerr << RED;
        perror("bind"); 
		std::cerr << RESET;
        exit(EXIT_FAILURE);
    }
    // Enable socket listening
    if (listen(server_fd, SOMAXCONN) < 0)
    {
		std::cerr << RED;
        perror("listen");
		std::cerr << RESET;
        exit(EXIT_FAILURE);
    }
    // Add the socket to poll_fds at index 3 (0 = stdin, 1 = stdout, 2 = stderr)
    add_poll_fd(server_fd, POLLIN);
    initializeCommandMap();
}

Server::~Server() 
{
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) 
    {
        delete it->second;
    }
    channels.clear();

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        close(it->first);
        delete it->second;
    }
    clients.clear();
    if (server_fd >= 0) 
        close(server_fd);
    
}

std::string Server::getHostname() const
{
    return hostname;
}

// Adds fd to poll_fds
void Server::add_poll_fd(int fd, short events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    poll_fds.push_back(pfd);
}
// Removes a fd from poll_fds
void Server::remove_poll_fd(int fd)
{
    for (std::vector <struct pollfd>:: iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            poll_fds.erase(it);
            break;
        }
    }
}


// Main server loop
void Server::run()
{
    while (!Server::Signal)  // Loop until a termination signal is received
    {
        // Call poll, check the size and handle signals
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);

        if (poll_count < 0) 
        {
            if (errno == EINTR) 
                break; // If poll is interrupted by a signal, exit the loop
            else 
            {
				std::cerr << RED; 
                perror("poll");
				std::cerr << RESET;
                exit(EXIT_FAILURE);
            }
        }

        // Traverse descriptors to check events
        for (size_t i = 0; i < poll_fds.size(); ++i) 
        {
            // Handle read events
            if (poll_fds[i].revents & POLLIN) 
            {
                if (poll_fds[i].fd == server_fd)
                    handle_new_connection();  // Handle a new connection to the server
                else
                    handle_client_data(poll_fds[i].fd);  // Handle data from an existing client
            }

            // Handle errors and disconnections
            if (poll_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) 
            {
                if (poll_fds[i].fd != server_fd) 
                {
                    std::cout << "Disconnection or error on client " << poll_fds[i].fd << std::endl;
                    close_connection(poll_fds[i].fd);  // Close the client's connection in case of error
                }
            }
        }
    }
    
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) 
    {
        delete it->second;
    }
    channels.clear();

    std::cout << RED << "\nSignal received." << RESET << std::endl;
    // Close all client sockets and free associated memory
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        close(it->first); // Close the client's file descriptor
        delete it->second; // Free memory associated with each Client object
    }
    clients.clear(); // Clear the clients map

    // Close server socket if still open
    if (server_fd >= 0) 
    {
        close(server_fd);
    }

}

void Server::close_connection(int client_fd)
{
    close(client_fd);
    remove_poll_fd(client_fd);
    delete clients[client_fd];
    clients.erase(client_fd);
}

void Server::handle_client_data(int client_fd) 
{
    Client *client = clients[client_fd];
    char tempBuffer[1024];
    memset(tempBuffer, 0, sizeof(tempBuffer));

    // Read data sent by the client
    ssize_t bytes_received = client->readFromClient(tempBuffer, sizeof(tempBuffer));

    if (bytes_received <= 0) 
    {
        // Handle disconnection or error
        if (bytes_received == 0) 
		{
            std::cout << "Client " << client_fd << " shut down the connection." << std::endl;
        } 
        else 
            perror("recv");
        close_connection(client_fd);
        return;
    }

    // While there is a complete line to process in the buffer
    while (client->hasCompleteLine()) 
    {
        // Retrieve and process the first complete line
        std::string command = client->getCompleteLine();
        
        // Remove end-of-line characters for processing
        command.erase(command.find_last_not_of("\r\n") + 1);

        // Process the command
        if (!client->isAuthenticated()) 
            handle_authentication(client_fd, command);
        else 
        {
            handleCommand(client_fd, command);
            if (command.find("/quit") == 0) 
            {
                close_connection(client_fd);
                return;
            }
        }
    }
}

void Server::SignalHandler(int signum)
{
	(void)signum;
	Server::Signal = true;
}


void Server::removeClientFromAllChannels(int client_fd) 
{
    Client* client = clients[client_fd];
    if (!client) return;

    std::string nickname = client->getNickname();
    const std::vector<std::string>& clientChannels = client->getChannels();

   	std::vector<std::string>::const_iterator it;
    for (it = clientChannels.begin(); it != clientChannels.end(); ++it) 
	{
        const std::string& channelName = *it;

        Channel* channel = channels[channelName];
        if (channel)
            channel->removeClient(nickname);

        std::string quitMessage = ":" + nickname + "!" + client->getUsername() + "@" + client->getHostname() + " QUIT :Client disconnected\r\n";
        channel->broadcastMessage(quitMessage, client_fd);

        if (channel->isOperator(nickname)) 
		{
            channel->removeOperator(nickname);
            if (!channel->hasOperators()) 
			{
                channel->promoteNextOperator(client_fd);
            }
        }

        if (channel->isEmpty()) {
            delete channel;
            channels.erase(channelName);
        }
    }


    client->clearChannels();
}
