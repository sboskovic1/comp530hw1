#ifndef TEMP_FILE_H
#define TEMP_FILE_H

#include <fstream>
#include <vector>

using namespace std;

class MyDB_TempFile {
    std::ofstream tempFile;
    size_t pageSize;
    size_t totalSize;
    vector<int> freePages;

    public:
        MyDB_TempFile(size_t pageSize, string fileName);

        ~MyDB_TempFile();

        // Write a page to temp memory when it is ejected from cache
        void writePage(void * buf, size_t pageNum);

        // Retrieve a page from temp memory when it is needed again
        void fetchPage(void * buf, size_t pageNum);

        // Get a free page number to write to
        int getFreePage();

        // Clear a page from temp memory when it is no longer needed, freeing up its space
        void clearPage(size_t pageNum);


};






#endif