/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:45 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 12:37:19 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd) 
    : socket_fd(fd), nickname(""), authenticated(false), irssi(false), pass_ok(false), nick_ok(false), invisible(false) {}

Client::~Client() 
{
    close(socket_fd);
    std::cout << "Client " << nickname << " disconnected." << std::endl;
}

void Client::setInvisible(bool invisible) {this->invisible = invisible;}
bool Client::isInvisible() const { return invisible;}

int Client::getSocketFd() const 
{
    return socket_fd;
}

void Client::setNickname(const std::string& nick) 
{
    this->nickname = nick;
}

std::string Client::getNickname() const 
{
    return nickname;
}

void Client::setHostname(const std::string& host) 
{
    hostname = host;
}

std::string Client::getHostname() const 
{
    return hostname;
}


void Client::setUsername(const std::string& user) 
{
    username = user;
}

std::string Client::getUsername() const 
{
    return username;
}

void Client::setRealname(const std::string& name) 
{
    realname = name;
}

bool Client::isAuthenticated() const 
{
    return authenticated;
}
std::string Client::getRealname() const
{
	return realname;
}

bool Client::isIrssi() const
{
	return irssi;
}

bool Client::isPass_Ok() const
{
	return pass_ok;
}

bool Client::isNick_Ok() const
{
	return nick_ok;
}

void Client::irssi_true()
{
	irssi = true;
}
void Client::pass_true()
{
	pass_ok = true;
}

void Client::nick_true()
{
	nick_ok = true;
}


void Client::authenticate() 
{
    authenticated = true;
}

void Client::sendToClient(const std::string& message) 
{
    send(socket_fd, message.c_str(), message.size(), 0);
	std::cout << "Server send to " << nickname << " :\n" << message;
}

ssize_t Client::readFromClient(char *tempBuffer, size_t size) 
{
    ssize_t bytes_received = recv(socket_fd, tempBuffer, size, 0);
    if (bytes_received > 0) 
        buffer.append(tempBuffer, bytes_received);
    return bytes_received;
}

// Vérifie si le buffer interne contient une ligne complète (terminée par '\n')
bool Client::hasCompleteLine() const 
{
    return buffer.find('\n') != std::string::npos;
}

// Récupère la première ligne complète et la supprime du buffer interne
std::string Client::getCompleteLine() 
{
    size_t pos = buffer.find('\n');
    if (pos != std::string::npos) 
	{
        std::string completeLine = buffer.substr(0, pos + 1); // Inclut le '\n'
        buffer.erase(0, pos + 1); // Supprime la ligne du buffer
        return completeLine;
    }
    return "";
}


bool Client::isBufferEmpty() const 
{
    return buffer.empty();
}

void Client::addChannelInvitation(const std::string& channelName) 
{
    channelInvitations.push_back(channelName);
}

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


void Client::clearChannels() 
{
    client_channels.clear();
}

const std::vector<std::string>& Client::getChannels() const { return client_channels;}



