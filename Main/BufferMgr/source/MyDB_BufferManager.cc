
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
    createDiskFile(tablePtr);
    MyDB_PageHandle pageHandle = make_shared<MyDB_PageHandleBase>();
    // Check if the page already exists
    if (this->table.find(tablePtr) != this->table.end()) {
        if (this->table[tablePtr].find(idx) != this->table[tablePtr].end()) {
            MyDB_Page * page = this->table[tablePtr][idx];
            page->refCount++;
            pageHandle->page = page;
            return pageHandle;
        }
    }
    // If not, create a new page handle and add it to the table
    MyDB_Page * page = buildPage(UNPINNED, DISK);
    pageHandle->page = page;
    page->location.table = tablePtr;
    page->location.pageIndex = idx;
    this->table[tablePtr][idx] = page;
    return pageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    MyDB_Page * newPage = buildPage(UNPINNED, TEMP);
    newPageHandle->page = newPage;
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr tablePtr, long idx) {
    createDiskFile(tablePtr);
    MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    // Check if the page already exists
    if (this->table.find(tablePtr) != this->table.end()) {
        if (this->table[tablePtr].find(idx) != this->table[tablePtr].end()) {
            MyDB_Page * page = this->table[tablePtr][idx];
            if (page->pinned != PINNED) {
                if (this->pinned == this->numPages) {
                    return nullptr; // All pages are pinned, cannot pin another
                } 
                // Remove the node from the LRU if it's there so it can't be ejected
                MyDB_LRUNode * node = findNode(page);
                if (node != nullptr) {
                    if (node == this->head) {
                        this->head = node->next;
                    }
                    if (node == this->tail) {
                        this->tail = node->prev;
                    }
                    node->eject();
                }
                this->pinned++;
            }
            newPageHandle->page = page;
            page->pinned = PINNED;
            page->refCount++;
            return newPageHandle;
        }
    }
    if (this->pinned == this->numPages) {
        return nullptr; // All pages are pinned, cannot pin another
    }
    // If not, create a new page handle and add all necessary information
    MyDB_Page * page = buildPage(PINNED, DISK);
    newPageHandle->page = page;
    page->location.table = tablePtr;
    page->location.pageIndex = idx;
    this->table[tablePtr][idx] = page;
    return newPageHandle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
    if (this->pinned == this->numPages) {
        return nullptr; // All pages are pinned, cannot pin another
    }
	MyDB_PageHandle newPageHandle = make_shared<MyDB_PageHandleBase>();
    MyDB_Page * page = buildPage(PINNED, TEMP);
    newPageHandle->page = page;
    return newPageHandle;
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
    // Concern with this function mentioned in README
    // For now naive approaches is used where unpin counts as a use
    unpinMe->page->pinned = UNPINNED;
    MyDB_LRUNode * node = new MyDB_LRUNode(unpinMe->page);
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
        tail = tail->prev;
        MyDB_Page * node = tail->next->eject()->page;
        node->writeBack();
        this->clear(node->location.buf);
        void * temp = node->location.buf;
        node->location.buf = nullptr;
        node->active = INACTIVE;
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
        curr->page->writeBack();
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
MyDB_LRUNode * MyDB_BufferManager :: findNode(MyDB_Page * page) {
    MyDB_LRUNode * curr = this->head;
    if (curr == nullptr) {
        return nullptr;
    }
    while (curr != nullptr) {
        if (curr->page == page) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

void MyDB_BufferManager :: removeFromLRU(MyDB_Page * page) {
    MyDB_LRUNode * node = findNode(page);
    if (node != nullptr) {
        if (node == this->head) {
            this->head = node->next;
        }
        if (node == this->tail) {
            this->tail = node->prev;
        }
        node->eject();
    }
}

void MyDB_BufferManager :: push(MyDB_Page * page) {
    MyDB_LRUNode * node = this->findNode(page);
    if (node != nullptr) { // Node already in LRU cache
        if (node == this->head) {
            return;
        }
        if (node == this->tail) {
            this->tail = node->prev;
        }
        node->eject();
        node->next = this->head;
        if (this->head != nullptr) {
            this->head->prev = node;
        }
        this->head = node;
        node->prev = nullptr;
    } else { // Node not in LRU cache, make a new node
        node = new MyDB_LRUNode(page);
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
    }
}

void MyDB_BufferManager :: returnPage(void * buf) {
    this->freePages.push_back(((char *)buf - (char *)this->buffer) / this->pageSize);
}

MyDB_Page * MyDB_BufferManager :: buildPage(int pinned, int permanent) {
    MyDB_Page * page = new MyDB_Page();
    page->pinned = pinned;
    page->active = INACTIVE;
    page->permanent = permanent;
    page->pageSize = this->pageSize;
    page->getBufferSpace = [this]() -> void* {
        return this->requestBufferSpace();
    };
    page->giveBack = [this](void * buf) {
        this->returnPage(buf);
    };
    page->pushNode = [this, page]() {
        this->push(page);
    };
    page->removeFromTable = [this](MyDB_TablePtr tablePtr, long idx) {
        this->removeFromTable(tablePtr, idx);
    };
    page->removeFromLRU = [this](MyDB_Page * page) {
        this->removeFromLRU(page);
    };
    page->decrementPinned = [this]() {
        this->pinned--;
    };
    page->refCount++;
    if (pinned == PINNED) {
        this->pinned++;
    }
    if (page->permanent == TEMP) {
        page->location.tempFile = this->tempFile;
        page->location.pageIndex = this->tempFile->getFreePage();
    }
    return page;
}

void MyDB_BufferManager :: removeFromTable(MyDB_TablePtr tablePtr, long idx) {
    auto it = table.find(tablePtr);
    if (it != table.end()) {
        it->second.erase(idx);
        if (it->second.empty()) {
            table.erase(it);
        }
    }
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
        MyDB_Page * page = curr->page;
        if (page->permanent == TEMP) {
            std::cout << "Temp Page " << page->location.pageIndex << std::endl;
        } else {
            std::cout << "Table: " << page->location.table->getName() << " " << page->location.pageIndex << std::endl;
        }
        std::cout << "ACTIVE: " << page->active << " PINNED: " << page->pinned << " DIRTY: " << page->dirty << std::endl << std::endl;
        curr = curr->next;
    }
    std::cout << std::endl << "End of Buffer State" << std::endl << std::endl;
}

#endif


