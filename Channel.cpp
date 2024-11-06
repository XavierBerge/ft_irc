/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:14 by xav               #+#    #+#             */
/*   Updated: 2024/11/06 16:20:42 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>

Channel::Channel(const std::string &channelName) : name(channelName) {}

Channel::~Channel() {}

std::string Channel::getName() const 
{
    return name;
}

bool Channel::addClient(const std::string &nickname, int client_fd) 
{
    return clients.insert(std::make_pair(nickname, client_fd)).second;
}

void Channel::removeClient(const std::string &nickname) 
{
    clients.erase(nickname);
    operators.erase(nickname);
}

bool Channel::isClientInChannel(const std::string &nickname) const 
{
    return clients.find(nickname) != clients.end();
}

bool Channel::isOperator(const std::string &nickname) const 
{
    return operators.find(nickname) != operators.end();
}

void Channel::promoteToOperator(const std::string &nickname, int client_fd) 
{
    if (isClientInChannel(nickname)) 
    {
        operators.insert(std::make_pair(nickname, client_fd));
    }
}

void Channel::broadcastMessage(const std::string &message, int sender_fd) 
{
    for (std::map<std::string, int>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second != sender_fd) 
        {
            send(it->second, message.c_str(), message.size(), 0);
        }
    }
}

void Channel::broadcastMessage(const std::string &message) 
{
    // Parcours de tous les clients du canal, y compris celui qui a envoyé le message
    for (std::map<std::string, int>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        // Envoi du message à tous les clients, y compris l'émetteur
        send(it->second, message.c_str(), message.size(), 0);
    }
}

int Channel::count() const 
{
    int uniqueClientsCount = 0;

    // Parcourt les clients et ne compte que ceux qui ne sont pas opérateurs
    for (std::map<std::string, int>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        if (operators.find(it->first) == operators.end()) 
            ++uniqueClientsCount;
    }

    // Ajoute le nombre total d'opérateurs
    int totalCount = uniqueClientsCount + operators.size();
    return totalCount;
}




