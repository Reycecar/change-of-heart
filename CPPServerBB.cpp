#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main()
{
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsok = WSAStartup(ver, &wsData);
	if (wsok != 0)
	{
		std::cerr << "Failed to INITIALIZE WINSOCK" << endl;
		return; // little to no error handling present
	}
	
	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Failed to CREATE SOCKET" << endl;
		return;
	}

	// Bind IP and port to socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000); //htons = host to network shorts
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton
	
	bind(listening, (sockaddr*)&hint, sizeof(hint)); // binds listening socket

	// Tell Winsock the socket is for listening 
    // doesnt accept connections simply listens
    // SOMAXCONN is 0x7fffffff long so it can listen for that many connections
	listen(listening, SOMAXCONN);

	// Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

    // deals with accepting the connection
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    // if (clientSocket = INVALIDSOCKET) do something

	char host[NI_MAXHOST];		// Client's remote name/or substitute for IP (would change for us)
	char service[NI_MAXSERV];	// Service (i.e. port) the client is connected on

	ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);
    //memset(host, 0, NI_MAXHOST); would be used in mac and linux

    // check if we can get the name information and if we cannot we will display the IP
    // can change for our functionality of the client
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << " CONNECTED ON PORT: " << service << endl;
	}
	else // else statement will only get IP
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST); //client.sin_addr gets IP
		std::cout << host << " CONNECTED ON PORT: " << ntohs(client.sin_port) << endl;
	}

	// Close listening socket
	closesocket(listening);

	// While loop: accept and echo message back to client
	char buf[4096];
    // reading until there are no bytes left to read but for mem issues 4k bytes is prob enough

	while (true)
	{
		ZeroMemory(buf, 4096);

		// Wait for client data
		int bytesReceived = recv(clientSocket, buf, 4096, 0); //recieve function returns num of bytes recived
		if (bytesReceived == SOCKET_ERROR)
		{
			std::cerr << "Error recieving data :( EXITING..." << endl;
			break;
		}

		if (bytesReceived == 0)
		{
			std::cout << "CLIENT HAS DISCONNECTED " << endl;
			break;
		}

		cout << string(buf, 0, bytesReceived) << endl;

		// Echo message back to client
        // for testing purposes/can change for our functionality
		send(clientSocket, buf, bytesReceived + 1, 0); // bytes recieved +1 to include the terminating 0 at the end
	
	}
	
	// Close the socket
	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();

	system("pause");
}
