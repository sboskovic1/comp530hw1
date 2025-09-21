#ifndef TEMP_FILE_H
#define TEMP_FILE_H

#include <fstream>
#include <vector>

using namespace std;

class MyDB_TempFile {
   
    size_t pageSize;
    size_t totalSize;
    vector<int> freePages;

    public:
        string fileName;
        
        MyDB_TempFile(size_t pageSize, string fileName);

        ~MyDB_TempFile();

        MyDB_TempFile() = default;

        // Get a free page number to write to
        int getFreePage();

        // Clear a page from temp memory when it is no longer needed, freeing up its space
        void clearPage(size_t pageNum);

};

#endif