
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_LRUNode.cc"
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

std::ofstream tempFile;
size_t pSize;
size_t nPages;
size_t usedPages;
void *buffer;
MyDB_LRUNode *head;
MyDB_LRUNode *tail;
unordered_map<MyDB_TablePtr, unordered_map<long, MyDB_PageHandle>> tables;
vector<int> free_pages;
unordered_map<MyDB_LRUNode, long> location_in_memory;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr, long) {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr, long) {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	return nullptr;		
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string fileName) {
    pSize = pageSize;
    nPages = numPages;
    tempFile.open(fileName, std::ios::out | std::ios::app);
    buffer = malloc(pageSize * numPages);
    usedPages = 0;
    head = nullptr;
    tail = nullptr;
    for (size_t i = numPages - 1; i >= 0; i--) {
        free_pages.push_back(i); // Add all pages to free list
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    // Loop through LRU cache and write all dirty pages to disk or temp
    tempFile.close();
}
	
#endif


