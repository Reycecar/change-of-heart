#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <iostream>
#include <Lmcons.h>
#include <wininet.h>
#include <winsock.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <fileapi.h>
#include <string.h>
#include <rpc.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Rpcrt4.lib")

#define PORT "25565" //Default port for Minecraft Java
#define SERV_ADDR "127.0.0.1"
#define DEFAULT_BUFFER_LEN 40000
using namespace std;

char* dec_uuids(char* data) {
	int len = sizeof(data[0]);
	printf("%d\n", len);
	int elements = sizeof data / len;
	char key = *data; // key = first uuid value in data array
	char callcode = *(data + 1); //callcode = second uuid value in data array
	printf("key: %s\n", key);
	printf("callcode: %s\n", callcode);
	
	
	//for (int i = 1; i < elements; i++) {
	//	printf("callcode (encrypted): %s\n", callcode);
	//	for (int j = 0; j < len; j++) {
	//		(data + i)[j] = ((data + i)[j] == '-') ? '-' : (data + i)[j] ^ key[j];
	//	}
	//	printf("callcode (decrypted): %s\n", *(data + i));
	//}
	return data;
}

void main() {
	char* data[] = {
	"54686973-4973-416e-584f-526b65793031", //key "ThisIsAnXORkey01"
	"3c0d051f-2604-2e1c-342b-6359564d0507"  //encrypted "helloworld123456"
	};
	dec_uuids(*data);
	printf("callcode (decrypted): %s\n", *(data + 1));
}