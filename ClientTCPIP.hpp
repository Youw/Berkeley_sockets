/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	simple TCP IPv4 server (listening) socket for windows, linux and OS X
*	You need c++11 compiler or greater to use this source
*/

#ifndef CLIENT_TCP_IP_V4_HPP
#define CLIENT_TCP_IP_V4_HPP

#include <Socket\Socket.hpp>

#include <vector>
#include <string>
#include <utility>

namespace sockets {
	using std::string;
	using std::vector;
	using std::move;

	class SocketServerTCPIP final: public Noncopyable, public Nonmovable {

		
	};
};

#endif //CLIENT_TCP_IP_V4_HPP