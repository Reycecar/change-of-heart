#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <string>
#include <iostream>



// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "9999"

//create client struct //change dog to hearts
DWORD WINAPI HandleClient(LPVOID hearts);
struct client {
    int id;
    SOCKET socket;
};

// Function to check if a command is valid
bool isValidCommand(const std::string& command) {
    // Add command validation logic 
    std::vector<std::string> validCommands = {"command1", "command2", "command3", "command4"};
    // Check if the command is in the list of valid commands
    for (const std::string& validCommand : validCommands) {
        if (command == validCommand) {
            // Return true if the command is valid
            return true;
        }
    }

    // Return false if the command is not valid
    return false;
    // return true if the command is valid, false otherwise
    return (command == "command1" || command == "command2" || command == "command3");
}

int main (void) 
{
    //initialize winsoc
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct client* newClient = (client*)malloc(sizeof(client));

    //error handling
    if (iResult != 0) {
        printf("WSAStartup failed with error %d\n", iResult);
        return 1;
    }

    int clID = 0;

    //new listening socket
    struct addrinfo* result = NULL;
    struct addrinfo hints;

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

    // start listening for clients
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

    // CODE BELOW NEEDS A HOME 
    // 
    // Main server loop TM (?) may combine with second loop for recieving
    // or may keep seperate one for recieving one for sending
    while (true) {
        std::cout << "Server waiting for input: ";
        std::string input;
        std::cin >> input;

        // Check if the input matches a command
        if (isValidCommand(input)) {
            // Send corresponding response
            if (input == "command1") {
                std::cout << "Command 1 executed." << std::endl;
                // Add code here to send response for command1
                send(NewCliSock, "getFileUpload()\n", 15, 0);
            } 
            else if (input == "command2") {
                std::cout << "Command 2 executed." << std::endl;
                // Add code here to send response for command2
                send(NewCliSock, "getFileDownload()\n", 17, 0);
            }
            else if (input == "command3") {
                std::cout << "Command 3 executed." << std::endl;
                // Add code here to send response for command3
                send(NewCliSock, "getUsername()\n", 13, 0);    
            }
            else if (input == "command4") {
                std::cout << "Command 4 executed." << std::endl;
                // Add code here to send response for command4
                send(NewCliSock, "getSysInfo()\n", 13, 0);    
            }
            
        } else {
            // Handle invalid command
            std::cout << "Invalid command. Please try again." << std::endl;
        }
    }
    
    //                  __/\__
    // FIND ME A HOME   | || | 
    //

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