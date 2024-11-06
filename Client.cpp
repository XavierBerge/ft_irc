/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:45 by xav               #+#    #+#             */
/*   Updated: 2024/11/06 16:20:12 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd, Server* serverInstance) 
    : socket_fd(fd), nickname(""), authenticated(false), irssi(false), pass_ok(false), nick_ok(false), server(serverInstance) {}

Client::~Client() 
{
    close(socket_fd);
    std::cout << "Client " << nickname << " disconnected." << std::endl;
}

int Client::getSocketFd() const 
{
    return socket_fd;
}

void Client::setNickname(const std::string& nick) 
{
    nickname = nick;
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
/*
ssize_t Client::readFromClient(char *buffer, size_t size) 
{
    return recv(socket_fd, buffer, size, 0);
}
*/

void Client::sendToClient(const std::string& message) 
{
    send(socket_fd, message.c_str(), message.size(), 0);
	std::cout << "Server send to " << nickname << " :\n" << message;
}




ssize_t Client::readFromClient(char *tempBuffer, size_t size) 
{
    ssize_t bytes_received = recv(socket_fd, tempBuffer, size, 0);
    if (bytes_received > 0) 
	{
        // Accumuler les données reçues dans le buffer interne
        buffer.append(tempBuffer, bytes_received);
    }
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

// Vérifie si le buffer interne est vide
bool Client::isBufferEmpty() const 
{
    return buffer.empty();
}


