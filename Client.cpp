/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:45 by xav               #+#    #+#             */
/*   Updated: 2024/11/03 15:47:17 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"

Client::Client(int fd, Server* serverInstance) 
    : socket_fd(fd), authenticated(false), irssi(false), pass_ok(false), nick_ok(false), server(serverInstance) {}

Client::~Client() 
{
    close(socket_fd);
    std::cout << "Client " << socket_fd << " disconnected." << std::endl;
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

ssize_t Client::sendToClient(const std::string& message) 
{
    return send(socket_fd, message.c_str(), message.size(), 0);
}

void Client::handleCommand(const std::string& command)
{
    if (command.find("NICK") == 0) 
    {
        std::string new_nickname = command.substr(5);  // Récupérer le nickname après "NICK "
        setNickname(new_nickname);
        std::cout << "Client " << socket_fd << " a défini son nickname à : " << new_nickname << std::endl;
        sendToClient("Nickname défini à " + new_nickname + "\n");
    }
    else if (command.find("QUIT") == 0) 
    {
        std::cout << "Client " << socket_fd << " a demandé la déconnexion." << std::endl;
    }
    else if (command.find("JOIN ") == 0) 
    {
        std::string channelName = command.substr(5);
        server->handleJoinCommand(socket_fd, channelName);  // Appel de la méthode existante
    }
    else if (command.find("PING") == 0) 
    {
        // Répondre avec un PONG
        std::string serverToken = command.substr(5);  // Récupérer le token envoyé par le client
        std::string pongResponse = "PONG " + serverToken + "\r\n";
        sendToClient(pongResponse);
        std::cout << "Client " << socket_fd << " a reçu un PING et un PONG a été envoyé." << std::endl;
    }
    else 
    {
        std::cout << "Commande inconnue du client " << socket_fd << ": " << command << std::endl;
        sendToClient("Commande inconnue.\n");
    }
}





