#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "9999"
DWORD WINAPI HandleClient(LPVOID dog);
struct client {
    int id;
    SOCKET socket;
};

int  main(void)
{
    //init winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct client* newClient = (client*)malloc(sizeof(client));

    if (iResult != 0) {
        printf("WSAStartup failed with error %d\n", iResult);
        return 1;
    }

    int clID = 0;

    //New Listening socket
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    //ZeroMemory(&hints, sizeof(hints)); //equivalent to
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connection to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Socket failed with error %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket (bind)
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    // start listening
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    //while loop for accepting clients
    while (1) {
        newClient->socket = accept(ListenSocket, NULL, NULL);
        if (newClient->socket == INVALID_SOCKET) {
            printf("accept failed %d\n", WSAGetLastError());
            //closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        struct client* data = (struct client*)malloc(sizeof(client));
        data->socket = newClient->socket;
        data->id = clID;
        HANDLE thread = CreateThread(NULL, 0, HandleClient, (LPVOID)data, 0, NULL);
        clID++;
    }
    return 0;
}

DWORD WINAPI HandleClient(LPVOID newClient) {
    struct client* data = (struct client*)newClient;
    SOCKET NewCliSock = data->socket;
    int id = data->id;
    int iResultSend;
    int randNum;
    int result;
    int iResultString;

    char recvbuf[DEFAULT_BUFLEN];
    char buf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    printf("Connecting Client %d\n", id);
    sprintf_s(buf, DEFAULT_BUFLEN, "Welcome Client %d, Thanks for connecting!", id);
    send(NewCliSock, buf, DEFAULT_BUFLEN, 0);

    char randstring[3];

    while (1) {
        memset(buf, 0, recvbuflen);
        memset(recvbuf, 0, recvbuflen);
        memset(randstring, 0, sizeof(randstring));

        result = recv(NewCliSock, recvbuf, recvbuflen, 0); // input from client
        if (strcmp(recvbuf, "quit") == 0) {
            printf("Disconnecting Client %d\n", id);
            closesocket(NewCliSock);
            break;
        }

        if (result > 0) {
            randNum = 2 + (rand() % 9);
            sprintf_s(randstring, 3, "%d", randNum);
            iResultString = send(NewCliSock, randstring, (int)strlen(randstring), 0);
            for (int i = 0; i < randNum; i++)
            {
                iResultSend = send(NewCliSock, recvbuf, (int)strlen(recvbuf), 0);
                Sleep(2);
                if (iResultSend == SOCKET_ERROR) {
                    printf("Send failed (1) %d\n", WSAGetLastError());
                    closesocket(NewCliSock);
                    WSACleanup();
                    return 1;
                }
            }
            if (iResultString == SOCKET_ERROR) {
                printf("Send failed (2) %d\n", WSAGetLastError());
                closesocket(NewCliSock);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            printf("Closing connection");
        }
        else if (result == SOCKET_ERROR) {
            printf("recv failed with error %d\n", WSAGetLastError());
            closesocket(NewCliSock);
            WSACleanup();
            return 1;
        }
    }
}