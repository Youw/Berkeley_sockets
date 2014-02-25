/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*	functionality of Berkeley sockets for windows, linux and OS X
*	Details: http://en.wikipedia.org/wiki/Berkeley_sockets
*	You need c++11 compiler or greater to use this source
*/

#ifndef BERKELEY_SOCKET_H
#define BERKELEY_SOCKET_H

#include <string>
#include <vector>
#include <utility>

#ifdef _WIN32 // || _WIN64

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER 
#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
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

#endif //_WIN32 // || _WIN64

namespace noncopyable {
	
	class Noncopyable {
	public:
		Noncopyable() { };
		Noncopyable(const Noncopyable&) = delete;
		Noncopyable& operator=(const Noncopyable&) = delete;
	};

};

#ifdef _WIN32
static WSADATA wsaData;
static int wsa_startup_result = -1;
#endif //_WIN32

//need to be called on Windows
bool 	InitSockets() {
#ifdef _WIN32
	if (wsa_startup_result != 0) {
		wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	return !wsa_startup_result;
#else //_WIN32
	return true;
#endif
}
class Socket : public noncopyable::Noncopyable /*cannot be copied*/ {

protected:
	SOCKET m_socket;

public:

	Socket(SOCKET sock = INVALID_SOCKET) {
		m_socket = sock;
	}

	Socket(int af, int type, int protocol) {
		init(af, type, protocol);
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

	bool init(int af, int type, int protocol) {
		m_socket = socket(af, type, protocol);
		return isInvalid();
	}

	int bind(const sockaddr *addr, socklen_t addr_len) {
		return ::bind(m_socket, addr, addr_len);
	}

	int listen(int backlog = SOMAXCONN) {
		return ::listen(m_socket, backlog);
	}

	Socket accept(sockaddr *addr, socklen_t *addr_len) {
		return std::move(Socket(::accept(m_socket, addr, addr_len)));
	}

	bool connect(sockaddr *addr, int addr_len) {
		return ::connect(m_socket, addr, addr_len)==0;
	}

	int sendStr(const std::string& str, int flags = 0) {
		return send(str.c_str(), (int)str.size(), flags);
	}

	std::string recvStr(int recv_len, int flags = 0) {
		std::string buf(recv_len, 0);
		int resived = recv(&buf[0], recv_len, flags);
		if (resived>0)
			buf.resize(resived);
		else {
			buf.clear();
			closesocket();
		}
		return std::move(buf);
	}

	int sendData(const std::vector<char>& data, int flags = 0) {
		return send(data.data(), (int)data.size(), flags);
	}

	std::vector<char> recvData(int recv_len, int flags = 0) {
		std::vector<char> buf(recv_len);
		int resived = recv(&buf[0], recv_len, flags);
		if (resived>0)
			buf.resize(resived);
		else {
			buf.clear();
			closesocket();
		}
		return std::move(buf);
	}

	int send(const char *buf, int len, int flags) {
		return ::send(m_socket, buf, len, flags);
	}

	int recv(char *buf, int len, int flags) {
		return ::recv(m_socket, buf, len, flags);
	}

	int getsockname(sockaddr *addr, socklen_t *addr_len) {
		return ::getsockname(m_socket, addr, addr_len);
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

	~Socket() {
		closesocket();
	}

	bool isInvalid() const {
		return m_socket == INVALID_SOCKET;
	}

	//should be comfortable, right?
	operator SOCKET () const { return m_socket; }
};

#endif //BERKELEY_SOCKET_H