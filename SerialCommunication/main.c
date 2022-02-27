#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <strsafe.h>
#include <wchar.h>
#include "SerialInterface.h"

int main()
{
	listSerialPorts();			// Lists available ports to write to
	wchar_t* com_port;			// The com_port name that will acutally be used
	com_port = promptCOMPort();	// Prompt the user for the COM port the want to connect to.
								// One example of a user input is "COM7"

	/*
	* SETUP: 
	* 
	* This code is necessary to setup the serial communication to the specified port.
	* The "initialize_serial" method should only be run once.
	* 
	*/
	initialize_serial(com_port);

	/*
	* EXAMPLE A: WRITE SINGLE STRING TO SERIAL
	* 
	* In this example, a string of ascii text will be written to the serial port
	*/
	int write_string_status = write_buffer("ABCDEFGHI", strlen("ABCDEFGHI") + 1);	// Write string to COM serial
	printf("write_buffer returned code: %d\n \n", write_string_status);

	/*
	* EXAMPLE B: WRITE TEXT FILE TO SERIAL
	* 
	* In this example, the contents of an ASCII text file will be written to the serial port
	*/
	int return_code = write_ascii_file(com_port, "ASCII.txt");			// Writes all data in text file to serial	
	printf("write_ascii_file returned code: %d\n \n", return_code);

	return 0;
}

// © 2022 Jackson Medina Mr.MiracleMan111@gmail.com