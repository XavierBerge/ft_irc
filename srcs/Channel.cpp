/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:14 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 12:51:50 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>
#include <ctime>

Channel::Channel(const std::string &channelName) 
	: name(channelName), _invited(false),_key(""), _keyNeeded(false), _limit(false), _topic(""), _topicRight(false), _topicOpe(""), _topicTime(0) {}


size_t Channel::getLimitValue() const
{
	return _limitValue;
}

void Channel::setLimitValue(size_t limitValue)
{
	this->_limitValue = limitValue;
}

void Channel::setLimit(bool limit)
{
	this->_limit = limit;
}

bool Channel::getLimit() const
{
	return _limit;
}

void Channel::setKey(std::string key)
{
	this->_key = key;
}
std::string Channel::getKey() const
{
	return _key;
}

bool Channel::getKeyNeeded() const
{
	return _keyNeeded;
}

void Channel::setKeyNeeded(bool keyNeeded)
{
	this->_keyNeeded = keyNeeded;
}
bool Channel::getInvited() const
{
	return _invited;
}


void Channel::setInvited(bool invited)
{
	this->_invited = invited;
}

Channel::~Channel() {}

std::string Channel::getTopic() const {return this->_topic;}
std::string Channel::getTopicOpe() const {return this->_topicOpe;}
time_t Channel::getTopicTime() const {return this->_topicTime;}
bool Channel::getTopicRight() const {return this->_topicRight;}
void Channel::setTopic(std::string topic) {this->_topic = topic;}
void Channel::setTopicOpe(std::string topicOpe) {this->_topicOpe = topicOpe;}
void Channel::setTopicTime(time_t topicTime) {this->_topicTime = topicTime;}
void Channel::setTopicRight(bool topicRight) {this->_topicRight = topicRight;}


std::string Channel::getName() const 
{
    return name;
}

bool Channel::addClient(const std::string &nickname, int client_fd) 
{
    return clients.insert(std::make_pair(nickname, client_fd)).second;
}

void Channel::removeOperator(const std::string &nickname)
{
	operators.erase(nickname);
}

void Channel::removeClient(const std::string &nickname) 
{
    clients.erase(nickname);
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

bool Channel::isEmpty() const 
{
    return clients.empty();
}



bool Channel::hasOperators() const 
{
    return !operators.empty();
}



void Channel::promoteNextOperator(int client_fd) 
{
    // Itérateur pour parcourir la map des clients
    std::map<std::string, int>::iterator it;

    // Parcours de tous les clients dans le canal
    for (it = clients.begin(); it != clients.end(); ++it) 
    {
        const std::string& nickname = it->first;  // Le nickname du client
        if (!isOperator(nickname))  // Si le client n'est pas déjà opérateur
        { 
            // Ajoute le client à la map des opérateurs (nickname, client_fd)
            operators.insert(std::make_pair(nickname, client_fd));

            // Création du message de promotion pour l'opérateur
            std::string promoteMessage = "MODE " + name + " +o " + nickname + "\r\n";
            broadcastMessage(promoteMessage);  // Envoie le message à tous les clients

            break;  // Arrête la boucle après avoir promu un opérateur
        }
    }
}







