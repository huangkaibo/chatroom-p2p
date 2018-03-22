#include "Client.h"

//等待其他Client连接
void Client::wait_connect() {
	if (listen(selfFD, 10) != 0) {
		std::cout << "listen failed!'\n";
		return;
	}
	while (true) {
		sockaddr_in new_addr;
		int sin_size = sizeof(new_addr);
		SOCKET new_FD = accept(selfFD, (sockaddr *)&new_addr, &sin_size);
		if (new_FD == INVALID_SOCKET) {
			std::cout << "accept failed!\n";
			continue;
		}
		std::cout << "accept success\n";

		char username[username_len];
		int len=recv(new_FD, username, username_len, 0);
		username[len] = 0x00;									//在文本末尾加上结束符
		int i;
		for (i = 0; i < userList.size(); i++) {
			if (strcmp(username,userList[i].username)==0) {				//根据username寻找对应的Node
				userList[i].socketFD = new_FD;
				break;
			}
		}
		char confirm[1] = "";
		send(userList[i].socketFD, (char*)&confirm, sizeof(char), 0);			//回发确认，解决TCP的粘包问题

		thread receive_message_th(&Client::recvMessage, this, userList[i]);		//开启线程监听该Client发送过来的消息
		receive_message_th.detach();
	}
}

//接收其他Client消息
void Client::recvMessage(Node &client_node) {					//要用 引用参数，否则是复制品
	char buffer[1024];
	int len;
	while (true) {
		if ((len = recv(client_node.socketFD, buffer, 1024, 0)) != -1) {
			buffer[len] = 0x00;
			client_node.message = +buffer;
			std::cout << client_node.username << " say: " << client_node.message;
		}
	}
}

//从中心服务器接收userList数据
void Client::recvUserList() {
	int node_num;
	while (true) {
		recv(serverFD, (char*)&node_num, sizeof(int), 0);				//接收node个数
		Node *buffer = (Node*)malloc(node_num * sizeof(Node));			//根据个数申请buffer
		recv(serverFD, (char*)buffer, node_num * sizeof(Node), 0);		//接收node信息
		for (int i = 0; i < userList.size(); i++) {
			bool flag = false;
			for (int ii = 0; ii < node_num; ii++) {
				if (Common::compare_node(userList[i], buffer[ii])) {
					flag = true;
					break;
				}
			}
			if (!flag) {					//新的userList中不含此Node，所以将其删除
				userList.erase(userList.begin() + i);
			}
		}
		for (int i = 0; i < node_num; i++) {
			bool flag = false;
			for (int ii = 0; ii < userList.size(); ii++) {
				if (Common::compare_node(buffer[i], userList[ii])) {
					flag = true;
					break;
				}
			}
			if (!flag) {					//新的userList中含有此新Node，所以将其添加
				buffer[i].socketFD = NULL;
				buffer[i].message = "";
				userList.push_back(buffer[i]);
			}
		}
		free(buffer);
		std::cout << "\n";
		for (int i = 0; i < userList.size(); i++) {
			std::cout << userList[i].ip << " " << userList[i].port << " " << userList[i].username << "\n";
		}
		std::cout << "\n";
	}
}

//根据username寻找Node的偏移值
int Client::find_by_username(string username) {
	for (int i = 0; i < userList.size(); i++) {
		if (strcmp(username.c_str(),userList[i].username)==0) {
			return i;
		}
	}
	return -1;
}

//构造函数
Client::Client() {
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		std::cout << "sever winsock init Failed!\n";
		return;
	}
	//连接中心服务器的socket
	serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Common::getServer_Node().port);
	serverAddr.sin_addr.s_addr = inet_addr(Common::getServer_Node().ip);
	//自身服务器的socket
	selfFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	self_port = Common::getAvailablePort();
	selfAddr.sin_family = AF_INET;
	selfAddr.sin_port = htons(self_port);
	selfAddr.sin_addr.s_addr = INADDR_ANY;
}

//登录，连接中心服务器
bool Client::logIn(string name) {
	if (connect(serverFD, (sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {		//连接中心服务器
		std::cout << "server connect fail\n";
		return false;
	}
	if (bind(selfFD, (sockaddr*)&selfAddr, sizeof(sockaddr)) == SOCKET_ERROR) {		//自身服务器绑定端口号
		std::cout << "self bind failed! " << "\n";
		return false;
	}
	strcpy(selfname, name.c_str());
	send(serverFD, (char*)&self_port, sizeof(unsigned short), 0);				//发布自身服务器端口号
	send(serverFD, selfname, strlen(selfname), 0);								//发布username

	thread recv_userList_th(&Client::recvUserList, this);						//开启线程监听中心服务器发来的userList数据
	recv_userList_th.detach();

	thread wait_connect_th(&Client::wait_connect, this);						//开启线程监听其他Client的连接
	wait_connect_th.detach();
	return true;
}

//下线
void Client::logOut() {
	char message[] = "0.0LOGOUT0.0";
	send(serverFD, message, strlen(message), 0);								//还应该关闭各种socket，线程，此处未做
}

//向特定Client发送消息
bool Client::sendMessage(string username, string message) {
	int user_i;
	if (((user_i = find_by_username(username)) == -1)) return false;			//根据username找到对应的Node
	if (userList[user_i].socketFD == NULL) {									//若未建立连接
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(sockVersion, &wsaData) != 0) {
			std::cout << "client winsock init Failed!\n";
			return false;
		}
		SOCKET newFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in new_addr;
		new_addr.sin_family = AF_INET;
		new_addr.sin_port = htons(userList[user_i].port);
		new_addr.sin_addr.s_addr = inet_addr(userList[user_i].ip);
		if (connect(newFD, (sockaddr*)&new_addr, sizeof(serverAddr)) != 0) {
			std::cout << "client connect  Failed!\n";
			return false;
		}
		std::cout << "client connect  Success!\n";
		send(newFD, selfname, strlen(selfname), 0);					//发送自己的用户名，否则对方无从知道是谁的连接

		char confirm[1];
		recv(newFD, confirm, sizeof(char), 0);						//阻塞等待对方确认，解决TCP粘包问题

		userList[user_i].socketFD = newFD;
		thread receive_message_th(&Client::recvMessage, this, userList[user_i]);			//建立监听线程接收该Client信息
		receive_message_th.detach();
	}
	int send_size = message.size();
	char *buffer = (char*)malloc(send_size*sizeof(char)+1);
	strcpy(buffer, message.c_str());
	if (send(userList[user_i].socketFD, buffer, send_size, 0) == -1) {
		std::cout << "client send  Failed!\n";
		return false;
	}
	free(buffer);
	return true;
}

//获取用所有Node的username
string* Client::getUserList() {
	int size = userList.size();
	string *result = new string[size];
	for (int i = 0; i < size; i++) {
		result[i] = userList[i].username;
	}
	return result;
}

//获取特定用户发来的消息
string Client::getMessage(string username) {
	int user_i;
	if ((user_i = find_by_username(username) == -1)) return NULL;
	return userList[user_i].message;
}
