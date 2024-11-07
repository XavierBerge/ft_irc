/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 21:12:32 by xav               #+#    #+#             */
/*   Updated: 2024/11/07 21:15:14 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::handleNicknameModeI(int client_fd, const std::string& channel)
{
    // Vérifie si `channel` est un nickname d'un client valide
    Client* targetClient = NULL;  // Remplacer nullptr par NULL
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second->getNickname() == channel) 
        {
            targetClient = it->second;
            break;
        }
    }

    if (targetClient != NULL)
    {
        targetClient->setInvisible(true);
        std::string response = "MODE " + channel + " +i\r\n";

        if (clients.find(client_fd) != clients.end()) 
        {
            clients[client_fd]->sendToClient(response);
        }
    }
}



void Server::handleModePlusO(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
	std::string mode, channelName, option, nickname;
	iss >> mode >> channelName >> option >> nickname; 

	if (nickname.empty())
		clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " MODE :Not enough parameters\r\n");
	

	int target_client_fd = -1;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        if (it->second->getNickname() == nickname) 
		{
            target_client_fd = it->first;
            break;
        }
    }

    if (target_client_fd == -1) 
	{
        clients[client_fd]->sendToClient("401 " + clients[client_fd]->getNickname() + " " + nickname + " :No such nick/channel\r\n");
        return;
    }
	
	Channel *channel = channels[channelName];

	if (!channel->isClientInChannel(nickname)) 
	{
        clients[client_fd]->sendToClient("441 " + clients[client_fd]->getNickname() + " " + nickname + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }
	if (!channel->isOperator(nickname))
	{
		channel->promoteToOperator(nickname, target_client_fd);
		std::string response = ":" +  clients[client_fd]->getNickname() + " MODE " + channelName + " +o " + nickname + "\r\n";
		channel->broadcastMessage(response);  // Broadcast à tout le canal	
	}
	else
		clients[client_fd]->sendToClient(":" + servername + " " + nickname + " " + channelName + " : User is already operator on that channel\r\n");
}

void Server::handleModeMinusO(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
	std::string mode, channelName, option, nickname;
	iss >> mode >> channelName >> option >> nickname; 

	if (nickname.empty())
		clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " MODE :Not enough parameters\r\n");
	

	int target_client_fd = -1;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        if (it->second->getNickname() == nickname) 
		{
            target_client_fd = it->first;
            break;
        }
    }
	if (target_client_fd == -1) 
	{
        clients[client_fd]->sendToClient("401 " + clients[client_fd]->getNickname() + " " + nickname + " :No such nick/channel\r\n");
        return;
    }
	Channel *channel = channels[channelName];

	if (!channel->isClientInChannel(nickname)) 
	{
        clients[client_fd]->sendToClient("441 " + clients[client_fd]->getNickname() + " " + nickname + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }
	if(channel->isOperator(nickname))
	{
		channel->removeOperator(nickname);
		std::string response = ":" +  clients[client_fd]->getNickname() + " MODE " + channelName + " -o " + nickname + "\r\n";
		channel->broadcastMessage(response);
	}
	else
		clients[client_fd]->sendToClient(":" + servername + " " + nickname + " " + channelName + " : User is not operator on that channel\r\n");
}

void Server::handleModePlusI(int client_fd, const std::string& command) 
{
    std::cout << "handleModePlusI called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusI(int client_fd, const std::string& command) 
{
    std::cout << "handleModeMinusI called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusL(int client_fd, const std::string& command) 
{
    std::cout << "handleModePlusL called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusL(int client_fd, const std::string& command) 
{
    std::cout << "handleModeMinusL called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusK(int client_fd, const std::string& command) 
{
    std::cout << "handleModePlusK called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusK(int client_fd, const std::string& command) 
{
    std::cout << "handleModeMinusK called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModePlusT(int client_fd, const std::string& command) 
{
    std::cout << "handleModePlusT called with command: " << command << " for client_fd: " << client_fd << std::endl;
}

void Server::handleModeMinusT(int client_fd, const std::string& command) 
{
    std::cout << "handleModeMinusT called with command: " << command << " for client_fd: " << client_fd << std::endl;
}
