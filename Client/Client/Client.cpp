#include "Client.h"

void Client::wait_connect(){
	if (listen(selfFD, 10) != 0) {
		std::cout << "listen failed!'\n";
		return;
	}
	while (true) {
		sockaddr_in new_addr;
		int sin_size = sizeof(new_addr);
		SOCKET new_FD = accept(serverFD, (sockaddr *)&new_addr, &sin_size);
		if (new_FD == INVALID_SOCKET) {
			std::cout << "accept failed!\n";
			continue;
		}
		std::cout << "accept success\n";
		char new_ip[20];
		unsigned short new_port;
		strcpy(new_ip, inet_ntoa(new_addr.sin_addr));
		new_port = ntohs(new_addr.sin_port);
		int i;
		for (i = 0; i < userList.size(); i++) {
			if (userList[i].port == new_port && userList[i].ip == new_ip) {
				userList[i].socketFD = new_FD;
				break;
			}
		}
		thread receive_message_th(&Client::recvMessage, this,userList[i]);
		//logOut(client_node);

		/*while (true);*/
		receive_message_th.detach();
	}
}

void Client::recvMessage(Node client_node){
	char buffer[1024];
	int len;
	while (true) {
		if ((len=recv(client_node.socketFD, buffer, 1024, 0)) != -1) {
			buffer[len] = 0x00;
			client_node.message = +buffer;
		}
	}
}

void Client::recvUserList(){
	int node_num;
	recv(serverFD, (char*)&node_num, sizeof(int), 0);				//接收node数量
	Node *buffer = (Node*)malloc(node_num * sizeof(Node));							//为毛用new时 delete会报错？？？？？
	recv(serverFD, (char*)buffer, node_num * sizeof(Node), 0);		//接收node数组
	for (int i = 0; i < userList.size(); i++) {
		bool flag = false;
		for (int ii = 0; ii < node_num; ii++) {
			if (Common::compare_node(userList[i], buffer[ii])) {
				flag = true;
				break;
			}
		}
		if (!flag) {					//userList中多余此Node，所以删除userList[i]
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
		if (!flag) {					//userList中不含此新Node，所以添加buffer[i]
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

int Client::find_by_username(string username){
	for (int i = 0; i < userList.size(); i++) {
		if (userList[i].username == username) {
			return i;
		}
	}
	return -1;
}

Client::Client() {
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		std::cout << "sever winsock init Failed!\n";
		return;
	}
	//中心服务器的socket配置
	serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Common::getServer_Node().port);
	serverAddr.sin_addr.s_addr = inet_addr(Common::getServer_Node().ip);
	//自身服务器的socket配置
	selfFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	selfAddr.sin_family = AF_INET;
	selfAddr.sin_port = htons(Common::getAvailablePort());
	selfAddr.sin_addr.s_addr = INADDR_ANY;
}

bool Client::logIN(string name){
	if (connect(serverFD, (sockaddr *)&serverAddr, sizeof(serverAddr))!=0) {
		std::cout << "server connect fail\n";
		return false;
	}

	char username[username_len];
	strcpy(username, name.c_str());
	send(serverFD, username, strlen(username), 0);
	thread recv_userList_th = thread(&Client::recvUserList, this);
	recv_userList_th.detach();
	return true;
}

void Client::logOut(){
	char message[] = "0.0用户下线0.0";
	send(serverFD, message, strlen(message), 0);
}

bool Client::sendMessage(string username, string message){
	int user_i;
	if ((user_i = find_by_username(username) == -1)) return false;
	Node node = userList[user_i];
	if (node.socketFD == NULL) {
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(sockVersion, &wsaData) != 0) {
			std::cout << "client winsock init Failed!\n";
			return false;
		}
		SOCKET newFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in new_addr;
		new_addr.sin_family = AF_INET;
		new_addr.sin_port = htons(node.port);
		new_addr.sin_addr.s_addr = inet_addr(node.ip);
		if (connect(newFD, (sockaddr*)&new_addr, sizeof(serverAddr)) != 0) {
			std::cout << "client connect  Failed!\n";
			return false;
		}
		node.socketFD = newFD;
	}
	int send_size = message.size();
	char *buffer = new char[send_size];
	strcpy(buffer, message.c_str());
	if (send(node.socketFD, buffer, send_size, 0) == -1) {
		std::cout << "client send  Failed!\n";
		return false;
	}
	delete[]buffer;
	return true;
}

string* Client::getUserList(){
	int size = userList.size();
	string *result = new string[size];
	for (int i = 0; i < size; i++) {
		result[i] = userList[i].message;
	}
	return result;
}

string Client::getMessage(string username){
	int user_i;
	if ((user_i = find_by_username(username) == -1)) return NULL;
	return userList[user_i].message;
}
