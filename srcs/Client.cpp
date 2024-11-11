/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:45 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 20:51:07 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd) 
    : socket_fd(fd), nickname(""), authenticated(false), irssi(false), pass_ok(false), nick_ok(false), invisible(false) {}

Client::~Client() 
{
    close(socket_fd);
    std::cout << RED << "Client " << nickname << " disconnected." << RESET << std::endl;
}

//getters
int Client::getSocketFd() const {return socket_fd;}
std::string Client::getNickname() const {return nickname;}
bool Client::isInvisible() const { return invisible;}
const std::vector<std::string>& Client::getChannels() const { return client_channels;}
std::string Client::getHostname() const {return hostname;}
std::string Client::getUsername() const {return username;}
std::string Client::getRealname() const{return realname;}
bool Client::isIrssi() const{return irssi;}
bool Client::isPass_Ok() const{return pass_ok;}
bool Client::isAuthenticated() const {return authenticated;}
bool Client::isNick_Ok() const{return nick_ok;}

//setters
void Client::setInvisible(bool invisible) {this->invisible = invisible;}
void Client::setNickname(const std::string& nick) {this->nickname = nick;}
void Client::setHostname(const std::string& host) {hostname = host;}
void Client::setUsername(const std::string& user) {username = user;}
void Client::setRealname(const std::string& name) {realname = name;}
void Client::irssi_true(){irssi = true;}
void Client::pass_true(){pass_ok = true;}
void Client::nick_true(){nick_ok = true;}
void Client::authenticate() {authenticated = true;}

// Send method for server to client 
void Client::sendToClient(const std::string& message) 
{
    send(socket_fd, message.c_str(), message.size(), 0);
	std::cout << "\033[1;93m"
          << "Server send to " 
          << "\033[1;32m"
          << nickname 
          << "\033[1;35m"
          << " :\n" << message 
          << "\033[0m" << std::endl;
}
//Buffer methods
ssize_t Client::readFromClient(char *tempBuffer, size_t size) 
{
    ssize_t bytes_received = recv(socket_fd, tempBuffer, size, 0);
    if (bytes_received > 0) 
        buffer.append(tempBuffer, bytes_received);
    return bytes_received;
}
std::string Client::getCompleteLine() 
{
    size_t pos = buffer.find('\n');
    if (pos != std::string::npos) 
	{
        std::string completeLine = buffer.substr(0, pos + 1);
        buffer.erase(0, pos + 1);
        return completeLine;
    }
    return "";
}
bool Client::hasCompleteLine() const {return buffer.find('\n') != std::string::npos;}
bool Client::isBufferEmpty() const {return buffer.empty();}

// Channels vectors methods
bool Client::isInvitedToChannel(const std::string& channelName) const 
{
    for (std::vector<std::string>::const_iterator it = channelInvitations.begin(); it != channelInvitations.end(); ++it) 
    {
        if (*it == channelName)
            return true;
    }
    return false;
}

void Client::addChannel(const std::string& channelName) 
{
    if (std::find(client_channels.begin(), client_channels.end(), channelName) == client_channels.end()) 
        client_channels.push_back(channelName);
}

void Client::removeChannel(const std::string& channelName) 
{
    std::vector<std::string>::iterator it = std::find(client_channels.begin(), client_channels.end(), channelName);
    if (it != client_channels.end()) 
        client_channels.erase(it);
}
void Client::addChannelInvitation(const std::string& channelName) {channelInvitations.push_back(channelName);}
void Client::clearChannels() {client_channels.clear();}




