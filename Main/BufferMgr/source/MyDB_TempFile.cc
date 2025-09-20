#ifndef TEMP_FILE_C
#define TEMP_FILE_C

#include <fstream>
#include <vector>
#include <fcntl.h>     // open
#include <unistd.h>    // lseek, read, close
#include <sys/stat.h>  // file permissions

#include "MyDB_TempFile.h"
#include <iostream>

using namespace std;

MyDB_TempFile :: MyDB_TempFile(size_t pageSize, string fileName) {
    this->pageSize = pageSize;
    this->totalSize = 0;
    this->fileName = fileName;
    
    // Create the tempFile if it hasn't been created yet
    int fd = open(fileName.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        // std::cout << "File created successfully: " << fileName << std::endl;
        close(fd);
    } else {
        // std::cout << "File already exists " << fileName << std::endl;
    }
}

MyDB_TempFile :: ~MyDB_TempFile() {
    // Delete all temp data
    for (size_t i = 0; i < totalSize; i++) {
        clearPage(i);
    }
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
    // Write all 0's for this page
    int fd = open(this->fileName.c_str(), O_WRONLY | O_FSYNC);
    if (fd < 0) {
        perror("open failed");
        return;
    }
    
    off_t offset = pageNum * this->pageSize;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        perror("lseek failed");
        close(fd);
    }

    ssize_t totalWritten = 0;
    std::vector<char> zeros(pageSize, 0);  // buffer of pageSize zeros

    while (totalWritten < (ssize_t)pageSize) {
        ssize_t written = write(fd, zeros.data() + totalWritten, pageSize - totalWritten);
        if (written <= 0) {
            perror("write failed");
            break;
        }
        totalWritten += written;
    }

    close(fd);

    // Add pageNum into free pages for storage for later pages
    this->freePages.push_back(pageNum);
}


#endif