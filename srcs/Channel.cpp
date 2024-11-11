/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:11:14 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 15:21:04 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <iostream>
#include <ctime>

Channel::Channel(const std::string &channelName) 
	: name(channelName), _invited(false),_key(""), _keyNeeded(false), _limit(false), _topic(""), _topicRight(false), _topicOpe(""), _topicTime(0) {}


Channel::~Channel() {}

//setters
void Channel::setLimitValue(size_t limitValue) {this->_limitValue = limitValue;}
void Channel::setLimit(bool limit){this->_limit = limit;}
void Channel::setKey(std::string key) {this->_key = key;}
void Channel::setInvited(bool invited){this->_invited = invited;}
void Channel::setTopic(std::string topic) {this->_topic = topic;}
bool Channel::getLimit() const {return _limit;}
void Channel::setTopicOpe(std::string topicOpe) {this->_topicOpe = topicOpe;}
void Channel::setTopicTime(time_t topicTime) {this->_topicTime = topicTime;}
void Channel::setTopicRight(bool topicRight) {this->_topicRight = topicRight;}
void Channel::setKeyNeeded(bool keyNeeded) {this->_keyNeeded = keyNeeded;}

//getters
size_t Channel::getLimitValue() const {return _limitValue;}
std::string Channel::getKey() const {return _key;}
bool Channel::getKeyNeeded() const {return _keyNeeded;}
bool Channel::getInvited() const {return _invited;}
std::string Channel::getTopic() const {return this->_topic;}
std::string Channel::getTopicOpe() const {return this->_topicOpe;}
time_t Channel::getTopicTime() const {return this->_topicTime;}
bool Channel::getTopicRight() const {return this->_topicRight;}
std::string Channel::getName() const {return name;}
bool Channel::isEmpty() const {return clients.empty();}
bool Channel::hasOperators() const {return !operators.empty();}
bool Channel::isClientInChannel(const std::string &nickname) const {return clients.find(nickname) != clients.end();}
bool Channel::isOperator(const std::string &nickname) const {return operators.find(nickname) != operators.end();}


// std::map clients methods
bool Channel::addClient(const std::string &nickname, int client_fd) 
{
    return clients.insert(std::make_pair(nickname, client_fd)).second;
}
void Channel::removeClient(const std::string &nickname) {clients.erase(nickname);}

//std::map operators methods
void Channel::removeOperator(const std::string &nickname) {operators.erase(nickname);}
void Channel::promoteToOperator(const std::string &nickname, int client_fd) 
{
    if (isClientInChannel(nickname)) 
    {
        operators.insert(std::make_pair(nickname, client_fd));
    }
}

void Channel::promoteNextOperator(int client_fd) 
{
    std::map<std::string, int>::iterator it;

    for (it = clients.begin(); it != clients.end(); ++it) 
    {
        const std::string& nickname = it->first;
        if (!isOperator(nickname))
        { 
            operators.insert(std::make_pair(nickname, client_fd));
            std::string promoteMessage = "MODE " + name + " +o " + nickname + "\r\n";
            broadcastMessage(promoteMessage);
            break;
        }
    }
}
// couting ppls in channel based on operators and clients maps
int Channel::count() const 
{
    int uniqueClientsCount = 0;

    for (std::map<std::string, int>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
	{
        if (operators.find(it->first) == operators.end()) 
            ++uniqueClientsCount;
    }
    int totalCount = uniqueClientsCount + operators.size();
    return totalCount;
}
// send messages in channels methods
void Channel::broadcastMessage(const std::string &message, int sender_fd) 
{
    for (std::map<std::string, int>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->second != sender_fd) 
            send(it->second, message.c_str(), message.size(), 0);
    }
}

void Channel::broadcastMessage(const std::string &message) 
{
    for (std::map<std::string, int>::const_iterator it = clients.begin(); it != clients.end(); ++it) 
        send(it->second, message.c_str(), message.size(), 0);
}









