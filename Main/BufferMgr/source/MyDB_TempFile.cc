#ifndef TEMP_FILE_C
#define TEMP_FILE_C

#include <fstream>
#include <vector>

#include "MyDB_TempFile.h"

using namespace std;

MyDB_TempFile :: MyDB_TempFile(size_t pageSize, string fileName) {
    this->pageSize = pageSize;
    this->totalSize = 0;
    tempFile.open(fileName, std::ios::out | std::ios::app);
}

MyDB_TempFile :: ~MyDB_TempFile() {
    // Delete all temp data
    for (size_t i = 0; i < totalSize; i++) {
        clearPage(i);
    }
    // Close the file
    tempFile.close();
}

// Write a page to temp memory when it is ejected from cache
void MyDB_TempFile :: writePage(void * buf, size_t pageNum) {
    // TODO
}

// Retrieve a page from temp memory when it is needed again
void MyDB_TempFile :: fetchPage(void * buf, size_t pageNum) {
    // TODO
}

// Get a free page number to write to
int MyDB_TempFile :: getFreePage() {
    if (this->freePages.size() > 0) {
        int pageNum = this->freePages.back();
        this->freePages.pop_back();
        return pageNum;
    } else {
        totalSize++;
        return totalSize - 1;
    }
}

// Clear a page from temp memory when it is no longer needed, freeing up its space
void MyDB_TempFile :: clearPage(size_t pageNum) {
    // TODO
}


#endif