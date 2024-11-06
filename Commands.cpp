/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 13:26:27 by xav               #+#    #+#             */
/*   Updated: 2024/11/06 09:11:01 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::handleJoin(int client_fd, const std::string& command) 
{
	(void) client_fd;
    std::cout << "handleJoin called with command: "  << command << std::endl;
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