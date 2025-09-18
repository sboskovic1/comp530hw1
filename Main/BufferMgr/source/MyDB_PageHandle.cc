
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include <iostream>
#include "MyDB_PageHandle.h"

using namespace std;

void *MyDB_PageHandleBase :: getBytes () {
    if (this->active != ACTIVE || this->location.buf == nullptr) {
        // Need to load the page from disk or temp file
        this->location.buf = this->getBufferSpace();
        this->active = ACTIVE;
        if (this->permanent == DISK) {
            // Load from disk
            void * disk = (void *)&this->location.table->getStorageLoc();
            // TODO
        } else {
            if (this->location.pageIndex == -1) {
                this->location.pageIndex = this->location.tempFile->getFreePage();
            }
            // Load from temp file
            this->location.tempFile->fetchPage(this->location.buf, this->location.pageIndex);
        }
    }
	return this->location.buf;
}

void MyDB_PageHandleBase :: wroteBytes () {
    this->dirty = DIRTY;
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
    // TODO
    if (this->active == ACTIVE) {
        this->writeBack();
    }
    if (this->permanent == TEMP) {
        this->location.tempFile->clearPage(this->location.pageIndex);
    }
}

void MyDB_PageHandleBase :: writeBack() {
    // Write back to disk or temp if dirty
    if (this->dirty == DIRTY) {
        if (this->permanent == DISK) {
            // Write to disk
            // TODO
        } else {
            // Write to temp
            this->location.tempFile->writePage(this->location.buf, this->location.pageIndex);
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

