#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <strsafe.h>
#include <wchar.h>
#include <process.h>
#include "MutexHelper.h"
#include "CallbackLinkedList.h"
#ifndef SERIAL_INTERFACE
#define SERIAL_INTERFACE

// 04/26/2022
// 6hr

#define SERIAL_SEND_BUF_SIZE 20
/*
Microsoft VC USB interface documentation
https://docs.microsoft.com/en-us/previous-versions/ms810467(v=msdn.10)?redirectedfrom=MSDN

Guide to interfacing with USB
https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
*/

/// <summary>
/// Wrapper function for QueryDosDevice to make Intellisense more legible
/// </summary>
/// <param name="deviceName"> The name of the device to search for [MUST BE A wchar_t*]!!!! </param>
/// <param name="pathBuffer"> Buffer in which to write the path for the device </param>
/// <param name="maxBufferSize"> The maximum size of the <paramref name="pathBuffer"/></param>
int QueryDosDeviceCustom(wchar_t* deviceName, TCHAR* pathBuffer, int maxBufferSize)
{
	return QueryDosDeviceW(deviceName, pathBuffer, maxBufferSize);
}

/// <summary>
///	Converts a char* string to a wchar_t* string
/// </summary>
/// <param name="str">The char* to be converted</param>
/// <return> Returns a wchar_t* with the data of the char* converted</return>
wchar_t* charToWchar(const char* str)
{
	// Get the length of the "text" string (account for \0 character at end)
	const size_t size = strlen(str) + 1;
	size_t charsConverted;	// Size of characters converted

	wchar_t* wStr = (wchar_t*)malloc(sizeof(wchar_t) * size);	// Allocate space for wchar_t string

	mbstowcs_s(&charsConverted, wStr, size, str, size + 1);		// Converts the char array to a wchar_t array
	return wStr;
}

// Communication channels for serial

// Synchronous write and read channel
HANDLE hComm;

COMMTIMEOUTS timeouts; // Set timeouts for the port

CallbackLinkedList callbackList;

// Prevents multiple threads from modifying linked list at same time
HANDLE linkedListMutex;

// Handle for the listen thread
HANDLE listenThreadHandle = NULL;

/// <summary>
/// Closes the thread listening for serial data
/// </summary>
void closeSerialListenThread()
{
	CloseHandle(listenThreadHandle);
}

/// <summary>
/// Writes to a buffer
/// 
/// DWORD is an unsigned long
/// 
/// </summary>
/// <param name="lpBuf">The chars to send through serial</param>
/// <param name="dwToWrite">The size of the <paramref name="lpBuf"/> char array</param>
/// <returns>Returns 1 if operation was a success and 0 if writing failed.</returns>
int write_buffer(char* lpBuf, DWORD dwToWrite)
{
	OVERLAPPED osWrite = { 0 };
	DWORD dwWritten;
	int fRes;

	fRes = WriteFile(hComm,	// Event handler
		lpBuf,				// Character buffer to write
		dwToWrite,			// Number of bytes to write
		&dwWritten,			// Variable that receives number of bytes written
		&osWrite);			// OVERLAPPED structre that contains asynchronous I/O information

	if (fRes != 1)
	{
		printf("ERROR: WriteFile failed\n");
	}

	return fRes;
}

// Function that will listen for serial data in a separate thread
// heavily based on code from:
// https://docs.microsoft.com/en-us/previous-versions/ms810467(v=msdn.10)?redirectedfrom=MSDN

/// <summary>
/// The "asynchronous" function that will run in the background listening for serial data
/// </summary>
/// <param name="args"> A pointer to arguements to pass into this multithread function </param>

void listen_serial(void* args)
{
	DWORD bytesRead;			// Nubmer of bytes read
	DWORD dwCommEvent;			// Event mask returned from "WaitCommEvent" will be stored here
	char readChar;				// The new byte will be stored in here

	HANDLE readFile = (HANDLE)args; // Not really necessary but makes things easier to understand
	BOOL fWaitingOnStat = FALSE;	// Waiting on status poll from serial port

	DWORD dwEventFlags = EV_RXCHAR; // New character event for serial

	// Apply event filter flags to the serial port
	BOOL setCommStatus = SetCommMask(readFile, dwEventFlags);

	// Attempt to apply event flags to serial port
	// (only listen for those events)
	if (!setCommStatus)
	{
		// Something went wrong applying event 
		printf("Something went wrong appling Event Flags for serial port");
		return;
	}

	for (; ; )
	{
		// Wait for a new character event on the serial port
		if (WaitCommEvent(readFile, &dwCommEvent, NULL))
		{
			// Prevent memory shenanigans with the callback linked list
			// which is accessed by both the main thread and the listen thread
			waitForMutext(&linkedListMutex);
			__try
			{
				do
				{
					if (ReadFile(readFile, &readChar, 1, &bytesRead, NULL))
					{
						// Run all callback functions
								// and pass in current byte
						CallbackFuncNode* node = callbackList.firstNode;
						while (node != NULL)
						{
							node->callback(readChar);
							node = node->nextNode;
						}
					}
					else
					{
						// An error occured in ReadFile operation
						// abort
						printf("Error: Something went wrong in listen thread with ReadFile operation\n");
						break;
					}
				} while (bytesRead > 0); // Read all bytes in buffer
			}
			__finally
			{
				ReleaseMutex(linkedListMutex);
			}
		}
		else
		{
			if (ERROR_IO_PENDING == GetLastError())
			{
				printf("I/O is pending (WaitCommEvent)...\n");
			}
			// Error in WaitCommEvent
			// abort
			printf("Error: Something went wrong in listen thread with WaitCommEvent\n");
			break;
		}
	}
}


/// <summary>
/// Sets up serial listening thread and mutex protection for the callback linked lists
/// </summary>
/// <param name="com_port">The name of the port to initialize async for</param>
/// <return>Returns TRUE if successful and FALSE if not</return>
BOOL initialize_async_serial(wchar_t* com_port)
{
	// Create mutex for linked list
	linkedListMutex = (HANDLE)(CreateMutexA(NULL, FALSE, NULL));

	// Begin the serial listen thread
	uintptr_t threadStatus = (_beginthread(listen_serial, 0, (void*)hComm));

	if (threadStatus == -1)
	{
		// Error starting the thread
		return FALSE;
	}
	else
	{
		// Save the handle so that the thread can be ended later
		listenThreadHandle = (HANDLE)threadStatus;
		return TRUE;
	}
}

/// <summary>
/// Initializes the program for synchronous serial communication on one port.
/// This should only be called once.
/// </summary>
/// <param name="com_port">The name of the port to connect to (ex. "COM7")</param>
/// <returns>Returns 1 if the the operation was a success and 0 if not</returns>
int initialize_serial(wchar_t* com_port)
{
	printf("\nSync Initializing Port: ");
	wprintf(com_port);
	printf("\n");

	printf("\n");
	printf("Sync Port File Name: ");
	wprintf(com_port);
	printf("\n");

	wchar_t portBase[20] = L"\\\\.\\";
	wcscat_s(portBase, 20, com_port);

	hComm = CreateFileW(portBase,
		GENERIC_WRITE | GENERIC_READ,
		0,				// do not share
		NULL,			// default secutiry 
		OPEN_EXISTING,	// opens existing file
		0,				// Synchronous I/O
		NULL);			// Null for COM devices

	if (hComm == INVALID_HANDLE_VALUE)
	{
		// error opening the port
		printf("ERROR: error opening port\n");
		return -1;
	}


	// Initiailzie the DCB structure which controls baud rate, parity, etc.
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB)); // Fills DCB memory block with 0s
	GetCommState(hComm, &dcb); // Initialize comm state
	dcb.BaudRate = 9600;	// Baud rate
	dcb.Parity = NOPARITY;			// No parity
	dcb.StopBits = ONESTOPBIT;		// One stop bit
	dcb.ByteSize = 8;

	// May not be necessary and varies from system to system
	dcb.fInX = FALSE;
	dcb.XonChar = 0x0D;
	dcb.XoffChar = 0x0B;

	BOOL fSuccess = SetCommState(hComm, &dcb);	// Apply USB settings
	if (!fSuccess)
	{
		printf("\n SetCommState failed with error %d, \n", GetLastError());
		return 0;
	}
	printf("baudrate: %d parity: %d Stop Bits: %d\n\n",
		dcb.BaudRate,
		dcb.Parity,
		dcb.StopBits);
	

	// Setup the serial port timeouts
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 100;


	if (!SetCommTimeouts(hComm, &timeouts))
	{
		printf("Error setting timeouts for the serial port");
	}

	return initialize_async_serial(com_port);
}

///<summary>
/// Connects to a USB port
/// and write serial data to that port from an ASCII file.
/// </summary>
/// <param name="ascii_file_name">The ascii file name</param>
/// <returns>Returns 1 if operation was a success and 0 if writing failed.</returns>
int write_ascii_file(char* ascii_file_name)
{
	FILE* file_to_read;
	fopen_s(&file_to_read, ascii_file_name, "r");
	if (file_to_read == NULL)
	{
		printf("Could not open file: %s", ascii_file_name);
		return -2;
	}
	char data_to_send[SERIAL_SEND_BUF_SIZE] = "";
	char c;
	int count = 0;
	while ((c = fgetc(file_to_read)) != EOF)
	{
		data_to_send[count] = c;

		// Increment the character point in the 
		count++;

		// If the "data_to_send" pointer has reached the end, of "data_to_send"
		// Write "data_to_send" to the serial and reset the "data_to_send_pointer"
		if (count > SERIAL_SEND_BUF_SIZE - 1)
		{
			if (write_buffer(data_to_send, SERIAL_SEND_BUF_SIZE) != 1)
			{
				return -1;
			}
			count = 0;
		}
	}
	fclose(file_to_read);	// Close the file
	return write_buffer(data_to_send, SERIAL_SEND_BUF_SIZE);
}


/// <summary>
/// Adds a callback function to a queue of callbacks that will be run when data enters the serial stream
/// 
/// <param name="callback">The function to run when data enters serial</param>
/// <return>Returns status of operation (true if success, false if not)</return>
/// </summary>
int addSerialListenCallback(void (*callback)(char))
{
	CallbackFuncNode* node = (CallbackFuncNode*)(malloc(sizeof(CallbackFuncNode)));
	node->callback = callback;
	node->prevNode = NULL;
	node->nextNode = NULL;
	pushCallbackNode(&callbackList, node, &linkedListMutex);

	return 1;
}

/// <summary>
/// Prints a list of DOS devices as COM ports
/// </summary>
void listSerialPorts()
{
	printf("The following ports were found:\n");
	int count = 0;
	// Query all COM ports
	for (int i = 0; i < 255; i++)
	{
		DWORD status;
		wchar_t port_path[5000]; // Buffer to store filepath to port
		char port_num_char[10];
		wchar_t* port_num;
		sprintf_s(port_num_char, 10, "%d", i + 1); // convert i to a string
		port_num = charToWchar(port_num_char);

		wchar_t com_port_name[20] = L"COM";
		wcscat_s(com_port_name, 20, port_num);

		status = QueryDosDeviceCustom(com_port_name, port_path, 5000);

		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			port_path[10000]; // Increase the port_path buffer size if necessary
			continue;
		}

		if (status != 0)
		{
			// Something was found
			wprintf(com_port_name);
			wprintf(L"\tDevice Path: %ls\n", port_path);
			count++;
		}
	}
	if (count == 0)
	{
		printf("No devices using COM ports were found\n");
	}
}

/// <summary>
/// Prompts the user to input a COM port name
/// </summary>
/// <returns>Returns the COM port the user inputted</returns>
wchar_t* promptCOMPort()
{
	char com_port_char[50];	// Char version of the com_port
	wchar_t* com_port;		//The com_port name that will acutally be used

	printf("Enter Serial COM Port: ");
	// Use newline as terminator which counts as another character
	fgets(com_port_char, 50, stdin);
	com_port_char[strlen(com_port_char) - 1] = '\0';	// Remove the "\n" near the end

	com_port = charToWchar(com_port_char);	// Convert the "com_port_char" into a wchar_t*

	return com_port;
}

#endif

// � 2022 Jackson Medina Mr.MiracleMan111@gmail.com