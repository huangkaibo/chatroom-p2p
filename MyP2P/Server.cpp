#include"Server.h"

//将新Client添加进userList
Node Server::addToUserList(SOCKET clientFD, sockaddr_in clientAddr) {
	Node client_node;
	//获取Client自身服务器的port
	recv(clientFD, (char*)&client_node.port, sizeof(unsigned short), 0);
	//获取username
	int len = recv(clientFD, client_node.username, username_len, 0);
	client_node.username[len] = 0x00;
	//构造client_node
	strcpy(client_node.ip, inet_ntoa(clientAddr.sin_addr));
	client_node.socketFD = clientFD;
	//加入到userList
	userList.push_back(client_node);
	std::cout << client_node.ip << " " << client_node.port << " " << client_node.username << "\n";
	return client_node;
}

//分发userList数据
void Server::distriubuteUserList() {
	int node_num = userList.size();
	if (node_num != 0) {
		Node *buffer =(Node*) malloc(node_num * sizeof(Node));		//用new的话会在delete时报错，为什么？
		memcpy(buffer, &userList[0], node_num * sizeof(Node));		//将userList中的数据存到buffer中，相当于序列号
		for (int i = 0; i < node_num; i++) {						//遍历userList，分发userList数据
			send(userList[i].socketFD, (char*)&node_num, sizeof(int), 0);
			send(userList[i].socketFD, (char*)buffer, node_num * sizeof(Node), 0);
		}
		free(buffer);
	}
}

//监听Client登录
void Server::logIn() {
	if (listen(serverFD, 10) != 0) {
		std::cout << "listen failed!'\n";
		return;
	}
	Node client_node;
	while (true) {
		sockaddr_in clientAddr;
		int sin_size = sizeof(clientAddr);
		SOCKET clientFD = accept(serverFD, (sockaddr *)&clientAddr, &sin_size);
		if (clientFD == INVALID_SOCKET) {
			std::cout << "accept failed!\n";
			continue;
		}
		std::cout << "accept success\n";
		client_node = addToUserList(clientFD, clientAddr);
		distriubuteUserList();

		thread receive_request_th(&Server::logOut, this, client_node);			//开启接听用户请求的线程
		receive_request_th.detach();											//detach(),将线程解绑。
	}
}

//监听用户请求
void Server::logOut(Node client_node) {
	while (true) {
		char request_str[100];
		int len = recv(client_node.socketFD, request_str, 100, 0);
		if (len != -1) {
			request_str[len] = 0x00;
			if (strcmp(request_str, "0.0LOGOUT0.0") == 0) {					//用户下线的指令：0.0LOGOUT0.0
				int i;
				for (i = 0; i < userList.size(); i++) {
					if (Common::compare_node(userList[i], client_node)) {	//判别是哪个Client下线
						std::cout << client_node.username << " logout\n";
						break;
					}
				}
				userList.erase(userList.begin() + i);						//从userList中删除该node
				distriubuteUserList();										//分发userList
			}
		}
	}
}

//构造函数
Server::Server(int a) {

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {					//构建socket环境
		std::cout << "winsock init Failed!\n";
		return;
	}
	if ((serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		std::cout << "socket error\n";
		return;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Common::getServer_Node().port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
}

//启动服务器
void Server::start() {
	if (bind(serverFD, (sockaddr*)&serverAddr, sizeof(sockaddr)) == SOCKET_ERROR) {
		std::cout << "bind failed! " << "\n";
		return;
	}
	thread listen_th(&Server::logIn, this);		//开启线程监听连接
	listen_th.detach();							
}
