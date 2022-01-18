// MainProcess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 256

#include <iostream>
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/HashmapData.cpp"
#include "/4 GOD/SEMESTAR 1/2_IKP/PROJEKAT/IKP_Tim_16/IKP_Team_16/Common/RingBuffer.cpp"


int SendData(char* data, int dataSize, SOCKET connectSocket);
void Ispisi_bazu(int* dictionary);

int main()
{
    int unos;

    int* dictionary = NewHashArray();

    SOCKET connectSocket = INVALID_SOCKET;



    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
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

    iResult = recv(connectSocket, dataBuffer, 1, 0);

    dataBuffer[iResult] = '\0';
    

    printf("Prijavili ste se kao main proces broj %s.", dataBuffer);


    srand(time(0));

    

    while (true) {
        do {
            printf("\n1.Manuelni unos\n");
            printf("2.Automatski unos\n");
            printf("3.Ispis podataka\n");
            printf("4. Prekid konekcije\n");

            scanf_s("%d", &unos);

        } while (unos != 1 && unos != 2 && unos != 3 && unos !=4);



        int kljuc = 0;
        int vrednost;

        if (unos == 1) {
            printf("Unesite kljuc: (pritisnite 0 za povratak)\n");

            scanf_s("%d", &kljuc);

            if (kljuc == 0) {
                continue;
            }

            

            printf("Unesite vrednost:\n");

            scanf_s("%d", &vrednost);
            
            HashInsertOrUpdate(kljuc, vrednost, dictionary);
             
            Data_for_send data;

            data.process_id = htonl(1);
            data.key = htonl(kljuc);
            data.value = htonl(vrednost);

            

            SendData((char*)&data, sizeof(Data_for_send), connectSocket);

            
            
        }
        else if (unos == 2){

            int broj_iteracija;

            printf("Unesite broj podataka koji se salju:");

            scanf_s("%d", &broj_iteracija);

            while (broj_iteracija > 0) {
                kljuc = rand();
                vrednost = rand();

                HashInsertOrUpdate(kljuc, vrednost, dictionary);

                
                Data_for_send data;

                unsigned long idL = 1;
                unsigned long kljucL = (unsigned long)kljuc;
                unsigned long vrednostL = (unsigned long)vrednost;

                data.process_id = htonl(1);
                data.key = htonl(kljucL);
                data.value = htonl(vrednostL);
                

               
                
                SendData((char*)&data, (int)sizeof(data), connectSocket);
                
                broj_iteracija--;

                    
                Sleep(200);
            }
        }
        else if (unos == 3) {
            
            Ispisi_bazu(dictionary);
        }
        else {
            break;
        }
    }

    // Shutdown the connection since we're done
    iResult = shutdown(connectSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    

    // Close connected socket
    closesocket(connectSocket);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
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

        //int count = sizeof(*dictionary) / sizeof(int);

        for (int i = 0; i < 100; i++) {
            printf("%d || ", dictionary[i]);
        }
    }