#ifndef SERVER_H
#define SERVER_H
#pragma warning(disable:4996)
#include<iostream>
//#include<winsock2.h>
#include<vector>
using std::vector;
#include<thread>
using std::thread;
#include"Node.h"
#include"Common.h"

class Server {
private:
	vector<Node> userList;
	SOCKET serverFD;
	sockaddr_in serverAddr;

	Node addToUserList(SOCKET clientFD, sockaddr_in clientAddr);
	void distriubuteUserList();
	void logIn();
	void logOut(Node client_node);
public:
	Server(int a);
	void start();
};

#endif
