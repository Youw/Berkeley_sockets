/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	simple TCP IPv4 server (listening) socket for windows, linux and OS X
*	You need c++11 compiler or greater to use this source
*/

#ifndef SERVER_TCP_IP_V4_HPP
#define SERVER_TCP_IP_V4_HPP

#include <Socket\Socket.hpp>

#include <functional>
#include <list>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <utility>

namespace sockets {
	using std::thread;
	using std::vector;
	using std::mutex;
	using std::lock_guard;
	using std::list;
	using std::string;
	using std::function;
	using std::move;

#define ClientID_t unsigned long long
#define ClientRecvLoopFunc function<bool(Socket&)>
#define ClientConnectedFunc function<bool(Socket&)>
#define ClientShutDownFunc function<bool(Socket&)>

	class SocketServerTCPIP final: public Noncopyable {
		
		struct ConnectedClient : public Noncopyable {
			ConnectedClient() : client_ID(++ID_inc) {}

			ConnectedClient(ConnectedClient&& right) : 
				cl_socket(move(right.cl_socket)), cl_thread(move(right.cl_thread)), client_ID(move(right.client_ID)){
				right.client_ID = 0;
			}

			ConnectedClient& operator=(ConnectedClient&& right) {
				cl_socket = move(right.cl_socket);
				cl_thread = move(right.cl_thread);
				client_ID = move(right.client_ID);
				right.client_ID = 0;
			}

			Socket cl_socket;
			thread cl_thread;
			ClientID_t clientID() const { return client_ID; }
		private:
			ClientID_t client_ID;
			static ClientID_t ID_inc;
		};

		bool is_running;
		Socket listen_socket;
		thread server_thread;

		mutex clients_mutex;
		list<ConnectedClient> clients;
		ClientRecvLoopFunc cl_loop_func;
		ClientConnectedFunc cl_conn_func;
		ClientShutDownFunc cl_shut_func;

		int static clientListener(SocketServerTCPIP* parent);
		int static clientProcessor(SocketServerTCPIP* parent, ConnectedClient* client);
	
		void AddClient(Socket& client);
		bool RemoveClient(ClientID_t cl_id);

	public:
		
		SocketServerTCPIP() : is_running(false) {}

		SocketServerTCPIP(SockType serv_type, const ClientConnectedFunc& client_conn, const ClientRecvLoopFunc& client_recv_loop, const ClientRecvLoopFunc& client_shutdown, unsigned short port, const string& bind_addr = "");

		bool startServer(SockType serv_type, const ClientConnectedFunc& client_conn, const ClientRecvLoopFunc& client_recv_loop, const ClientRecvLoopFunc& client_shutdown, unsigned short port, const string& bind_addr = "");

		void stopServer();

		void SocketServerTCPIP::sendToAll(const char* data, unsigned data_len, int flags = 0);

		void SocketServerTCPIP::sendToAllStr(const string& str, int flags = 0);

		void SocketServerTCPIP::sendToAllData(const vector<char>& data, int flags = 0);

		~SocketServerTCPIP();

		bool isRunning() const { return is_running; }
	};
};

#endif //SERVER_TCP_IP_V4_HPP