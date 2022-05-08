#pragma once
#ifndef MUTEX_HELPER
#define MUTEX_HELPER

/// <summary>
///	Waits for the "mutex" to be freed, then retrieves it.
/// </summary>
/// <param name="mutex">The mutex to be retrieved</param>
/// <return> Returns status of wait operation (TRUE if mutex was retrieved, FALSE if the operation was abandoned</return>
BOOL waitForMutext(HANDLE* mutex)
{
	// A byte has been read; process it.
				// Wait for mutex to be freed
	DWORD dwWaitResult = WaitForSingleObject(mutex, INFINITE);

	switch (dwWaitResult)
	{
		// This thread got ownership of the mutex
	case WAIT_OBJECT_0:
		return TRUE;
		// This thread got ownership of abandoned mutex
	case WAIT_ABANDONED:
		printf("Mutex was abandoned");
		return FALSE;
	}
}
#endif // !MUTEX_HELPER
