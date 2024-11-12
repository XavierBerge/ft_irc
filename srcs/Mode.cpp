/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 21:12:32 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 16:49:33 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handleNicknameModeI(int client_fd, const std::string& channel)
{

    Client* targetClient = NULL;
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
		channel->broadcastMessage(response);
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
    std::istringstream iss(command);
    std::string mode, channelName;
    iss >> mode >> channelName;

    Channel* channel = channels[channelName];

    if (channel->getInvited()) 
    {
        clients[client_fd]->sendToClient(":" + servername + " MODE " + channelName + " :Channel is already in invite-only mode\r\n");
        return;
    }

    channel->setInvited(true);
    std::string response = ":" + clients[client_fd]->getNickname() + " MODE " + channelName + " +i\r\n";
    channel->broadcastMessage(response);

    clients[client_fd]->sendToClient(":" + clients[client_fd]->getNickname() + " MODE " + channelName + " +i :Channel is now invite-only\r\n");
}

void Server::handleModeMinusI(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName;
    iss >> mode >> channelName;

    Channel* channel = channels[channelName];

    if (!channel->getInvited()) 
    {
        clients[client_fd]->sendToClient(":" + servername + " MODE " + channelName + " :Channel is not in invite-only mode\r\n");
        return;
    }

    channel->setInvited(false);
    std::string response = ":" + clients[client_fd]->getNickname() + " MODE " + channelName + " -i\r\n";
    channel->broadcastMessage(response);

    clients[client_fd]->sendToClient(":" + clients[client_fd]->getNickname() + " MODE " + channelName + " -i :Channel doesn't require invite anymore\r\n");
}

void Server::handleModePlusL(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName, option, limitStr;
    iss >> mode >> channelName >> option >> limitStr;

    if (limitStr.empty()) 
    {
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    size_t limitValue;
    std::stringstream ss(limitStr);
    ss >> limitValue;
    if (ss.fail() || !ss.eof()) 
    {
        clients[client_fd]->sendToClient("472 " + clients[client_fd]->getNickname() + " " + limitStr + " :Invalid limit value\r\n");
        return;
    }
	if (limitValue <= 0)
	{
		clients[client_fd]->sendToClient("472 " + clients[client_fd]->getNickname() + " " + limitStr + " Error: limitValue needs to be > 0 \r\n");
		return;
	}
    Channel* channel = channels[channelName];

    if (channel->getLimit()) 
    {
        clients[client_fd]->sendToClient("467 " + clients[client_fd]->getNickname() + " NOTICE " + channelName + " :Channel limit already set\r\n");
        return;
    }
    channel->setLimit(true);
    channel->setLimitValue(limitValue);
    std::string response = ":" + clients[client_fd]->getNickname()+ " MODE " + channelName + " +l " + limitStr + "\r\n";
    channel->broadcastMessage(response);
}

void Server::handleModeMinusL(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName, option;
    iss >> mode >> channelName >> option;

    Channel* channel = channels[channelName];

    if (!channel->getLimit()) 
    {
        clients[client_fd]->sendToClient("467 " + clients[client_fd]->getNickname() + " NOTICE " + channelName + " :No channel limit  set\r\n");
        return;
    }
    channel->setLimit(false);
    channel->setLimitValue(0);
    std::string response = ":" + clients[client_fd]->getNickname()+ " MODE " + channelName + " +l\r\n";
    channel->broadcastMessage(response);
}

void Server::handleModePlusK(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName, option, key;
    iss >> mode >> channelName >> option >> key;

    Channel *channel = channels[channelName];

    if (key.empty()) 
	{
        clients[client_fd]->sendToClient("461 " + clients[client_fd]->getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

if (channel->getKeyNeeded()) 
{
    std::string response = ":" + clients[client_fd]->getNickname() + " NOTICE " + clients[client_fd]->getNickname() + " :Channel key already set for " + channelName + "\r\n";
    clients[client_fd]->sendToClient(response);
    return;
}
    channel->setKey(key);
    channel->setKeyNeeded(true);

    std::string response = ":" + clients[client_fd]->getNickname() + " MODE " + channelName + " +k " + key + "\r\n";
    channel->broadcastMessage(response);
}


void Server::handleModeMinusK(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName, option;
    iss >> mode >> channelName >> option;

    Channel *channel = channels[channelName];


	if (!channel->getKeyNeeded()) 
	{
		std::string response = ":" + clients[client_fd]->getNickname() + " NOTICE " + clients[client_fd]->getNickname() + " : No channel key set for " + channelName + "\r\n";
		clients[client_fd]->sendToClient(response);
		return;
	}
		channel->setKey("");
    channel->setKeyNeeded(false);

    std::string response = ":" + clients[client_fd]->getNickname() + " MODE " + channelName + " -k\r\n";
    channel->broadcastMessage(response);
}

void Server::handleModePlusT(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName;
    iss >> mode >> channelName;

    Channel *channel = channels[channelName];
	
    if (channel->getTopicRight()) 
    {
        clients[client_fd]->sendToClient(": 691 " + clients[client_fd]->getNickname() + " NOTICE  " + channelName + " :Topic protection already enabled\r\n");
        return;
    }
    channel->setTopicRight(true);
    std::string response = ":" + clients[client_fd]->getNickname() + " MODE " + channelName + " +t\r\n";
    channel->broadcastMessage(response);
}

void Server::handleModeMinusT(int client_fd, const std::string& command) 
{
    std::istringstream iss(command);
    std::string mode, channelName;
    iss >> mode >> channelName;

    Channel *channel = channels[channelName];

    if (!channel->getTopicRight()) 
    {
        clients[client_fd]->sendToClient(": 692 " + clients[client_fd]->getNickname() + " NOTICE  " + channelName + " :Topic protection already disabled\r\n");
        return;
    }
    channel->setTopicRight(false);
    std::string response = ":" + clients[client_fd]->getNickname() + " MODE " + channelName + " -t\r\n";
    channel->broadcastMessage(response);
}
