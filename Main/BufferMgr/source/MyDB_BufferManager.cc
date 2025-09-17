
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <unordered_map>
#include <vector>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr, long) {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr, long) {
    // Don't forget case where it already exists in buffer
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	return nullptr;		
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string fileName) {
    this->pageSize = pageSize;
    this->numPages = numPages;
    this->tempFile.open(fileName, std::ios::out | std::ios::app);
    this->buffer = malloc(pageSize * numPages);
    this->usedPages = 0;
    this->head = nullptr;
    this->tail = nullptr;
    for (size_t i = numPages - 1; i >= 0; i--) {
        this->freePages.push_back(i); // Add all pages to free list
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    // Loop through LRU cache and write all dirty pages to disk or temp
    tempFile.close(); // Takes care of all pages in tempFile
}

	
#endif


