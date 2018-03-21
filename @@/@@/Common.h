#ifndef COMMON_H
#define COMMON_H

#include"Node.h"
class Common {
private:
	static Node server;
public:
	static Node getServer_Node();
	static unsigned short getAvailablePort();
	static bool compare_node(Node node1, Node node2);
};

#endif COMMON_H
