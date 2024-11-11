/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:50 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 12:34:06 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <vector>
#include <algorithm>
#include "Server.hpp"
#include "Channel.hpp"

class Server;

class Channel;

class Client 
{
	private:
		int socket_fd;
		std::string nickname;
		std::string username;
		std::string realname;
		std::string hostname;
		bool authenticated;
		bool irssi;
		bool pass_ok;
		bool nick_ok;
		bool invisible;
		std::string buffer;
		std::vector<std::string> channelInvitations;
		std::vector<std::string> client_channels;


	public:
		Client(int fd);
		~Client();

		int getSocketFd() const;
		void setNickname(const std::string& nick);
		std::string getNickname() const;;

		void setUsername(const std::string& user);
		std::string getUsername() const;

		void setRealname(const std::string& name);
		std::string getRealname() const;

		void setHostname(const std::string& host);
    	std::string getHostname() const;

		bool isAuthenticated() const;
		void authenticate();
		void pass_true();
		void irssi_true();
		void nick_true();

		bool isIrssi() const;
		bool isPass_Ok() const;
		bool isNick_Ok() const;
		bool isInvisible() const;
		void setInvisible(bool invisible);

		ssize_t readFromClient(char *buffer, size_t size);
		void sendToClient(const std::string& message);

		void handleCommand(const std::string& command);
		void handleModeCommand(const std::string& command);

		bool isBufferEmpty() const;
		std::string getCompleteLine();
		bool hasCompleteLine() const;
		void addChannelInvitation(const std::string& channelName);
    	bool isInvitedToChannel(const std::string& channelName) const;
		void addChannel(const std::string& channelName);
		void removeChannel(const std::string& channelName);
		void clearChannels();
		const std::vector<std::string>& getChannels() const;
		
	};

#endif

