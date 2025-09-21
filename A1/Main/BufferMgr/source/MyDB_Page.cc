
#ifndef PAGE_C
#define PAGE_C

#include <memory>
#include <iostream>
#include "MyDB_PageHandle.h"
#include <fcntl.h>     // open
#include <unistd.h>    // lseek, read, close
#include <sys/stat.h>  // file permissions
#include <cstring>     // memset, optional

using namespace std;

void *MyDB_Page :: getBytes () {
    if (this->active != ACTIVE || this->location.buf == nullptr) {
        // Need to load the page from disk or temp file
        this->location.buf = this->getBufferSpace();
        this->active = ACTIVE;
        readBytesIntoBuf();
    }
    if (this->pinned == UNPINNED) {
        this->pushNode(); // Push to front of LRU
    }
	return this->location.buf;
}

void MyDB_Page :: wroteBytes () {
    if (this->active != ACTIVE || this->location.buf == nullptr) {
        std::cout << "Error: wroteBytes called on inactive page" << std::endl;
        return;
    }
    this->dirty = DIRTY;
    //shouldn't push the node if it is a pinned page bc it exists outside of the LRU
    if (this->pinned == UNPINNED) {
        this->pushNode(); // Push to front of LRU
    }
}

MyDB_Page :: MyDB_Page () {
    this->refCount = 0;
    this->dirty = CLEAN;
    this->active = INACTIVE;
    this->pinned = UNPINNED;
    this->location.buf = nullptr;
    this->location.pageIndex = -1;
    this->location.table = nullptr;
}

MyDB_Page :: ~MyDB_Page () {
    // If the page is currently pinned, unpin it and add it to the LRU
    if (this->refCount == 0) {
        this->refCount = -1;
        if (this->pinned == PINNED) {
            this->decrementPinned();
        } else {
            this->removeFromLRU(this);
        }
        if (this->active == ACTIVE) {

            this->giveBack(this->location.buf);
            this->writeBack();
        }
        if (this->permanent == TEMP) {
            this->location.tempFile->clearPage(this->location.pageIndex);
        } else {
            this->removeFromTable(this->location.table, this->location.pageIndex);
        }
    }
}

void MyDB_Page :: readBytesIntoBuf() {
    long idx = this->location.pageIndex;
    std::string fileName;
    if (this->permanent == DISK) {
        fileName = this->location.table->getStorageLoc();
    } else {
        fileName = this->location.tempFile->fileName;
    }

    // Open read/write so we can extend if needed
    int fd = open(fileName.c_str(), O_RDWR | O_FSYNC);
    if (fd < 0) {
        perror("open failed");
        return;
    }

    off_t offset = idx * this->pageSize;

    // Ensure file is large enough for this page:
    // Move to last byte of the page and write a '\0'
    if (lseek(fd, offset + this->pageSize - 1, SEEK_SET) == (off_t)-1) {
        perror("lseek failed");
        close(fd);
        return;
    }

    if (write(fd, "\0", 1) != 1) {
        perror("write to extend file failed");
        close(fd);
        return;
    }

    // Now seek back to the start of the page
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("lseek failed");
        close(fd);
        return;
    }

    // Read page into buffer
    ssize_t bytesRead = read(fd, this->location.buf, this->pageSize);
    if (bytesRead < 0) {
        perror("read failed");
        close(fd);
        return;
    }

    close(fd);
}

void MyDB_Page :: writeBack() {
    // This should never happen (it does in the tests though)
    // if (this->pinned == PINNED) {
    //     perror("page is pinned, cannot write to disk");
    //     return;
    // }

    if (this->dirty == CLEAN) {
        return;
    }

    long idx = this->location.pageIndex;
    string fileName = "";
    if (this->permanent == DISK) {
        fileName = this->location.table->getStorageLoc();
    } else {
        fileName = this->location.tempFile->fileName;
    }

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
}

void MyDB_Page :: printHandle() {
    if (this->permanent == TEMP) {
        std::cout << "Temp Page " << this->location.pageIndex << std::endl;
    } else {
        std::cout << "Table: " << this->location.table->getName() << " " << this->location.pageIndex << std::endl;
    }
    std::cout << "ACTIVE: " << this->active << " PINNED: " << this->pinned << " DIRTY: " << this->dirty << std::endl << std::endl;
}


#endif

