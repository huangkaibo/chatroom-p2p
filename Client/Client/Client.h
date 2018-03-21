#ifndef CLIENT_H
#define CLIENT_H
#pragma warning(disable:4996)
#include<iostream>
#include<vector>
using std::vector;
#include<thread>
using std::thread;
#include"Node.h"
#include"Common.h"
class Client {
private:
	vector<Node> userList;
	SOCKET serverFD,selfFD;
	sockaddr_in serverAddr,selfAddr;
	void wait_connect();
	void recvMessage(Node client_node);
	void recvUserList();
	int find_by_username(string username);
public:
	Client();
	bool logIN(string name);
	void logOut();
	bool sendMessage(string username, string message);
	string* getUserList();						//返回的是string指针，记得要释放内存
	string getMessage(string username);
};

#endif CLIENT_H
