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
//#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/HashmapData.cpp"
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/RingBuffer.cpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

typedef struct Process_m {
	int id;
	SOCKET socket;

} Process;

#define SERVER_PORT 27016
#define BUFFER_SIZE 256


DWORD WINAPI Prihvat(LPVOID lpParam);
DWORD WINAPI Prijava(LPVOID lpParam); 
DWORD WINAPI Slanje(LPVOID lpParam);

int SendData(char* data, int dataSize, SOCKET connectSocket);

SOCKET* clientSockets = (SOCKET*)malloc(sizeof(SOCKET));
int lastIndex = 0;

RingBuffer buffer;

HANDLE Empty;
HANDLE Full;
CRITICAL_SECTION BufferAccess;

int main()
{

	Process* Processes = (Process*)malloc(sizeof(Process));
	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	DWORD PrijavaID, PrihvatID, SlanjeID;
	HANDLE hPrijava, hPrihvat, hSlanje;
	
	Empty = CreateSemaphore(0, RING_SIZE, RING_SIZE, NULL);
	Full = CreateSemaphore(0, 0, RING_SIZE, NULL);
	InitializeCriticalSection(&BufferAccess);


	hPrijava = CreateThread(NULL, 0, &Prijava, NULL, 0, &PrijavaID);
	hSlanje = CreateThread(NULL, 0, &Slanje, NULL, 0, &SlanjeID);
	hPrihvat = CreateThread(NULL, 0, &Prihvat, NULL, 0, &PrihvatID);

	int liI = getchar();
	CloseHandle(hPrijava);
	CloseHandle(hPrihvat);
	CloseHandle(hSlanje);
	CloseHandle(Empty);
	CloseHandle(Full);
	

	for (int i = 0; i < lastIndex; i++)
	{
		closesocket(clientSockets[i]);
	}
	// Deinitialize WSA library

	DeleteCriticalSection(&BufferAccess);

	free(clientSockets);

	WSACleanup();

	return 0;
	
}





DWORD WINAPI Prijava(LPVOID lpParam)
{
	SOCKET listenSocket = INVALID_SOCKET;


	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

	// Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("T-PRIJAVA: socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	int iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("T-PRIJAVA: bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//// All connections are by default accepted by protocol stek if socket is in listening mode.
	//// With SO_CONDITIONAL_ACCEPT parameter set to true, connections will not be accepted by default
	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		printf("T-PRIJAVA: setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}

	unsigned long  mode = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &mode) != 0)
		printf("T-PRIJAVA: ioctlsocket failed with error.");

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("T-PRIJAVA: listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Main replikator je pokrenut.\n");

	


	fd_set readfds;

	while (true)
	{


		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		// initialize socket set
		FD_ZERO(&readfds);

		// add server's socket and clients' sockets to set

		FD_SET(listenSocket, &readfds);

		// wait for events on set
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("T-PRIJAVA: Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{

			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// New connection request is received. Add new socket in array on first free position.
			clientSockets[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSockets[lastIndex] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("T-PRIJAVA: accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("T-PRIJAVA: accept failed with error: %d\n", WSAGetLastError());
				}
			}
			else
			{
				unsigned long mode = 1;

				if (ioctlsocket(clientSockets[lastIndex], FIONBIO, &mode) != 0)
				{
					printf("T-PRIJAVA: ioctlsocket failed with error.");
					continue;
				}

				
				char dataBuffer[1];

				sprintf(dataBuffer, "%d", lastIndex);
				SendData(dataBuffer, sizeof(dataBuffer), clientSockets[lastIndex]);

				printf("Prijavljen main proces broj %d\n", lastIndex);
				lastIndex++;
				
				
				
			}
		}
	}
	

	return 0;
}



DWORD WINAPI Prihvat(LPVOID lpParam)
{
	fd_set readfds;

	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	SOCKET dummy = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol


	char dataBuffer[BUFFER_SIZE];

	
		while (true) {

			FD_ZERO(&readfds);

			
			FD_SET(dummy, &readfds);

			for (int i = 0; i < lastIndex; i++)
			{
				FD_SET(clientSockets[i], &readfds);
			}

			int selectResult = select(1, &readfds, NULL, NULL, &timeVal);

			if (selectResult == SOCKET_ERROR)
			{
				printf("T-PRIHVAT: Select failed with error: %d\n", WSAGetLastError());

				WSACleanup();
				return 1;
			}
			else if (selectResult == 0) // timeout expired
			{
				continue;
			}
			else
			{

				// Check if new message is received from connected clients
				for (int i = 0; i < lastIndex; i++)
				{
					// Check if new message is received from client on position "i"
					if (FD_ISSET(clientSockets[i], &readfds))
					{
						int iResult = recv(clientSockets[i], dataBuffer, 13, 0);

						if (iResult > 0)
						{
							dataBuffer[iResult] = '\0';
							printf("\nPristigla je poruka od klijenta broj (%d).\n", i);

							
							Data_for_send data = *(Data_for_send*)dataBuffer;
							
							data.process_id = i;

							WaitForSingleObject(Empty, INFINITE);

							EnterCriticalSection(&BufferAccess);

							BufferPut(&buffer, &data);

							
							LeaveCriticalSection(&BufferAccess);

							ReleaseSemaphore(Full, 1, NULL);
							
							
						
							

						}
						else if (iResult == 0)
						{
							// connection was closed gracefully
							printf("\nZatvorena konekcija sa klijentom broj (%d).\n", i);
							closesocket(clientSockets[i]);

							// sort array and clean last place
							for (int j = i; j < lastIndex - 1; j++)
							{
								clientSockets[j] = clientSockets[j + 1];
							}
							clientSockets[lastIndex - 1] = 0;

							lastIndex--;
						}
						else
						{
							// there was an error during recv
							printf("\nT-PRIHVAT: Recv failed with error: %d\n", WSAGetLastError());
							closesocket(clientSockets[i]);

							// sort array and clean last place
							for (int j = i; j < lastIndex - 1; j++)
							{
								clientSockets[j] = clientSockets[j + 1];
							}
							clientSockets[lastIndex - 1] = 0;

							lastIndex--;
						}
					}
				}
			}
		}
	

}


DWORD WINAPI Slanje(LPVOID lpParam)
{

	SOCKET connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("T-SLANJE: socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");	// ip address of server
	serverAddress.sin_port = htons(27017);

	int iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("T-SLANJE: Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	Data_for_send data;
	Data_for_send* dataPok = NULL;

	while (true) {


		WaitForSingleObject(Full, INFINITE);

		EnterCriticalSection(&BufferAccess);

		data = BufferGet(&buffer);

		

		dataPok = &data;

		LeaveCriticalSection(&BufferAccess);

		ReleaseSemaphore(Empty, 1, NULL);

		


		int dataSize = sizeof(data);

		SendData((char*)dataPok, dataSize, connectSocket);

		

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
