/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	simple TCP IPv4 server (listening) socket for windows, linux and OS X
*	You need c++11 compiler or greater to use this source
*/

#include "ServerTCPIPv4.hpp"

namespace sockets {
	ClientID_t SocketServerTCPIPv4::ConnectedClient::ID_inc = 0;

	int SocketServerTCPIPv4::clientListener(SocketServerTCPIPv4* parent) {
		while (parent->isRunning()) {
			Socket client_socket = parent->listen_socket.accept(nullptr, nullptr);
			if (client_socket.isInvalid()) {
				if (parent->isRunning()) {
					parent->listen_socket.closesocket();
				}
			}
			else {
				parent->AddClient(client_socket);
			}
		}
		return 0;
	}

	int  SocketServerTCPIPv4::clientProcessor(SocketServerTCPIPv4* parent, ClientProcessorFunc& clientProcessor, ConnectedClient* client) {
		while (!client->cl_socket.isInvalid()) {
			char tmp;
			client->cl_socket.recv(&tmp, 1, MSG_PEEK);
			if (!client->cl_socket.isInvalid())
				clientProcessor(client->cl_socket);
			else {
				parent->RemoveClient(client->clientID());
				return 0;
			}
		}
		return 0;
	}

	void SocketServerTCPIPv4::AddClient(Socket& client) {
		unique_lock<mutex> lck(clients_mutex);
		if (isRunning()) {
			{
				ConnectedClient c_cl;
				c_cl.cl_socket = move(client);
				clients.push_back(move(c_cl));
			}
			ConnectedClient& c_cl = *clients.rbegin();
			c_cl.cl_thread = thread(clientProcessor, this, cl_processor, &c_cl);
		}
	}

	bool SocketServerTCPIPv4::RemoveClient(ClientID_t cl_id) {
		unique_lock<mutex> lck(clients_mutex);
		if (isRunning()) {
			//fell to sleep
		}
		return true;
	}

	SocketServerTCPIPv4::SocketServerTCPIPv4(const ClientProcessorFunc& client_processor, unsigned short port, const string& bind_addr) : SocketServerTCPIPv4() {
		startServer(client_processor, port, bind_addr);
		ConnectedClient c_cl;
	}

	bool SocketServerTCPIPv4::startServer(const ClientProcessorFunc& client_processor, unsigned short port, const string& bind_addr) {
		if (isRunning()) return false;
		struct addrinfo *result = NULL;
		struct addrinfo hints{ AI_PASSIVE, AF_INET, SOCK_STREAM, IPPROTO_TCP };
		int iResult;
		if (bind_addr.empty())
			iResult = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result);
		else
			iResult = getaddrinfo(bind_addr.c_str(), std::to_string(port).c_str(), &hints, &result);
		if (iResult != 0) {
			return false;
		}

		iResult = listen_socket.init(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (iResult != 0) {
			return false;
		}
		iResult = listen_socket.bind(result->ai_addr, (int)result->ai_addrlen);
		if (iResult != 0) {
			return false;
		}
		iResult = listen_socket.listen();
		if (iResult != 0) {
			return false;
		}
		freeaddrinfo(result);
		
		cl_processor = client_processor;
		is_running = true;
		server_thread = thread(clientListener, this);
		return true;
	}

	void SocketServerTCPIPv4::stopServer() {
		unique_lock<mutex> lck(clients_mutex);
		listen_socket.closesocket();
		if (server_thread.joinable()) {
			server_thread.join();
		}
		for (auto& i : clients) {
			i.cl_socket.closesocket();
			if (i.cl_thread.joinable()) {
				i.cl_thread.join();
			}
		}
		is_running = false;
	}

	void SocketServerTCPIPv4::sendToAll(const char* data, unsigned data_len, int flags) {
		unique_lock<mutex> lck(clients_mutex);
		for (auto& i : clients) {
			i.cl_socket.send(data, data_len, flags);
		}
	}

	void SocketServerTCPIPv4::sendToAllStr(const string& str, int flags) {
		unique_lock<mutex> lck(clients_mutex);
		for (auto& i : clients) {
			i.cl_socket.sendStr(str, flags);
		}
	}

	void SocketServerTCPIPv4::sendToAllData(const vector<char>& data, int flags) {
		unique_lock<mutex> lck(clients_mutex);
		for (auto& i : clients) {
			i.cl_socket.sendData(data, flags);
		}
	}

	SocketServerTCPIPv4::~SocketServerTCPIPv4() {
		stopServer();
	}
};