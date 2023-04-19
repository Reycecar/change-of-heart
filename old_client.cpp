#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "9999"

int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    char* server = "localhost";
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(server, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("Connection failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break; //got a connection, or none worked
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    char buf[DEFAULT_BUFLEN];
    char rep[3];
    int temp;

    recv(ConnectSocket, buf, DEFAULT_BUFLEN, 0);

    printf("%s\n", buf);


    // Receive until the peer closes the connection
    do {
        memset(buf, 0, DEFAULT_BUFLEN);
        memset(recvbuf, 0, DEFAULT_BUFLEN);
        memset(rep, 0, sizeof(rep));
        printf("enter a word to send to the server: ");
        scanf_s("%s", recvbuf, DEFAULT_BUFLEN);

        iResult = send(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failure with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        if (strcmp(recvbuf, "quit") == 0) {
            break;
        }

        recv(ConnectSocket, rep, sizeof(rep), 0); // should be the random num
        temp = atoi(rep);

        for (int i = 0; i < temp; i++) {
            memset(recvbuf, 0, DEFAULT_BUFLEN);
            if (iResult > 0) {
                printf("Bytes received: %d\n", iResult);
                printf("Received: %s\n", recvbuf);
            }
            else if (iResult == 0) {
                printf("Connection closed\n");
            }
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
            }
        }

    } while (strcmp(recvbuf, "quit") != 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}