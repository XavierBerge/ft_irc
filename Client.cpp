/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:45 by xav               #+#    #+#             */
/*   Updated: 2024/11/04 22:34:10 by xav              ###   ########.fr       */
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

ssize_t Client::readFromClient(char *buffer, size_t size) 
{
    return recv(socket_fd, buffer, size, 0);
}

void Client::sendToClient(const std::string& message) 
{
    send(socket_fd, message.c_str(), message.size(), 0);
	std::cout << "Server send to " << nickname << " :\n" << message;
}
/*
void Client::handleModeCommand(const std::string& command) 
{
    std::istringstream iss(command);
    std::string commandName, channelName, mode, target;
    iss >> commandName >> channelName >> mode >> target;

    // Vérification si le channel existe dans le serveur
    Channel* channel = server->getChannelByName(channelName); // Assurez-vous que cette méthode est bien définie
    if (!channel) 
	{
        // RPL_NOSUCHCHANNEL : 403
        sendToClient("403 " + getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    if (mode == "+o") 
	{
        int targetFd = server->getClientFdByNickname(target);
        if (targetFd == -1) 
		{
            // ERR_NOSUCHNICK : 401
            sendToClient("401 " + getNickname() + " " + target + " :No such nick/channel\r\n");
            return;
        }

        // Vérifier si l'utilisateur actuel est opérateur
        if (!channel->isOperator(socket_fd)) 
		{
            // ERR_CHANOPRIVSNEEDED : 482
            sendToClient("482 " + getNickname() + " " + channelName + " :You're not channel operator\r\n");
            return;
        }

        // Promouvoir l'utilisateur en opérateur
        channel->promoteToOperator(targetFd, socket_fd);
    } 
	else 
        sendToClient("472 " + getNickname() + " " + mode + " :is an unknown mode character\r\n");
}
*/







