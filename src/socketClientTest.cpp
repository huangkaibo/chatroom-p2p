#include<stdio.h>
#include<iostream>
#include<time.h>
#include<winsock2.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main()
{
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if(WSAStartup(sockVersion, &wsaData)!=0)    
    {
        cout<<"end"<<endl;
        return 0;
    }

    SOCKET clientFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3490);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(clientFD, (sockaddr *)&serverAddr, sizeof(serverAddr));

    char data[] = "hello";

    send(clientFD, data, strlen(data), 0);

    char recvData[255];
    int ret = recv(clientFD, recvData, 255, 0);
    recvData[ret] = 0x00;
    
    cout<<recvData<<endl;
}