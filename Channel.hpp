/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:39 by xav               #+#    #+#             */
/*   Updated: 2024/11/04 15:45:23 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "Client.hpp"
#include "Server.hpp"

class Client;
class Server;

class Channel 
{
	private:
		std::string name;
		std::set<int> clients;
		std::map<int, bool> operators;
	

	public:
		Channel(const std::string &channelName);
		~Channel();

		std::string getName() const;
		bool addClient(int client_fd);
		void removeClient(int client_fd);
		bool isClientInChannel(int client_fd) const;
		bool isOperator(int client_fd) const;
		void promoteToOperator(int client_fd);
		void broadcastMessage(const std::string &message, int sender_fd);
};

#endif
