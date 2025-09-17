#ifndef LRU_NODE_C
#define LRU_NODE_C

#include "MyDB_PageHandle.h"

using namespace std;

struct MyDB_LRUNode {
    MyDB_PageHandle pageHandle;
    MyDB_LRUNode *prev;
    MyDB_LRUNode *next;
};

MyDB_LRUNode * eject() {
    return nullptr;
}

#endif