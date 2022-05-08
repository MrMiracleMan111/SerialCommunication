# SerialCommunication
A USB Serial Communication Interface written for Robert Sturges. Can write messages to a serial buffer and read bytes in serial.

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <strsafe.h>
#include <wchar.h>
#include "SerialInterface.h"

void onSerialChar(char data)		// Prints a char when a char enters the serial

{
	printf("%c", data);
}

int main()
{
	listSerialPorts();		// Lists available ports to write to
	wchar_t* com_port;		// The com_port name that will acutally be used
	com_port = promptCOMPort();	// Prompt the user for the COM port the want to connect to.
					// One example of a user input is "COM7"

	initialize_serial(com_port); // Setup the serial interface
	
  	// Example, writing string to serial
	int write_string_status = write_buffer("ABCDEFGHI", strlen("ABCDEFGHI") + 1);	// Write string to COM serial
	printf("write_buffer returned code: %d\n \n", write_string_status);		// Print write operation status
  
  
	addSerialListenCallback(onSerialChar);  	// Add the "onSerialChar" function declared above to the list of serial callbacks

	printf("Press Enter to finish...");		// Halt program from finishing
	char buffer[1];
	fgets(buffer, 1, stdin);


	closeSerialListenThread();			// Close the thread listening for serial data (necessary for cleanup purposes)
	
	return 0;
}
```
