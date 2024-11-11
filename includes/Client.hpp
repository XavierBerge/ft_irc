/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:50 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 15:28:43 by xav              ###   ########.fr       */
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
		//getters
		int getSocketFd() const;
		std::string getNickname() const;;
		std::string getUsername() const;
		std::string getRealname() const;
    	std::string getHostname() const;
		bool isAuthenticated() const;
		bool isIrssi() const;
		bool isPass_Ok() const;
		bool isNick_Ok() const;
		bool isInvisible() const;
		const std::vector<std::string>& getChannels() const;
    	bool isInvitedToChannel(const std::string& channelName) const;
	
		//setters
		void setNickname(const std::string& nick);
		void setUsername(const std::string& user);
		void setRealname(const std::string& name);
		void setInvisible(bool invisible);
		void setHostname(const std::string& host);
		void pass_true();
		void nick_true();
		void irssi_true();
		void authenticate();

		//buffer methods
		ssize_t readFromClient(char *buffer, size_t size);
		void sendToClient(const std::string& message);
		bool isBufferEmpty() const;
		bool hasCompleteLine() const;
		std::string getCompleteLine();

		// channels vectors methods
		void addChannelInvitation(const std::string& channelName);
		void addChannel(const std::string& channelName);
		void removeChannel(const std::string& channelName);
		void clearChannels();
		
	};

#endif

