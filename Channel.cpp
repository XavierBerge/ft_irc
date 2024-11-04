/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:14 by xav               #+#    #+#             */
/*   Updated: 2024/11/04 09:44:47 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>


Channel::Channel(const std::string &channelName, Server& server) 
    : name(channelName), serverInstance(server) {}


Channel::~Channel() 
{
    clients.clear();
    operators.clear();
}

std::string Channel::getName() const 
{
    return name;
}

bool Channel::addClient(int client_fd) 
{
    if (clients.find(client_fd) == clients.end()) 
	{
        clients.insert(client_fd);
        std::cout << "Client " << client_fd << " a rejoint le channel " << name << std::endl;
        return true;
    }
    return false;
}

void Channel::removeClient(int client_fd) 
{
    clients.erase(client_fd);
    operators.erase(client_fd);
    std::cout << "Client " << client_fd << " a quitté le channel " << name << std::endl;
}

bool Channel::isClientInChannel(int client_fd) const 
{
    return clients.find(client_fd) != clients.end();
}

bool Channel::isOperator(int client_fd) const 
{
    return operators.find(client_fd) != operators.end() && operators.at(client_fd);
}

void Channel::promoteToOperator(int client_fd) 
{
    if (isClientInChannel(client_fd)) 
    {
        operators[client_fd] = true;
        std::string nickname = serverInstance.getNicknameByFd(client_fd);  // Récupère le pseudo
        std::string modeMessage = ":" + nickname + " MODE " + name + " +o " + nickname + "\r\n";

        // Envoie le message à tous les clients du channel
        for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) 
        {
            send(*it, modeMessage.c_str(), modeMessage.size(), 0);
        }
    }
}


void Channel::broadcastMessage(const std::string &message, int sender_fd) 
{
	for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) 
	{
		int client_fd = *it;
		if (client_fd != sender_fd)
			send(client_fd, message.c_str(), message.size(), 0);
	}
}


