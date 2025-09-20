
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
    return this->page->getBytes();
}

void MyDB_PageHandleBase :: wroteBytes () {
    this->page->wroteBytes();
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    this->page->refCount--;
    if (this->page->refCount == 0) {
        delete this->page;
    }
}

MyDB_PageHandleBase :: MyDB_PageHandleBase () {
    this->page = nullptr;
}


#endif

