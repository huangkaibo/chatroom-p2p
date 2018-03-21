#include"Server.h"

Node Server::addToUserList(SOCKET clientFD,sockaddr_in clientAddr){
	Node client_node;
	std::cout << "adding\n";
	//��ȡip
	//��ȡusername
	int len = recv(clientFD, client_node.username, username_len, 0);
	client_node.username[len] = 0x00;
	//��װclient_node
	strcpy(client_node.ip, inet_ntoa(clientAddr.sin_addr));
	client_node.port = clientAddr.sin_port;
	client_node.socketFD = clientFD;
	//ѹ��userList
	userList.push_back(client_node);
	std::cout << client_node.ip << " " << client_node.port << " " << client_node.username << "\n";
	return client_node;
}

void Server::distriubuteUserList(){
	int node_num = userList.size();
	if (node_num != 0) {
		Node *buffer = new Node[node_num];
		memcpy(buffer, &userList[0], node_num * sizeof(Node));
		for (vector<Node>::iterator user = userList.begin(); user != userList.cend(); user++) {
			send(user->socketFD, (char*)&node_num, sizeof(int), 0);				//�ȷ���node����
			send(user->socketFD, (char*)buffer, node_num * sizeof(Node), 0);	//�ٷ���node����
		}
		delete[]buffer;
	}
}

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
		thread receive_request_th(&Server::logOut, this, client_node);
		//logOut(client_node);
		
		/*while (true);*/
		receive_request_th.detach();
	}
}

void Server::logOut(Node client_node) {
	while (true) {
		char request_str[100];
		int len = recv(client_node.socketFD, request_str, 100, 0);
		if (len != -1) {
			request_str[len] = 0x00;
			if (strcmp(request_str, "0.0�û�����0.0")==0) {
				int i = 0;
				for (vector<Node>::iterator user = userList.begin(); user != userList.cend(); user++, i++) {
					if (Common::compare_node(*user, client_node)) {
						std::cout << client_node.username << "logout\n";
						//userList.erase(userList.begin() + i);
						break;
					}
				}
				userList.erase(userList.begin() + i);
				distriubuteUserList();
			}
		}
	}
}

Server::Server(int a) {

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {
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

void Server::start(){
	if (bind(serverFD, (sockaddr*)&serverAddr, sizeof(sockaddr))==SOCKET_ERROR) {
		std::cout << "bind failed! "<<"\n";
		return;
	}
	//���������߳�
	thread listen_th(&Server::logIn,this);
	listen_th.detach();							//detach()���̴߳��������з��룬�������У������������̣߳�
}
