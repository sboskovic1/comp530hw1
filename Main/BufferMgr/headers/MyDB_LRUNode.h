#ifndef LRU_NODE_H
#define LRU_NODE_H

#include "MyDB_PageHandle.h"

using namespace std;
class  MyDB_LRUNode {

    public:

        MyDB_PageHandle pageHandle;
        MyDB_LRUNode *prev;
        MyDB_LRUNode *next;
        
        MyDB_LRUNode(MyDB_PageHandle pageHandle);

        MyDB_LRUNode * eject();

};

#endif