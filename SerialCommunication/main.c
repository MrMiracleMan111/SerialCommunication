#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <strsafe.h>
#include <wchar.h>
#include "SerialInterface.h"

/*
* 
*	PORTION OF EXAMPLE C: LISTEN FOR SERIAL CHARACTERS
* 
*	The function below "onSerialChar" is an example of a serial callback function you would write.
*	This function will be run once for every new character in the serial stream. Each new character
*	will be passed into this function so it only takes one "char" as a parameter.
* 
*/

// Print the current serial character.
void onSerialChar(char data)
{
	printf("%c", data);
}

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
	* The "initialize_sync_serial" and "initialize_async_serial" methods should only be run once.
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
	int return_code = write_ascii_file("ASCII.txt");			// Writes all data in text file to serial	
	printf("write_ascii_file returned code: %d\n \n", return_code);

	/*
	* EXAMPLE C: LISTEN FOR SERIAL CHARACTERS
	* 
	* In this example, a callback function will be run everytime a new
	* character enters the serial buffer. The new character will be passed into
	* the callback function. The read char operation is synchronous, so readling large
	* volumes of data will prevent you from writing.
	* 
	* 
	* [LIMITATIONS]
	* The "asynchronous" functionality is not truly asynchronous. Currently, a READ operation cannot
	* take place during a WRITE operation. So, if you were to write a large buffer to serial,
	* the "asyc" serial callbacks would not be able to fire until you are done writing your buffer.
	* 
	* To circumvent this, break large buffers into smaller portions and send each portion separately.
	* 
	Contraversely, if your READ operation takes a long period of time, you will not be able to make any
	* WRITE operations to serial while waiting for READ to finish.
	*/

	// Add the "onSerialChar" function declared above to the list of
	// serial callbacks
	addSerialListenCallback(onSerialChar);

	// End the program after the users presses the enter key (this just keeps the program running)
	printf("Press Enter to finish...");
	char buffer[1];
	fgets(buffer, 1, stdin);

	// Close the thread listening for serial data (necessary for cleanup purposes)
	closeSerialListenThread();
	
	printf("Program Finished");
	return 0;
}

// © 2022 Jackson Medina Mr.MiracleMan111@gmail.com