/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Authentification.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:02:38 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 20:33:35 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handle_new_connection()
{
    if (clients.size() >= MAX_CLIENTS)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int temp_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        if (temp_fd >= 0) 
        {
            std::string full_message = "Server is full. Connection refused.\r\n";
            send(temp_fd, full_message.c_str(), full_message.size(), 0);
            close(temp_fd);
        }
        return;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0)
    {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            perror("accept");
        return; 
    }

    // Use inet_ntoa() to get the client's IP address
    std::cout << "\033[1;32m"
              << "New connection accepted : " << client_fd 
              << " from " << inet_ntoa(client_addr.sin_addr) 
              << ":" << ntohs(client_addr.sin_port)
              << "\033[0m"
              << std::endl;

    // Make the client socket non-blocking
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        close(client_fd);
        return; 
    }

    // Add the client to the list of connected clients
    Client *new_client = new Client(client_fd);
    clients[client_fd] = new_client;

    // Add the client descriptor to poll() to monitor I/O events
    add_poll_fd(client_fd, POLLIN);

    // Send a password request message to the client
    std::string welcome_message = "Please enter the server password to get access : PASS <password>\r\n";
    if (send(client_fd, welcome_message.c_str(), welcome_message.size(), 0) < 0)
    {
        perror("send");
        close_connection(client_fd); // Close if the send fails
        return;
    }
}

void Server::handle_authentication(int client_fd, const std::string& data) 
{
    Client *client = clients[client_fd];
    std::stringstream ss(data);
    std::string line;
    
    while (std::getline(ss, line)) 
    {
        if (!line.empty() && line[line.size() - 1] == '\r') 
            line.erase(line.size() - 1);

        std::cout << "\033[1;94m"
          << "Received command from " 
          << "\033[1;32m"
          << client->getNickname() 
          << "\033[1;36m"
          << " : \n" << line 
          << "\033[0m\n" << std::endl;


        if (line.find("CAP LS") == 0) 
        {
            client->irssi_true();
            continue;
        }

        if (line.find("PASS ") == 0) 
        {
            std::string client_pass = line.substr(5);
            if (client_pass == password) 
            {
                if (!client->isIrssi())
                {
                    std::string response = "Correct password. Please enter your nickname : NICK <nickname>.\r\n";
                    send(client_fd, response.c_str(), response.size(), 0);
                }
                client->pass_true();
            } 
            else 
            {
                std::string error = "Wrong password. Try to reconnect with the correct password\r\n";
				send(client_fd, error.c_str(), error.size(), 0);
                close_connection(client_fd);
                return;
            }
            continue;
        }

        if (line.find("NICK ") == 0) 
        {
            handleNick(client_fd, line);
            continue;
        }

        if (line.find("USER ") == 0) 
        {
            if (!client->isNick_Ok())
            {
                std::string error = "Please first define your nickname : NICK <nickname>.\r\n";
                send(client_fd, error.c_str(), error.size(), 0);
                continue;
            }

            // Extract the parameters from the USER command
            std::istringstream iss(line);
            std::string command, username, hostname, servername, realname;
            iss >> command >> username >> hostname >> servername;
            std::getline(iss, realname);

            realname.erase(0, realname.find_first_not_of(" \t"));
            realname.erase(realname.find_last_not_of(" \t") + 1);
            if (!realname.empty() && realname[0] == ':')
                realname = realname.substr(1);

            if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
            {
                std::string error = "461 " + client->getNickname() + " USER :Not enough parameters\r\n";
                send(client_fd, error.c_str(), error.size(), 0);
                continue;
            }
			std::string realHostname = getHostname();
            // Update client information
            client->setUsername(username);
            client->setRealname(realname);
			client->setHostname(realHostname);
            client->authenticate();
            send_welcome_messages(client);
            return;  // Stop processing after successful authentication
        }

        // Default error message if the command is unknown
        std::string response = "In authentication process PASS, NICK and USER must be defined, command : " + line + " is not valid.\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}

void Server::send_welcome_messages(Client *client)
{
    std::string nickname = client->getNickname();
    std::string username = client->getUsername();
    std::string realname = client->getRealname();
    std::string hostname = this->hostname;
    std::string servername = this->servername;
    
    std::string response = "001 " + nickname + " :Welcome to the Internet Relay Network " +
                           nickname + "!" + username + "@" + hostname + "\r\n";
    client->sendToClient(response);

    response = "002 " + nickname + " :Your host is " + servername + ", running version 1.0\r\n";
    client->sendToClient(response);

    std::time_t now = std::time(NULL);
    std::tm* local_time = std::localtime(&now);

    char date_str[100];
    std::strftime(date_str, sizeof(date_str), "%a %b %d %Y at %H:%M", local_time);

    response = "003 " + nickname + " :This server was created " + date_str + "\r\n";
    client->sendToClient(response);

    response = "004 " + nickname + " " + servername + " 1.0 iowghraAsORTVSx \r\n";
    client->sendToClient(response);

    response = "375 " + nickname + " :- " + servername + " Message of the day -\r\n";
    client->sendToClient(response);

    response = "372 " + nickname + " :- Welcome to the Tower IRC server!\r\n";
    client->sendToClient(response);

    response = "376 " + nickname + " :End of /MOTD command\r\n";
    client->sendToClient(response);
}


