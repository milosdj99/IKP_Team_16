#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <iostream>
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/HashmapDelta.cpp"
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/RingBuffer.cpp"




#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define SERVER_PORT 27017
#define BUFFER_SIZE 256

typedef struct Delta {
	Data_for_send* data;
	int size;
} Delta;


int SendData(char* data, int dataSize, SOCKET connectSocket);

DWORD WINAPI Prihvat(LPVOID lpParam);
DWORD WINAPI Prijava(LPVOID lpParam);
DWORD WINAPI IU(LPVOID lpParam);
DWORD WINAPI Slanje(LPVOID lpParam);

typedef struct Rezerva_m {
	int id;
	SOCKET socket;
	bool podignuta;

} Rezerva;

Rezerva* rezerve = (Rezerva*)calloc(0, sizeof(Rezerva));
int br_rezervi = 0;

RingBuffer buffer;


HANDLE Empty;
HANDLE Full;
CRITICAL_SECTION BufferAccess;


Data_for_send** dictionaryDelta = NewHashArray();
int array_of_sizes[100];

int main()
{
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	for (int i = 0; i < 100; i++) {
		array_of_sizes[i] = 0;
	}
	

	DWORD  PrihvatID;
	DWORD  PrijavaID;
	DWORD  IUID;
	DWORD  SlanjeID;

	HANDLE hPrihvat;
	HANDLE hPrijava;
	HANDLE hIU;
	HANDLE hSlanje;

	Empty = CreateSemaphore(0, RING_SIZE, RING_SIZE, NULL);
	Full = CreateSemaphore(0, 0, RING_SIZE, NULL);
	InitializeCriticalSection(&BufferAccess);
	
	hPrihvat = CreateThread(NULL, 0, &Prihvat, NULL, 0, &PrihvatID);
	hPrijava = CreateThread(NULL, 0, &Prijava, NULL, 0, &PrijavaID);
	hIU = CreateThread(NULL, 0, &IU, NULL, 0, &IUID);
	hSlanje = CreateThread(NULL, 0, &Slanje, NULL, 0, &SlanjeID);
	
	int liI = getchar();

	CloseHandle(hPrihvat);
	CloseHandle(hPrijava);
	CloseHandle(hIU);
	CloseHandle(hSlanje);
	CloseHandle(Empty);
	CloseHandle(Full);


	DeleteCriticalSection(&BufferAccess);

	WSACleanup();

	free(rezerve);
	free(dictionaryDelta);

	return 0;
}

DWORD WINAPI Prihvat(LPVOID lpParam)
{
	fd_set readfds;

	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	SOCKET listenSocket = INVALID_SOCKET;

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
		printf("T-PRIHVAT: socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	int iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("T-PRIHVAT: bind failed with error: %d\n", WSAGetLastError());
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
		printf("T-PRIHVAT: setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}

	unsigned long  mode = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &mode) != 0)
		printf("T-PRIHVAT: ioctlsocket failed with error.");

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("T-PRIHVAT: listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Soket namenjen Replicator Main-u je stavljen u stanje slusanja.\n");



	char dataBuffer[13];

	SOCKET ReplicatorMainSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	while (true) {



		FD_ZERO(&readfds);


		FD_SET(listenSocket, &readfds);
		FD_SET(ReplicatorMainSocket, &readfds);


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
		else if (FD_ISSET(listenSocket, &readfds))
		{
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);
			// Check if new message is received from connected clients
			ReplicatorMainSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (ReplicatorMainSocket == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("T-PRIHVAT: accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("T-PRIHVAT: accept failed with error: %d\n", WSAGetLastError());
				}
			}
			else
			{
				unsigned long mode = 1;

				if (ioctlsocket(ReplicatorMainSocket, FIONBIO, &mode) != 0)
				{
					printf("T-PRIHVAT: ioctlsocket failed with error.");
					continue;
				}

				printf("\nReplicator main je uspostavio konekciju.");
				


			}

		}
		else if (FD_ISSET(ReplicatorMainSocket, &readfds))
		{
			int iResult = recv(ReplicatorMainSocket, dataBuffer, 13, 0);

			

			if (iResult > 0)
			{
				dataBuffer[iResult] = '\0';
				printf("\nMessage received");

				Data_for_send data = *(Data_for_send*)dataBuffer;


				WaitForSingleObject(Empty, INFINITE);

				EnterCriticalSection(&BufferAccess);

				BufferPut(&buffer, &data);


				LeaveCriticalSection(&BufferAccess);

				ReleaseSemaphore(Full, 1, NULL);



			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("\nKonekcija sa Main replikatorom uspesno zatvorena.");
				closesocket(ReplicatorMainSocket);


			}
			else
			{
				// there was an error during recv
				printf("\nT-PRIHVAT: Recv failed with error: %d\n", WSAGetLastError());
				closesocket(ReplicatorMainSocket);


			}
		}
	}
}





DWORD WINAPI Prijava(LPVOID lpParam)
{
		SOCKET listenSocket = INVALID_SOCKET;

		

		// Initialize serverAddress structure used by bind
		sockaddr_in serverAddress;
		memset((char*)&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;				// IPv4 address family
		serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
		serverAddress.sin_port = htons(27018);	// Use specific port



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

		printf("Socket namenjen prijavi rezervi je stavljen u stanje slusanja.\n");

		


		fd_set readfds;

		fd_set rezervafds;

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
				printf("Select failed with error: %d\n", WSAGetLastError());
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
				SOCKET rezervaSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
				

				if (rezervaSocket == INVALID_SOCKET)
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

					if (ioctlsocket(rezervaSocket, FIONBIO, &mode) != 0)
					{
						printf("T-PRIJAVA: ioctlsocket failed with error.");
						continue;
					}
					
					

					

					while (true) {

						
						FD_ZERO(&rezervafds);

						FD_SET(rezervaSocket, &rezervafds);

						int selectResult = select(0, &rezervafds, NULL, NULL, &timeVal);

						if (selectResult == SOCKET_ERROR)
						{
							printf("Select failed with error: %d\n", WSAGetLastError());
							closesocket(listenSocket);
							WSACleanup();
							return 1;
						}
						else if (selectResult == 0) // timeout expired
						{

							continue;
						}
						else if (FD_ISSET(rezervaSocket, &rezervafds))
						{
							char dataBuffer[1];

							int iResult = recv(rezervaSocket, dataBuffer, 1, 0);

							if (iResult > 0)
							{
								
								

								int id = atoi(dataBuffer);


								if (id <= br_rezervi-1) {		///rezerva je vec prijavljivana

									rezerve[id].socket = rezervaSocket;
									rezerve[id].podignuta = true;
									printf("\nPonovo je prijavljena je rezerva broj %d", id);

								}
								else {
									Rezerva rez;
									rez.id = id;
									rez.socket = rezervaSocket;
									rez.podignuta = true;

									rezerve[br_rezervi] = rez;

									printf("\nPrijavljena je rezerva broj %d", id);

									br_rezervi++;
								}
							}
							else if (iResult == 0)
							{
								// connection was closed gracefully
								printf("\nT-PRIJAVA: Konekcija sa rezervom je zatvorena.");
								//closesocket(ReplicatorMainSocket);


							}
							else
							{
								// there was an error during recv
								printf("\nT-PRIJAVA: Recv failed with error: %d\n", WSAGetLastError());
								//losesocket(ReplicatorMainSocket);


							}
							break;
						}
						
					}

										

				}
			}
		}
		
}

DWORD WINAPI IU(LPVOID lpParam)
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

		
		


		for (int i = 0; i < br_rezervi; i++)
		{
			FD_SET(rezerve[i].socket, &readfds);
		}

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
		else
		{

			// Check if new message is received from connected clients
			for (int i = 0; i < br_rezervi; i++)
			{
				// Check if new message is received from client on position "i"
				if (FD_ISSET(rezerve[i].socket, &readfds))
				{
					int iResult = recv(rezerve[i].socket, dataBuffer, BUFFER_SIZE, 0);

					if (iResult > 0)
					{
						dataBuffer[iResult] = '\0';
						printf("\n IU: Pristigla je poruka od rezerve broj (%d).\n", i);

						if (strcmp(dataBuffer, "IU") == 0) {

							printf("Zapocet IU sa rezervom %d.", i);

							Data_for_send* niz = HashSearch(i, dictionaryDelta);

							int velicina = array_of_sizes[i];				//da bi rezerva znala koliko da cita

							Delta delta;

							delta.data = niz;
							delta.size = velicina;

							Delta* deltaPok = &delta;

							SendData((char*)deltaPok, sizeof(velicina), rezerve[i].socket);

							

							printf("Zavrsen IU sa rezervom %d.", i);
						}
						





					}
					else if (iResult == 0)
					{
						// connection was closed gracefully
						printf("\nZatvorena konekcija sa rezervom broj (%d).\n", i);
						rezerve[i].podignuta = false;

						
						
					}
					else
					{
						// there was an error during recv
						printf("\nRecv failed with error: %d\n", WSAGetLastError());
						closesocket(rezerve[i].socket);

						// sort array and clean last place
						
					}
				}
			}
		}
	}


}

DWORD WINAPI Slanje(LPVOID lpParam)
{


	Data_for_send data;
	Data_for_send* dataPok = NULL;

	while (true) {

		WaitForSingleObject(Full, INFINITE);

		EnterCriticalSection(&BufferAccess);

		data = BufferGet(&buffer);
		
		LeaveCriticalSection(&BufferAccess);

		ReleaseSemaphore(Empty, 1, NULL);




		dataPok = &data;

		int process_id = data.process_id;

		printf("%d", process_id);
		for (int i = 0; i < br_rezervi; i++) {

			if (rezerve[i].id == process_id && rezerve[i].podignuta == true) {

				SendData((char*)dataPok, sizeof(data), rezerve[i].socket);

			}
			else{

				HashInsertOrUpdate(process_id, data, dictionaryDelta, array_of_sizes);

				array_of_sizes[process_id]++;

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

	//char dataBuffer[256];

	//iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);

	//printf(dataBuffer);

	return 0;
}



