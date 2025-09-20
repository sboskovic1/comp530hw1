#ifndef LRU_NODE_H
#define LRU_NODE_H

#include "MyDB_Page.h"

using namespace std;
class  MyDB_LRUNode {

    public:

        MyDB_Page *page;
        MyDB_LRUNode *prev;
        MyDB_LRUNode *next;

        MyDB_LRUNode(MyDB_Page *page);

        MyDB_LRUNode * eject();

};

#endif