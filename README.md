# Serial Communication Interface
A USB Serial Communication Interface written for Robert Sturges. This interface system can write byte buffers to serial and read individual incoming serial bytes. The example below will request the user to specify a serial port to use and will write the message "*ABCDEFGHI*" to that serial port. Whenever a byte enters that serial port's stream, the callback function ``onSerialChar(char data)`` will run and print the incoming byte to the console.

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
	
	int write_string_status = write_buffer("ABCDEFGHI", strlen("ABCDEFGHI") + 1);	// Write to serial
	printf("write_buffer returned code: %d\n \n", write_string_status);		// Print status
  
  
	addSerialListenCallback(onSerialChar);  	// Add the "onSerialChar" function declared 
							// above to the list of serial callbacks

	printf("Press Enter to finish...");		// Halt program from finishing
	char buffer[1];
	fgets(buffer, 1, stdin);


	closeSerialListenThread();			// Close the thread listening for serial data 
							// (necessary for cleanup purposes)
	
	return 0;
}
```

| Function      |    Return Value  | Description |
| ------------- | ----------- | ----------- |
| ``listSerialPorts``   |   **void**        | Prints available serial port options to console.                                                           |
| ``pomptCOMPort``     |   **wchar_t\***   | Prompts the user to specify a serial port to use and returns the value the user inputted as a ``wchar_t*`` (ex. *COM7*). |
| ``initialize_serial (wchar_t* com_port`` | **int** | Initializes the serial port for read and write operations. Returns **-1** if the initialization failed. |
| ``write_buffer (char* lpBuf, DWORD dwToWrite)`` | **int** | Writes a character buffer ``lpBuf`` to the serial port that was previosuly initialized. The length of the character buffer ``lpBuf`` should be passed in to parameter ``dwToWrite``. Returns **-1** if the write oepration failed. |
| ``write_ascii_file (char* ascii_file_name)`` | **int** | Writes the contents of a text file located to serial stream. The location of the text filed is passed in through parameter ``ascii_file_name``. The function returns **-1** if the write operation fails. |
| ``addSerialListenCallback(void (*callback)(char))`` | **int** | Adds a function to the list of callback functions to be run when a new byte enters the serial stream. The parameter ``callback`` is a pointer to the callback function. Returns **-1** if the program is unable to add the callback function.
| ``closeSerialListenThread`` | **void** | Closes the thread listening for incoming serial data. |

## Limitations
The asynchronous listening functionality is not truly asynchronous. In this version of the *SerialInterface*, read and write operations block each meaning only one can run at any instance of time. Usually this is not an issue. However, if large buffers are written to serial all at once, the read operations will be blocked for extended periods of time. The reverse is also true, having large incoming buffers and slow callback functions will delay write operations to serial. 

To prevent read/write operations from blocking each other, try to break large buffers up into smaller buffers and write those smaller buffers to serial.
