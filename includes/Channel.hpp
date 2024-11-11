/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xav <xav@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:10:39 by xav               #+#    #+#             */
/*   Updated: 2024/11/11 12:52:20 by xav              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "Client.hpp"

class Client;

class Channel 
{
    private:
        std::string name;
        std::map<std::string, int> clients; // Map nickname -> client_fd
        std::map<std::string, int> operators; // Map nickname -> client_fd
		bool _invited;
		std::string _key;
		bool _keyNeeded;
		bool _limit;
		size_t _limitValue;
		std::string _topic;
		bool _topicRight;
		std::string _topicOpe;
		time_t _topicTime;
    public:
        Channel(const std::string &channelName);
        ~Channel();

        std::string getName() const;
        bool addClient(const std::string &nickname, int client_fd);
        void removeClient(const std::string &nickname);
        bool isClientInChannel(const std::string &nickname) const;
        bool isOperator(const std::string &nickname) const;
        void promoteToOperator(const std::string &nickname, int client_fd);
		void broadcastMessage(const std::string &message);
		void broadcastMessage(const std::string &message, int sender_fd);
		void removeOperator(const std::string &nickname);
		bool isEmpty() const;
		int count() const;
		void setInvited(bool invited);
		bool getInvited() const;
		void setKey(std::string key);
		std::string getKey() const;
		bool getKeyNeeded() const;
		void setKeyNeeded(bool keyNeeded);
		size_t getLimitValue() const;
		void setLimitValue(size_t limitValue);
		void setLimit(bool limit);
		bool getLimit() const;
		std::string getTopic() const;
		std::string getTopicOpe() const; 
		time_t getTopicTime() const; 
		bool getTopicRight() const; 
		void setTopic(std::string topic);
		void setTopicOpe(std::string topicOpe); 
		void setTopicTime(time_t topicTime); 
		void setTopicRight(bool topicRight);
		bool hasOperators() const;
		void promoteNextOperator(int client_fd);
				
};

#endif

