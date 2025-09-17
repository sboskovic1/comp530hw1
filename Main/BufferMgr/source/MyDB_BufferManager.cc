
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_LRUNode.cc"
#include <string>
#include <unordered_map>

using namespace std;

std::ofstream tempFile;
size_t pSize;
size_t nPages;
void *buffer;
// std::unordered_map<>;


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
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    // Write to tempfile before closing
    tempFile.close();
}
	
#endif


