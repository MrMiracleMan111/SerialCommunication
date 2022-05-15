#pragma once
#ifndef CALLBACK_LINKED_LIST_S
#define CALLBACK_LINKED_LIST_S
/*
* 
* A linked list for serial read callback functions
* of the "SerialInterface"
* 
* 
* A dummy node is placed at the beginning
*/

typedef struct CallbackFuncNode CallbackFuncNode;

struct CallbackFuncNode
{
	void (*callback)(char rChar); // Function pointer
	CallbackFuncNode* nextNode;
	CallbackFuncNode* prevNode;
};

typedef struct CallbackLinkedList CallbackLinkedList;

// The linked list "object" that you will interact with
struct CallbackLinkedList
{
	CallbackFuncNode* firstNode;
};

// Gets a node at a specified index of the linked list (N time complexity)
CallbackFuncNode* getCallbackNode(CallbackLinkedList* list, int index, HANDLE* mutex)
{
	if (mutex != NULL)
	{
		waitForMutext(mutex);
	}

	CallbackFuncNode* pointerNode = list->firstNode;

	for (int i = 0; i <= index; i++)
	{
		pointerNode = pointerNode->nextNode;
	}

	if (mutex != NULL)
	{
		ReleaseMutex(mutex);
	}

	return pointerNode;
}

// Adds a node to the end of the linked list (N time complexity)
BOOL pushCallbackNode(CallbackLinkedList* list, CallbackFuncNode* newNode, HANDLE* mutex)
{
	if (newNode == NULL || list == NULL)
	{
		printf("Your CallbackList and or new node to the list are NULL");
		return FALSE;
	}

	if (mutex != NULL)
	{
		waitForMutext(mutex);
	}

	if (list->firstNode == NULL)
	{
		list->firstNode = newNode;

		if (mutex != NULL)
		{
			ReleaseMutex(mutex);
		}
		else
		{
			printf("WARNING: Mutex was null for callback linked list");
		}

		return TRUE;
	}

	CallbackFuncNode* currentNode = list->firstNode;

	while (currentNode->nextNode != NULL)
	{
		currentNode = currentNode->prevNode;
	}

	currentNode->nextNode = newNode;
	newNode->prevNode = currentNode;

	if (mutex != NULL)
	{
		ReleaseMutex(mutex);
	}
	return TRUE;
}

// Removes a node at the specified index (2N time complexity)
BOOL removeCallbackNode(CallbackLinkedList* list, int index, HANDLE* mutex)
{
	if (mutex != NULL)
	{
		waitForMutext(mutex);
	}


	if (index == 0)
	{
		printf("Error: Do not remove dummy node at beginning of linked list (CallbackLinkedList.h)");
		return FALSE;
	}
	CallbackFuncNode* currentNode = list->firstNode;
	if (currentNode == NULL)
	{
		return FALSE;
	}

	for (int i = 0; i <= index; i++)
	{
		// Out of bounds check
		if (currentNode->nextNode == NULL)
		{
			printf("Error: out of bounds (CallbackLinkedList.h)");
			if (mutex != NULL)
			{
				ReleaseMutex(mutex);
			}
			return FALSE;
		}
		currentNode = currentNode->nextNode;
	}
	

	if (currentNode->prevNode != NULL)
	{
		currentNode->prevNode->nextNode = currentNode->nextNode;
	}

	if (currentNode->nextNode != NULL)
	{
		currentNode->nextNode->prevNode = currentNode->prevNode;
	}

	// Deallocate the memory of the current node
	free(currentNode);

	if (mutex != NULL)
	{
		ReleaseMutex(mutex);
	}

	return TRUE;
}

#endif CALLBACK_LINKED_LIST_S