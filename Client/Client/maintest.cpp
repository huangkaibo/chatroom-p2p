#include"Client.h"

int main() {
	Client *client=new Client();
	string s;
	std::cin >> s;
	client->logIN(s);
	//while (std::cin);
	client->logOut();
	system("pause");

}