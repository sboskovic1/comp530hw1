#ifndef LRU_NODE_C
#define LRU_NODE_C

#include "MyDB_LRUNode.h"
#include <iostream>

using namespace std;

MyDB_LRUNode :: MyDB_LRUNode(MyDB_Page *page) {
    this->page = page;
    this->prev = nullptr;
    this->next = nullptr;
}

MyDB_LRUNode * MyDB_LRUNode :: eject() {
    if (this->prev != nullptr) {
        this->prev->next = this->next;
    }
    if (this->next != nullptr) {
        this->next->prev = this->prev;
    }
    this->prev = nullptr;
    this->next = nullptr;
    return this;
}

#endif