#ifndef LRU_NODE_C
#define LRU_NODE_C

#include "MyDB_LRUNode.h"

using namespace std;

MyDB_LRUNode :: MyDB_LRUNode(MyDB_PageHandle pageHandle) {
    this->pageHandle = pageHandle;
    this->prev = nullptr;
    this->next = nullptr;
}

MyDB_LRUNode * MyDB_LRUNode :: eject() {
    // TODO
    return nullptr;
}

#endif