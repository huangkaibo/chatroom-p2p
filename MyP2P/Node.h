#ifndef NODE_H
#define NODE_H
#define username_len 40
#include<string>
#include<winsock2.h>
#pragma comment(lib, "ws2_32.lib")
using std::string;
struct Node {
	char ip[20];
	unsigned short port;
	char username[40];
	SOCKET socketFD;
	string message;
};

#endif