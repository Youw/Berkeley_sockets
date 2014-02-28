/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	simple TCP IPv4 server (listening) socket for windows, linux and OS X
*	You need c++11 compiler or greater to use this source
*/

#include "ServerTCPIP.hpp"

namespace sockets {
	ClientID_t SocketServerTCPIP::ConnectedClient::ID_inc = 0;

	int SocketServerTCPIP::clientListener(SocketServerTCPIP* parent) {
		while (parent->isRunning()) {
			Socket client_socket = parent->listen_socket.accept(nullptr, nullptr);
			if (client_socket.isInvalid()) {
				if (parent->isRunning()) {
					parent->listen_socket.closesocket();
				}
			}
			else {
				if (parent->cl_conn_func(client_socket))
					parent->AddClient(client_socket);
				else
					client_socket.detach();
			}
		}
		return 0;
	}

	int  SocketServerTCPIP::clientProcessor(SocketServerTCPIP* parent, ClientRecvLoopFunc& clientProcessor, ConnectedClient* client) {
		while (!client->cl_socket.isInvalid()) {
			char tmp;
			if (0 == client->cl_socket.recv(&tmp, 1, MSG_PEEK)) {
				if (parent->cl_shut_func(client->cl_socket))
					client->cl_socket.detach();
				return 0;
			}
			if (!client->cl_socket.isInvalid()) {
				if (!clientProcessor(client->cl_socket)) {
					client->cl_socket.detach();
					return 0;
				}
			}
			else {
				parent->RemoveClient(client->clientID());
				return 0;
			}
		}
		return 0;
	}

	void SocketServerTCPIP::AddClient(Socket& client) {
		lock_guard<mutex> lck(clients_mutex);
		if (isRunning()) {
			{
				ConnectedClient c_cl;
				c_cl.cl_socket = move(client);
				clients.push_back(move(c_cl));
			}
			ConnectedClient& c_cl = clients.back();
			c_cl.cl_thread = thread(clientProcessor, this, cl_loop_func, &c_cl);
		}
	}

	bool SocketServerTCPIP::RemoveClient(ClientID_t cl_id) {
		lock_guard<mutex> lck(clients_mutex);
		if (isRunning()) {
			for (auto i = clients.begin(); i != clients.end(); i++) {
				if (i->clientID() == cl_id) {
					i->cl_thread.detach();
					clients.erase(i);
					return true;
				}
			}
		}
		return false;
	}

	SocketServerTCPIP::SocketServerTCPIP(SockType serv_type, const ClientConnectedFunc& client_conn, const ClientRecvLoopFunc& client_recv_loop, const ClientRecvLoopFunc& client_shutdown, unsigned short port, const string& bind_addr) : SocketServerTCPIP() {
		startServer(serv_type, client_conn, client_recv_loop, client_shutdown, port, bind_addr);
	}

	bool SocketServerTCPIP::startServer(SockType serv_type, const ClientConnectedFunc& client_conn, const ClientRecvLoopFunc& client_recv_loop, const ClientRecvLoopFunc& client_shutdown, unsigned short port, const string& bind_addr) {

		if (isRunning()) return false;
		struct addrinfo *result = NULL;
		int af_fmly;
		switch (serv_type) {
		case SockType::TCPIPv4:
			af_fmly = AF_INET;
			break;
		case SockType::TCPIPv6:
			af_fmly = AF_INET6;
			break;
		}
		struct addrinfo hints{ AI_PASSIVE | AI_ALL | AI_V4MAPPED, af_fmly, SOCK_STREAM, IPPROTO_TCP };
		int iResult;
		if (bind_addr.empty())
			iResult = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result);
		else
			iResult = getaddrinfo(bind_addr.c_str(), std::to_string(port).c_str(), &hints, &result);
		if (iResult != 0) {
			return false;
		}

		iResult = listen_socket.socket(result->ai_family, result->ai_socktype, result->ai_protocol);
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
		is_running = true;

		freeaddrinfo(result);
		
		cl_conn_func = client_conn;
		cl_loop_func = client_recv_loop;
		cl_shut_func = client_shutdown;

		server_thread = thread(clientListener, this);
		return true;
	}

	void SocketServerTCPIP::stopServer() {
		lock_guard<mutex> lck(clients_mutex);
		is_running = false;
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
	}

	void SocketServerTCPIP::sendToAll(const char* data, unsigned data_len, int flags) {
		lock_guard<mutex> lck(clients_mutex);
		for (auto& i : clients) {
			i.cl_socket.send(data, data_len, flags);
		}
	}

	void SocketServerTCPIP::sendToAllStr(const string& str, int flags) {
		lock_guard<mutex> lck(clients_mutex);
		for (auto& i : clients) {
			i.cl_socket.sendStr(str, flags);
		}
	}

	void SocketServerTCPIP::sendToAllData(const vector<char>& data, int flags) {
		lock_guard<mutex> lck(clients_mutex);
		for (auto& i : clients) {
			i.cl_socket.sendData(data, flags);
		}
	}

	SocketServerTCPIP::~SocketServerTCPIP() {
		stopServer();
	}
};