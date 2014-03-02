/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	simple TCP IPv4 server (listening) socket for windows, linux and OS X
*	You need c++11 compiler or greater to use this source
*/

#ifndef CLIENT_TCP_IP_V4_HPP
#define CLIENT_TCP_IP_V4_HPP

#include <Socket/Socket.hpp>

#include <vector>
#include <string>
#include <utility>

namespace sockets {
	using std::string;
	using std::vector;
	using std::move;

	class ClientTCPIP final: public Socket {
		
		//hidden due to logic compatibility
		int listen(int backlog) { return 0; }

		//hidden due to logic compatibility
		Socket accept(sockaddr *addr, socklen_t *addr_len) { return 0; }

	public:

		ClientTCPIP() {}

		ClientTCPIP(const string name, unsigned short port, SockType conn_type = SockType::TCPIPv4v6, unsigned short bind_port = 0) {
			connect(name, port, conn_type, bind_port);
		}

		bool connect(const string name, unsigned short port, SockType conn_type = SockType::TCPIPv4v6, unsigned short bind_port = 0) {
			if (0!=Socket::peerPort()) return false;
			if (!isInvalid()) closesocket();

			addrinfo *result = NULL;
			addrinfo hints{ bind_port?AI_PASSIVE:0, int(conn_type), SOCK_STREAM, IPPROTO_TCP };


			int iResult = getaddrinfo(name.c_str(), std::to_string(port).c_str(), &hints, &result);
			if (iResult != 0) {
				return 0;
			}

			for (auto ptr = result; ptr != NULL; ptr = ptr->ai_next) {

				// Create a SOCKET for connecting to server
				Socket::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (isInvalid()) {
					continue;
				}

				if (bind_port) {
					sockaddr_in bind_addr{};
					bind_addr.sin_family = decltype(bind_addr.sin_family)(ptr->ai_family);
					bind_addr.sin_port = htons(bind_port);
					int iSetOption = 1;
//					if (0 != setsockopt( SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption))) {
//						closesocket();
//						continue;
//					}
					if (0 != bind((sockaddr*)&bind_addr, sizeof(bind_addr))) {
						closesocket();
						continue;
					}
				}
				// Connect to server.
				if (!Socket::connect(ptr->ai_addr, (int)ptr->ai_addrlen)) {
					closesocket();
					continue;
				}
				break;
			}

			if (Socket::isInvalid()) {
				return false;
			}
			return true;
		}

		
	};
};

#endif //CLIENT_TCP_IP_V4_HPP
