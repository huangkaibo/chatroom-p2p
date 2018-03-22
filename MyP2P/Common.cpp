#include"Common.h"

Node Common::server = { "127.0.0.1",3490,NULL,NULL,NULL };

Node Common::getServer_Node() {
	return server;
}

unsigned short Common::getAvailablePort() {
	SOCKET test_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in test_addr;
	test_addr.sin_family = AF_INET;
	test_addr.sin_addr.s_addr = INADDR_ANY;
	for (unsigned short i = 1025; i < 65535; i++) {
		test_addr.sin_port = htons(i);
		if (bind(test_sock, (sockaddr*)&test_addr, sizeof(sockaddr)) != SOCKET_ERROR) {
			closesocket(test_sock);
			return i;
		}
	}
}

bool Common::compare_node(Node node1, Node node2) {
	return strcmp(node1.ip, node2.ip) == 0 && node1.port == node2.port;
}
