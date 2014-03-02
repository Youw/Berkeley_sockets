/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	functionality of Berkeley sockets for windows, linux and OS X
*	Details: http://en.wikipedia.org/wiki/Berkeley_sockets
*	You need c++11 compiler or greater to use this source
*/

#ifndef BERKELEY_SOCKET_HPP
#define BERKELEY_SOCKET_HPP

#include <string>
#include <vector>
#include <utility>
#include <stdint.h>
#include <stdexcept>

#ifdef _WIN32 // || _WIN64

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER 
#pragma comment (lib, "Ws2_32.lib")
#endif //_MSC_VER

#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__)) //_WIN32 // || _WIN64

#include <sys/types.h>
//POSIX.1-2001 does not require the inclusion of <sys/types.h>, and
//this header file is not required on Linux.  However, some historical
//(BSD) implementations required this header file, and portable
//applications are probably wise to include it.

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET_ERROR	(-1)
#define INVALID_SOCKET	(~0)
#define SOCKET int
#define closesocket(x) close(x)

//Shutdown receive operations.
#define SD_RECEIVE 0
//Shutdown send operations.
#define SD_SEND 1
//Shutdown both send and receive operations.
#define SD_BOTH 2

#include <arpa/inet.h>

#ifndef _SS_MAXSIZE

#define _SS_MAXSIZE		128
#define _SS_ALIGNSIZE	(sizeof(int64_t))
#define _SS_PAD1SIZE	(_SS_ALIGNSIZE - sizeof(u_char) * 2)
#define _SS_PAD2SIZE	(_SS_MAXSIZE - sizeof(u_char) * 2 - \
  				_SS_PAD1SIZE - _SS_ALIGNSIZE)

typedef struct SOCKADDR_STORAGE {
  short   ss_family;
  char    __ss_pad1[_SS_PAD1SIZE];
  int64_t __ss_align;
  char    __ss_pad2[_SS_PAD2SIZE];
} *PSOCKADDR_STORAGE;
#else
typedef sockaddr_storage SOCKADDR_STORAGE;
#endif

#endif //_WIN32 // || _WIN64

namespace sockets {

	class Noncopyable {
	public:
		Noncopyable() { };
		Noncopyable(const Noncopyable&) = delete;
		Noncopyable& operator=(const Noncopyable&) = delete;
	protected:
		~Noncopyable() {}
	};

	class Nonmovable {
	public:
		Nonmovable() { };
		Nonmovable(Noncopyable&&) = delete;
		Nonmovable& operator=(Noncopyable&&) = delete;
	protected:
		~Nonmovable() {}
	};


#ifdef _WIN32
	static WSADATA wsaData;
	static int wsa_startup_result = -1;
#endif //_WIN32

	//need to be called on Windows
	inline bool InitSockets() {
#ifdef _WIN32
		if (wsa_startup_result != 0) {
			wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		}
		return !wsa_startup_result;
#else //_WIN32
		return true;
#endif
	}

	//need to be called on Windows
	inline bool ReleaseSockets() {
#ifdef _WIN32
		int result = WSACleanup();
		wsa_startup_result = -1;
		return !result;
#else //_WIN32
		return true;
#endif
	}

	enum class SockType {TCPIPv4v6, TCPIPv4 = AF_INET, TCPIPv6 = AF_INET6 };

	class Socket : public Noncopyable /*cannot be copied*/ {

	protected:
		SOCKET m_socket;

	public:

		Socket(SOCKET sock = INVALID_SOCKET) {
			m_socket = sock;
		}

		Socket(int af, int type, int protocol) {
			socket(af, type, protocol);
		}

		Socket(Socket&& sock) {
			m_socket = sock.m_socket;
			sock.m_socket = INVALID_SOCKET;
		}

		Socket& operator=(Socket&& sock) {
			m_socket = sock.m_socket;
			sock.m_socket = INVALID_SOCKET;
			return *this;
		}

		bool socket(int af, int type, int protocol) {
			m_socket = ::socket(af, type, protocol);
			return isInvalid();
		}

		int bind(const sockaddr *addr, socklen_t addr_len) {
			return ::bind(m_socket, addr, addr_len);
		}

		int listen(int backlog = SOMAXCONN) {
			return ::listen(m_socket, backlog);
		}

		int setsockopt(int level, int optname, const char *optval, int optlen) {
			return ::setsockopt(m_socket, level, optname, optval, optlen);
		}

		int getsockopt(int level, int optname, char *optval, socklen_t *optlen) {
			return ::getsockopt(m_socket, level, optname, optval, optlen);
		}

		Socket accept(sockaddr *addr, socklen_t *addr_len) {
			return ::accept(m_socket, addr, addr_len);
		}

		bool connect(sockaddr *addr, socklen_t addr_len) {
			return ::connect(m_socket, addr, addr_len) == 0;
		}

		int sendStr(const std::string& str, int flags = 0) {
			return send(str.c_str(), int(str.size()), flags);
		}

		std::string recvStr(unsigned recv_len, int flags = 0) {
			std::string buf(recv_len, 0);
			int resived = recv(&buf[0], int(recv_len), flags);
			if (resived > 0)
				buf.resize(resived);
			else {
				buf.clear();
			}
			return std::move(buf);
		}

		int sendData(const std::vector<char>& data, int flags = 0) {
			return send(data.data(), int(data.size()), flags);
		}

		std::vector<char> recvData(unsigned recv_len, int flags = 0) {
			std::vector<char> buf(recv_len);
			int resived = recv(&buf[0], int(recv_len), flags);
			if (resived > 0)
				buf.resize(resived);
			else {
				buf.clear();
			}
			return std::move(buf);
		}

		int send(const char *buf, unsigned len, int flags) {
			int result = ::send(m_socket, buf, int(len), flags);
			if (result < 0) closesocket();
			return result;
		}

		int recv(char *buf, unsigned buf_len, int flags) {
			int result = ::recv(m_socket, buf, int(buf_len), flags);
			if (result < 0) closesocket();
			return result;
		}

		int getsockname(sockaddr *addr, socklen_t *addr_len) const {
			return ::getsockname(m_socket, addr, addr_len);
		}

		int getpeername(sockaddr *addr, socklen_t *addr_len) const {
			return ::getpeername(m_socket, addr, addr_len);
		}

		unsigned short port() const {
			//port part of sockaddr_in6 (IPv6) is same as in sockaddr_in (IPv4)
			sockaddr_in6 addr;
			socklen_t addr_size = sizeof addr;
			if (getsockname((sockaddr*)(&addr), &addr_size) == 0) {
				return ntohs(addr.sin6_port);
			}
			return 0;
		}

		unsigned short peerPort() const {
			//port part of sockaddr_in6 (IPv6) is same as in sockaddr_in (IPv4)
			sockaddr_in6 addr;
			socklen_t addr_size = sizeof addr;
			if (getpeername((sockaddr*)(&addr), &addr_size) == 0) {
				return ntohs(addr.sin6_port);
			}
			return 0;
		}

		std::string ip() const {
			SOCKADDR_STORAGE addr;
			socklen_t addr_size = sizeof addr;
			if (getsockname((sockaddr*)(&addr), &addr_size) == 0) {
				if (addr.ss_family == AF_INET) {
					char dst[INET_ADDRSTRLEN];
					const char* ip_char = inet_ntop(addr.ss_family, addr.__ss_pad1 + sizeof(unsigned short), dst, INET_ADDRSTRLEN);
					return ip_char ? ip_char : "";
				}
				if (addr.ss_family == AF_INET6) {
					char dst[INET6_ADDRSTRLEN];
					const char * ip_char = inet_ntop(addr.ss_family, addr.__ss_pad1 + sizeof(unsigned short)+sizeof(uint32_t), dst, INET6_ADDRSTRLEN);
					return ip_char ? ip_char : "";
				}
			}
			return "";
		}

		std::string peerIp() const {
			SOCKADDR_STORAGE addr;
			socklen_t addr_size = sizeof addr;
			if (getpeername((sockaddr*)(&addr), &addr_size) == 0) {
				if (addr.ss_family == AF_INET) {
					char dst[INET_ADDRSTRLEN];
					const char* ip_char = inet_ntop(addr.ss_family, addr.__ss_pad1 + sizeof(unsigned short), dst, INET_ADDRSTRLEN);
					return ip_char ? ip_char : "";
				}
				if (addr.ss_family == AF_INET6) {
					char dst[INET6_ADDRSTRLEN];
					const char * ip_char = inet_ntop(addr.ss_family, addr.__ss_pad1 + sizeof(unsigned short)+sizeof(uint32_t), dst, INET6_ADDRSTRLEN);
					return ip_char ? ip_char : "";
				}
			}
			return "";
		}

		int shutdown(int how = SD_BOTH) {
			return ::shutdown(m_socket, how);
		}

		int closesocket() {
			shutdown();
			int result = ::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return result;
		}

		SOCKET detach() {
			SOCKET result = m_socket;
			m_socket = INVALID_SOCKET;
			return result;
		}

		~Socket() {
			if (!isInvalid()) {
				throw std::logic_error("Socket destroed before closed");
			}
		}

		bool isInvalid() const {
			return m_socket == INVALID_SOCKET;
		}

		//should be comfortable, right?
		operator SOCKET () const { return m_socket; }

	};

};//namespace sockets

#endif //BERKELEY_SOCKET_HPP
