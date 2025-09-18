
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include <iostream>
#include "MyDB_PageHandle.h"

using namespace std;

void *MyDB_PageHandleBase :: getBytes () {
	return nullptr;
}

void MyDB_PageHandleBase :: wroteBytes () {
}


MyDB_PageHandleBase :: MyDB_PageHandleBase () {
    this->refCount = 0;
    this->dirty = CLEAN;
    this->active = INACTIVE;
    this->pinned = UNPINNED;
    this->location.buf = nullptr;
    this->location.pageIndex = -1;
    this->location.table = nullptr;
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {

}

void MyDB_PageHandleBase :: writeBack() {
    // Write back to disk or temp if dirty
    if (this->dirty == DIRTY) {
        if (this->permanent == DISK) {
            // Write to disk
        } else {
            // Write to temp
        }
    }
}

void MyDB_PageHandleBase :: printHandle() {
    if (this->permanent == TEMP) {
        std::cout << "Temp Page " << this->location.pageIndex << std::endl;
    } else {
        std::cout << "Table: " << this->location.table->getName() << " " << this->location.pageIndex << std::endl;
    }
    std::cout << "ACTIVE: " << this->active << " PINNED: " << this->pinned << " DIRTY: " << this->dirty << std::endl << std::endl;
}


#endif

