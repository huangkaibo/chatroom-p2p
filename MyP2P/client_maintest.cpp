#include"Client.h"

int main() {
	Client *client = new Client();
	string s;
	std::cin >> s;
	client->logIn(s);
	//while (std::cin);
	int a;
	while (std::cin >> a) {
		if (a == 1) {
			break;
		}
		else if (a == 2) {
			std::cout << "\ncin username\n";
			string username, message;
			std::cin >> username >> message;
			client->sendMessage(username, message);
		}
	}
	client->logOut();
	system("pause");

}