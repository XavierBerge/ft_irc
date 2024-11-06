/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 13:26:27 by xav               #+#    #+#             */
/*   Updated: 2024/11/06 13:45:11 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <algorithm> 

void Server::handleJoin(int client_fd, const std::string& command) 
{
    std::string channelName = command.substr(5);

    size_t start = 0;
    while (start < channelName.size() && isspace(channelName[start])) 
        ++start;
    size_t end = channelName.size();
    while (end > start && isspace(channelName[end - 1])) 
        --end;
    channelName = channelName.substr(start, end - start);
    
    if (channelName.empty()) 
    {
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

    bool firstToJoin = false;
    if (channels.find(channelName) == channels.end())
    {
        channels[channelName] = new Channel(channelName);
        firstToJoin = true;
    }

    Channel *channel = channels[channelName];
    std::string nickname = clients[client_fd]->getNickname();

    if (channel->addClient(nickname, client_fd))
    {
        std::string joinMessage = ":" + clients[client_fd]->getNickname() + "!" + clients[client_fd]->getUsername() + "@" + clients[client_fd]->getHostname() + " JOIN " + channelName + "\r\n";
        if (!firstToJoin)
			channel->broadcastMessage(joinMessage);
        clients[client_fd]->sendToClient("Vous avez rejoint " + channelName + "\r\n");

        if (firstToJoin)
		{
			
            channel->promoteToOperator(nickname, client_fd); // Utilise le nickname pour promouvoir l'opérateur
			std::string modemsg = ":" + servername + " MODE " + channelName + " +o " + nickname + "\r\n";
			channel->broadcastMessage(modemsg);
		}
    } 
    else 
    {
        clients[client_fd]->sendToClient("Vous êtes déjà dans " + channelName + "\r\n");
    }
}


void Server::handlePrivmsg(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handlePrivmsg called with command: " << command << std::endl;
}

void Server::handleKick(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleKick called with command: " << command << std::endl;
}

void Server::handleInvite(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleInvite called with command: " << command << std::endl;
}

void Server::handleTopic(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleTopic called with command: " << command << std::endl;
}

void Server::handleMode(int client_fd, const std::string& command) 
{
    std::map<std::string, void (Server::*)(int, const std::string&)> modeMap;

    modeMap["+o"] = &Server::handleModePlusO;
    modeMap["-o"] = &Server::handleModeMinusO;
    modeMap["+i"] = &Server::handleModePlusI;
    modeMap["-i"] = &Server::handleModeMinusI;
    modeMap["+l"] = &Server::handleModePlusL;
    modeMap["-l"] = &Server::handleModeMinusL;
    modeMap["+k"] = &Server::handleModePlusK;
    modeMap["-k"] = &Server::handleModeMinusK;
    modeMap["+t"] = &Server::handleModePlusT;
    modeMap["-t"] = &Server::handleModeMinusT;


    std::istringstream iss(command);
    std::string commandType, channel, modeOption;
    iss >> commandType >> channel >> modeOption;

    if (!modeOption.empty() && modeMap.find(modeOption) != modeMap.end()) 
        (this->*modeMap[modeOption])(client_fd, command);
	else 
        std::cout << "Unknown or missing mode option in command: " << command << std::endl;
}


void Server::handlePart(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handlePart called with command: " << command << std::endl;
}

void Server::handlePing(int client_fd, const std::string& command) 
{
    std::string response = "PONG :";
    std::string param = command.substr(5);  // Extraction du paramètre après "PING "
    response += param + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handleQuit(int client_fd, const std::string& command) 
{
	(void) command;
    // Récupérer le client associé au client_fd
    Client* client = clients[client_fd];

    // Vérifier si le client existe et récupérer le nickname
    std::string nickname = (client && client->isNick_Ok()) ? client->getNickname() : "Unknown";

    // Afficher le message de déconnexion avec le nickname
    std::cout << "Client " << nickname << " has quit the server.\n";

    // Supprimer le client de la map et fermer la connexion
    close_connection(client_fd);
}

void Server::handleNick(int client_fd, const std::string& command) 
{
    Client* client = clients[client_fd];


    // Vérifier si le client a passé le mot de passe
    if (!client->isPass_Ok()) 
	{
        std::string error = "You must enter the server password before choosing a nickname : PASS <password>.\r\n";
        send(client_fd, error.c_str(), error.size(), 0);
        return;
    }

    // Récupérer le nickname depuis la commande
    std::string nickname = command.substr(5);

    // Vérifier l'unicité du nickname
    bool nickname_taken = false;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        if (it->second->getNickname() == nickname) 
		{
            nickname_taken = true;
            break;
        }
    }

    if (nickname_taken) 
	{
        std::string response = "433 * " + nickname + " :This nickname is already taken. Please choose a different nickname.\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }

    // Définir le nickname s'il est unique
    client->setNickname(nickname);
	if (!client->isAuthenticated())
	{
		if (!client->isIrssi()) 
		{
			std::string response = nickname + " :NICK accepted. Please enter a username: USER <username> <hostname> <servername> :<realname>\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
		}
		client->nick_true();
	}
}