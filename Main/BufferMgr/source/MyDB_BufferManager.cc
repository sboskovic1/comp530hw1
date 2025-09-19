
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <fcntl.h>      // open, O_* flags
#include <unistd.h>     // close
#include <sys/stat.h>   // S_IRUSR, S_IWUSR

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
    newPageHandle->pushNode = [this, newPageHandle]() {
        this->push(newPageHandle);
    };
    newPageHandle->location.table = tablePtr;
    newPageHandle->location.pageIndex = idx;
    newPageHandle->refCount++;
    newPageHandle->permanent = DISK;
    newPageHandle->getBufferSpace = [this]() -> void* {
        return this->requestBufferSpace();
    };
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    newPageHandle->refCount++;
    newPageHandle->permanent = TEMP;
    newPageHandle->location.tempFile = this->tempFile;
    newPageHandle->pushNode = [this, newPageHandle]() {
        this->push(newPageHandle);
    };
    newPageHandle->getBufferSpace = [this]() -> void* {
        return this->requestBufferSpace();
    };
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr tablePtr, long idx) {

    // TOOD: Don't forget case where it already exists in buffer

    if (this->table.find(tablePtr) != this->table.end()) {
        if (this->table[tablePtr].find(idx) != this->table[tablePtr].end()) {
            MyDB_PageHandle pageHandle = this->table[tablePtr][idx];
            if (pageHandle->pinned != PINNED) {
                if (this->pinned == this->numPages) {
                    return nullptr; // All pages are pinned, cannot pin another
                } 
               this->pinned++;
            }
            pageHandle->pinned = PINNED;
            MyDB_LRUNode * node = findNode(pageHandle);
            if (node == nullptr) {
                void * buf = this->requestBufferSpace();
                pageHandle->location.buf = buf;
                pageHandle->active = ACTIVE;
                // TODO
                // Write to buffer with file IO

            } else { // Node already in LRU cache, remove it so it cannot be ejected
                node->eject();
            }
            pageHandle->refCount++;
            return pageHandle;
        }
    }
    if (this->pinned == this->numPages) {
        return nullptr; // All pages are pinned, cannot pin another
    }
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    // If not, create a new page handle and add all necessary information
    this->table[tablePtr][idx] = newPageHandle;
    newPageHandle->location.table = tablePtr;
    newPageHandle->location.pageIndex = idx;
    newPageHandle->pinned = PINNED;
    newPageHandle->active = INACTIVE;
    newPageHandle->permanent = DISK;
    newPageHandle->refCount++;
    newPageHandle->getBufferSpace = [this]() -> void* {
        return this->requestBufferSpace();
    };
    newPageHandle->pushNode = [this, newPageHandle]() {
        this->push(newPageHandle);
    };
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
    if (this->pinned == this->numPages) {
        return nullptr; // All pages are pinned, cannot pin another
    }
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    newPageHandle->pinned = PINNED;
    newPageHandle->active = INACTIVE;
    newPageHandle->permanent = TEMP;
    newPageHandle->getBufferSpace = [this]() -> void* {
        return this->requestBufferSpace();
    };
    newPageHandle->pushNode = [this, newPageHandle]() {
        this->push(newPageHandle);
    };
    newPageHandle->refCount++;
    pinned++;
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
    pinned--;
}

void * MyDB_BufferManager :: requestBufferSpace() { // Notably also writes whatever is ejected back to memory
    if (this->freePages.size() != 0) {
        void * buf = (char *) buffer + (freePages.back() * pageSize);
        freePages.pop_back();
        return buf;
    } else {
        // Eject the most recent page
        if (this->pinned == this->numPages) {
            return nullptr; // All pages are pinned, cannot eject any
        }
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
    this->pinned = 0;
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
    // TODO
    // Clear page to be replaced in buffer
}

// Should we implement a hashtable for O(1) lookup to find a pageHandle? I know there was an 
// issue with making a c++ hashtable. The big test case is just a buffer size of 16 tho so maybe
// it won't matter
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

void MyDB_BufferManager :: push(MyDB_PageHandle pageHandle) {
    MyDB_LRUNode * node = this->findNode(pageHandle);
    if (node != nullptr) { // Node already in LRU cache
        node->eject();
        node->next = this->head;
        if (this->head != nullptr) {
            this->head->prev = node;
        }
        this->head = node;
        node->prev = nullptr;
    } else { // Node not in LRU cache, make a new node
        node = new MyDB_LRUNode(pageHandle);
        if (this->head == nullptr) { // There will always be space since we already gave this node buffer space and ejected the tail
            this->head = node;
            this->tail = node;
        } else {
            node->next = this->head;
            this->head->prev = node;
            this->head = node;
        }
        node->prev = nullptr;
    }
}

void MyDB_BufferManager :: createDiskFile(MyDB_TablePtr whichTable) {
    const char * filename = whichTable->getStorageLoc().c_str();
    // O_CREAT | O_EXCL → create only if file doesn’t exist; otherwise open fails.
    // O_RDWR → open for reading and writing.
    // O_FSYNC → force writes to disk immediately.
    // S_IRUSR | S_IWUSR → owner can read/write (needed with O_CREAT
    int fd = open(filename, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        std::cout << "File created successfully: " << filename << std::endl;
        close(fd);
    } else {
        std::cout << "File already exists " << filename << std::endl;
    }
}

#endif


