/**
*	Created 25.02.2014 by Ihor Dutchak (Youw)
*/

#ifndef BERKELEY_SOCKET_H
#define BERKELEY_SOCKET_H

#ifdef _WIN32 // || _WIN64

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER 
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#endif //_MSC_VER

#elif defined(__linux__) || defined(__APPLE__) //_WIN32 // || _WIN64

#include <sys/types.h>
//POSIX.1-2001 does not require the inclusion of <sys/types.h>, and
//this header file is not required on Linux.  However, some historical
//(BSD) implementations required this header file, and portable
//applications are probably wise to include it.

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET (~0)
#define SOCKET int
#define closesocket(x) close(x)
#define SD_BOTH 2

#endif //_WIN32 // || _WIN64

class Socket {

};

#endif //BERKELEY_SOCKET_H