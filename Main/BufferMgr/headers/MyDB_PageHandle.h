
#ifndef PAGE_HANDLE_H
#define PAGE_HANDLE_H

#include <memory>
#include <functional>
#include "MyDB_Table.h"
#include "MyDB_TempFile.h"


// page handles are basically smart pointers
using namespace std;
class MyDB_PageHandleBase;
typedef shared_ptr <MyDB_PageHandleBase> MyDB_PageHandle;

typedef struct {
    MyDB_TablePtr table;
    MyDB_TempFile * tempFile;
    long pageIndex;
    void * buf;
} Location;

class MyDB_PageHandleBase {

#define INACTIVE 0
#define ACTIVE 1

#define TEMP 0
#define DISK 1

#define UNPINNED 0
#define PINNED 1

#define CLEAN 0
#define DIRTY 1

public:

    int refCount;

    int active; // active/inactive
    int pinned; // pinned/unpinned
    int permanent; // temp/disk
    int dirty; // clean/dirty

    std::function<void*()> getBufferSpace; // Passed down from buffer manager to request buffer space
    std::function<void()> pushNode; // Passed down from buffer manager to push node to front of LRU

    Location location; // location of page

	// THESE METHODS MUST BE IMPLEMENTED WITHOUT CHANGING THE DEFINITION

	// access the raw bytes in this page... if the page is not currently
	// in the buffer, then the contents of the page are loaded from 
	// secondary storage. 
	void *getBytes ();

	// let the page know that we have written to the bytes.  Must always
	// be called once the page's bytes have been written.  If this is not
	// called, then the page will never be marked as dirty, and the page
	// will never be written to disk. 
	void wroteBytes ();

	// There are no more references to the handle when this is called...
	// this should decrmeent a reference count to the number of handles
	// to the particular page that it references.  If the number of 
	// references to a pinned page goes down to zero, then the page should
	// become unpinned.  
	~MyDB_PageHandleBase ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS

    MyDB_PageHandleBase ();

    void writeBack();

    void printHandle();

private:

	// YOUR CODE HERE
};

#endif

