
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include <iostream>
#include "MyDB_PageHandle.h"
#include <fcntl.h>     // open
#include <unistd.h>    // lseek, read, close
#include <sys/stat.h>  // file permissions
#include <cstring>     // memset, optional

using namespace std;

void *MyDB_PageHandleBase :: getBytes () {
    if (this->active != ACTIVE || this->location.buf == nullptr) {
        // Need to load the page from disk or temp file
        this->location.buf = this->getBufferSpace();
        this->active = ACTIVE;
        if (this->permanent == DISK) {
            std::string fileName = this->location.table->getStorageLoc();
            long idx = this->location.pageIndex;

            int fd = open(fileName.c_str(), O_RDONLY);
            if (fd < 0) {
                perror("open failed");
                return nullptr;
            }

            // Seek to the correct page offset
            off_t offset = idx * this->pageSize;
            if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
                perror("lseek failed");
                close(fd);
                return nullptr;
            }

            // Read page into buffer
            ssize_t bytesRead = read(fd, this->location.buf, this->pageSize);
            if (bytesRead < 0) {
                perror("read failed");
                close(fd);
                return nullptr;
            }

            // If the page in the didn't have pageSize bytes, fill the rest of the buffer page with 0's
            if ((size_t)bytesRead < this->pageSize) {
                std::memset((char*)this->location.buf + bytesRead, 0, this->pageSize - bytesRead);
            }

            close(fd);
        } else {
            if (this->location.pageIndex == -1) {
                this->location.pageIndex = this->location.tempFile->getFreePage();
            }
            // Load from temp file
            this->location.tempFile->fetchPage(this->location.buf, this->location.pageIndex);
        }
    }
    this->pushNode(); // Push to front of LRU
	return this->location.buf;
}

void MyDB_PageHandleBase :: wroteBytes () {
    this->dirty = DIRTY;
    // TODO: We shouldn't push the node to the front of the LRU if it is a pinned page bc it exists outside of the LRU
    this->pushNode(); // Push to front of LRU
}


MyDB_PageHandleBase :: MyDB_PageHandleBase () {
    this->refCount = 0;
    this->dirty = CLEAN;
    this->active = INACTIVE;
    this->pinned = UNPINNED;
    this->location.buf = nullptr;
    this->location.pageIndex = -1;
    this->location.table = nullptr;
    this->pageSize = 64; // Default page size
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
            std::string fileName = this->location.table->getStorageLoc();
            long idx = this->location.pageIndex;

            int fd = open(fileName.c_str(), O_WRONLY | O_FSYNC);
            if (fd < 0) {
                perror("open failed");
                return;
            }
            
            // Seek to the correct page offset
            off_t offset = idx * this->pageSize;
            if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
                perror("lseek failed");
                close(fd);
            }

            // Write to the file
            ssize_t totalWritten = 0;
            const char *data = (const char *)this->location.buf;
            while (totalWritten < (ssize_t)this->pageSize) {
                ssize_t written = write(fd, data + totalWritten, pageSize - totalWritten);
                if (written <= 0) {
                    perror("write failed");
                    break;
                }
                totalWritten += written;
            }

            close(fd);

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

