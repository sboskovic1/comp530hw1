
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr tablePtr, long idx) {
    // Check if the page already exists
    if (this->table.find(tablePtr) != this->table.end()) {
        if (this->table[tablePtr].find(idx) != this->table[tablePtr].end()) {
            MyDB_PageHandle pageHandle = this->table[tablePtr][idx];
            pageHandle->refCount++;
            return pageHandle;
        }
    }
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    // If not, create a new page handle and add it to the table
    this->table[tablePtr][idx] = newPageHandle;
    newPageHandle->location.table = tablePtr;
    newPageHandle->location.pageIndex = idx;
    newPageHandle->refCount++;
    newPageHandle->permanent = DISK;
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    newPageHandle->refCount++;
    newPageHandle->permanent = TEMP;
    newPageHandle->location.tempFile = this->tempFile;
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr tablePtr, long idx) {
    // Don't forget case where it already exists in buffer
    if (this->table.find(tablePtr) != this->table.end()) {
        if (this->table[tablePtr].find(idx) != this->table[tablePtr].end()) {
            MyDB_PageHandle pageHandle = this->table[tablePtr][idx];
            pageHandle->pinned = PINNED;
            MyDB_LRUNode * node = findNode(pageHandle);
            if (node == nullptr) {
                void * buf = this->requestBufferSpace();
                pageHandle->location.buf = buf;
                pageHandle->active = ACTIVE;
                // Write to buffer with file IO
            } else { // Node already in LRU cache, remove it so it cannot be ejected
                node->eject();
            }
            pageHandle->refCount++;
            return pageHandle;
        }
    }
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    // If not, create a new page handle and add all necessary information
    this->table[tablePtr][idx] = newPageHandle;
    newPageHandle->location.table = tablePtr;
    newPageHandle->location.pageIndex = idx;
    newPageHandle->pinned = PINNED;
    newPageHandle->active = ACTIVE;
    newPageHandle->permanent = DISK;
    void * buf = this->requestBufferSpace();
    newPageHandle->location.buf = buf;
    newPageHandle->refCount++;
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    newPageHandle->pinned = PINNED;
    newPageHandle->active = ACTIVE;
    newPageHandle->permanent = TEMP;
    void * buf = this->requestBufferSpace();
    newPageHandle->location.buf = buf;
    newPageHandle->refCount++;
    return newPageHandle;	
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
    // Concern with this function mentioned in README
    // For now naive approaches is used where unpin counts as a use
    unpinMe->pinned = UNPINNED;
    MyDB_LRUNode * node = new MyDB_LRUNode(unpinMe);
    if (this->head == nullptr) {
        this->head = node;
        this->tail = node;
    } else { // Since we already have the page, don't need to worry about list being full
        node->next = this->head;
        this->head->prev = node;
        this->head = node;
    }
}

void * MyDB_BufferManager :: requestBufferSpace() {
    if (this->freePages.size() != 0) {
        void * buf = (char *) buffer + (freePages.back() * pageSize);
        freePages.pop_back();
        return buf;
    } else {
        MyDB_PageHandle * node = &tail->eject()->pageHandle;
        (*node)->writeBack();
        this->clear((*node)->location.buf);
        void * temp = (*node)->location.buf;
        (*node)->location.buf = nullptr;
        (*node)->active = INACTIVE;
        return temp;
    }
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string fileName) {
    this->pageSize = pageSize;
    this->numPages = numPages;
    this->tempFile = new MyDB_TempFile(pageSize, fileName);
    this->buffer = malloc(pageSize * numPages);
    this->head = nullptr;
    this->tail = nullptr;
    for (int i = numPages - 1; i >= 0; i--) {
        this->freePages.push_back(i); // Add all pages to free list
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    // Loop through LRU cache and write all dirty pages to disk or temp
    std::cout << "Destroying Buffer Manager" << std::endl;
    MyDB_LRUNode * curr = head;
    while (curr != nullptr) {
        curr->pageHandle->writeBack();
        curr = curr->next;
    }
    delete this->tempFile; // Takes care of all pages in tempFile
    free(this->buffer);
}

void MyDB_BufferManager :: clear (void * page) {
    // Clear page to be replaced in buffer
}

MyDB_LRUNode * MyDB_BufferManager :: findNode(MyDB_PageHandle pageHandle) {
    MyDB_LRUNode * curr = this->head;
    while (curr != nullptr) {
        if (curr->pageHandle == pageHandle) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

void MyDB_BufferManager :: printBuffer() {
    std::cout << "Printing Buffer State: " << std::endl << std::endl;
    std::cout << "Free Pages: " << this->numPages << std::endl;
    for (int f : freePages) {
        std::cout << f << " ";
    }
    std::cout << std::endl << "LRU Cache: " << std::endl;
    MyDB_LRUNode * curr = this->head;
    while (curr != nullptr) {
        MyDB_PageHandle pageHandle = curr->pageHandle;
        if (pageHandle->permanent == TEMP) {
            std::cout << "Temp Page " << pageHandle->location.pageIndex << std::endl;
        } else {
            std::cout << "Table: " << pageHandle->location.table->getName() << " " << pageHandle->location.pageIndex << std::endl;
        }
        std::cout << "ACTIVE: " << pageHandle->active << " PINNED: " << pageHandle->pinned << " DIRTY: " << pageHandle->dirty << std::endl << std::endl;
        curr = curr->next;
    }
    std::cout << std::endl << "End of Buffer State" << std::endl << std::endl;
}

	
#endif


