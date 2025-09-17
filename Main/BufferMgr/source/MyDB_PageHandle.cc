
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

int refCount = 0;

void *MyDB_PageHandleBase :: getBytes () {
	return nullptr;
}

void MyDB_PageHandleBase :: wroteBytes () {
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
}

void MyDB_PageHandleBase :: addRef () {
	refCount++;
}

void MyDB_PageHandleBase :: release () {
	refCount--;
	if (refCount == 0) {
		delete this;
	}
}

#endif

