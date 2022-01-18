#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <iostream>
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/HashmapData.cpp"
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/RingBuffer.cpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27018
#define BUFFER_SIZE 256

typedef struct Delta {
	Data_for_send* data;
	int size;
} Delta;

int SendData(char* data, int dataSize, SOCKET connectSocket);
void Ispisi_bazu(int* dictionary);
FILE* safe_fopen(char* filename, char* mode);
void save_to(FILE* out, int* dictionary);
void read_from(FILE* in, int* dictionary);

DWORD WINAPI Prihvat(LPVOID lpParam);

int* dictionary = NewHashArray();


SOCKET connectSocket = INVALID_SOCKET;

int main() {

   
    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

	
    int id;

    printf("Unesite id rezerve:\n");

    scanf_s("%d", &id);


	//////////////
	char filename[14];

	sprintf(filename, "database%d.txt", id);

	printf(filename);

	char mode[] = "r";

	FILE* file = safe_fopen(filename, mode);

	if (file != NULL) {
		read_from(file, dictionary);
		fclose(file);
	}

	

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;								// IPv4 protocol
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
    serverAddress.sin_port = htons(SERVER_PORT);					// server port

    // Connect to server specified in serverAddress and socket connectSocket
    int iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

	char dataBuffer[2];

	sprintf(dataBuffer, "%d", id);
	SendData((char*)dataBuffer, sizeof(dataBuffer), connectSocket);

	Sleep(1000);

	int broj = 0;
	SendData((char*)"IU", 2, connectSocket);



	iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);	//primamo delta objekte

	if (iResult > 0)
	{
		//dataBuffer[iResult] = '\0';

		Delta* delta = (Delta*)dataBuffer;

		Data_for_send* data = delta->data;
		int broj = delta->size;

		for (int i = 0; i < broj; i++) {
			HashInsertOrUpdate(data[i].key, data[i].value, dictionary);
		}
		

	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("\nConnection with client closed");
		closesocket(connectSocket);


	}
	else
	{
		// there was an error during recv
		printf("\nRecv failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);


	}


	DWORD PrihvatID;
	HANDLE hPrihvat;

	hPrihvat = CreateThread(NULL, 0, &Prihvat, NULL, 0, &PrihvatID);


	char a;

	

	while (true) {

		printf("1. Ispis baze\n");
		printf("2. Odjava\n");		

		scanf("%c", &a);

		
		if (a == '1') {
			Ispisi_bazu(dictionary);
		}
		else if (a == '2') {

			iResult = shutdown(connectSocket, SD_BOTH);

			// Check if connection is succesfully shut down.
			if (iResult == SOCKET_ERROR)
			{
				printf("Shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			break;
		}
	}

	char modee[] = "w";

	file = safe_fopen(filename, modee);

	save_to(file, dictionary);

	fclose(file);

	free(dictionary);
	closesocket(connectSocket);
	CloseHandle(Prihvat);
	WSACleanup();

	return 0;
}




DWORD WINAPI Prihvat(LPVOID lpParam)
{
	fd_set readfds;

	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	

	char dataBuffer[BUFFER_SIZE];

	

	while (true) {



		FD_ZERO(&readfds);


		FD_SET(connectSocket, &readfds);
		


		int selectResult = select(1, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());

			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{
			continue;
		}
		else if (FD_ISSET(connectSocket, &readfds))
		{
			int iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);

			if (iResult > 0)
			{
				dataBuffer[iResult] = '\0';

				Data_for_send data = *(Data_for_send*)dataBuffer;



				int kljuc = ntohl(data.key);
				int vrednost = ntohl(data.value);

				HashInsertOrUpdate(kljuc, vrednost, dictionary);

				printf("\nPrimljeni podaci:\nKljuc: %d, vrednost: %d", kljuc, vrednost);

			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("\nConnection with client closed");
				closesocket(connectSocket);


			}
			else
			{
				// there was an error during recv
				printf("\nRecv failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);


			}
		}
	}
}




int SendData(char* data, int dataSize, SOCKET connectSocket) {


	int iResult = send(connectSocket, data, dataSize, 0);

	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	printf("Message successfully sent. Total bytes: %ld\n", iResult);


	return 0;
}



void Ispisi_bazu(int* dictionary) {

	

	for (int i = 0; i < 100; i++) {
		printf("%d || ", dictionary[i]);
	}
}

FILE* safe_fopen(char* filename, char* mode) {
	FILE* fp = fopen(filename, mode);
	
	return fp;
}


void save_to(FILE* out, int* dictionary)
{
	for (int i = 0; i < 100; i++) {

		fprintf(out, "%d %d\n", i, dictionary[i]);
	}
	
}

void read_from(FILE* in, int* dictionary) {
	int key;
	int value;

	while (fscanf(in, "%d %d", &key, &value) != EOF) {
		HashInsertOrUpdate(key, value, dictionary);
	}
}





