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

    SOCKET serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3490);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    //bzero(&(serverAddr.sin_zero), 8);

    bind(serverFD, (sockaddr*)&serverAddr, sizeof(sockaddr));

    listen(serverFD, 10);

    SOCKET clientFD;

    sockaddr_in clientAddr;
    int sin_size = sizeof(clientAddr);
    clientFD = accept(serverFD, (SOCKADDR *)&clientAddr, &sin_size);

    char recvData[255];
    int ret = recv(clientFD, recvData, 255, 0);
    recvData[ret] = 0x00;
    
    cout<<recvData<<endl;
}