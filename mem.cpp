#include <cstddef>
#include <cstring>
#include <cmath>
#include <iostream>

#include <sys/mman.h>

#include "freelist.h"

typedef char byte;

void* GetPage();
Header* GetBlock(int level);
void* MyMalloc(size_t size);
size_t Round(size_t size);
uint8_t LogSize(size_t size);
Header* CreateHeader(void* block, uint8_t size);

FreeList flists[9];

void* GetPage()
{
    void* addr = mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return addr;
}

Header* GetBlock(int level)
{
    /* 4 is the first level, so for 0-index array... */
    int adjusted_level = level - 4;

    /* Get pointer to head of list for this level */
    FreeList &listL = flists[adjusted_level];

    /* Get available block node header from list, or null if list is empty */
    Header* block = listL.popHead();

    /* No blocks of the needed size */
    if (block == nullptr) {

        /* Special case - need to map another page of memory from the OS */
        if (level == 12) { // BASE CASE of recursion
            void* page = GetPage();
            if ((uint64_t) page == -1) // have to cast here
                return nullptr;
            block = CreateHeader(page, 12);
        } else {
            /* No blocks of needed size, recursively call GetBlock to get a block of the next higher level */
            Header* nextLevelHigherBlock = GetBlock(level + 1);
            if (nextLevelHigherBlock == nullptr)
                return nullptr;

            // split higher block, assign one half to `block` and insert other half to free list
            size_t blockSizeInBytes = pow(2, nextLevelHigherBlock->size);
            byte* midAddr = ((byte*)nextLevelHigherBlock) + (blockSizeInBytes / 2);
            Header* leftBuddy = CreateHeader((void*) nextLevelHigherBlock, level);
            Header* rightBuddy = CreateHeader((void*) midAddr, level);
            block = leftBuddy;
            listL.append(rightBuddy);
        }
    }

    return block;
}

Header* CreateHeader(void* block, uint8_t size)
{
    Header* header = (Header*) block;
    header->size = size;
    header->link = nullptr;
    return (Header*) header;
}

void* MyMalloc(size_t size)
{
    size = Round(size); // nearest viable size
    uint8_t level = LogSize(size);
    Header* block = GetBlock(level); // pointer to header preceding actual memory block
    if (block == nullptr)
        return (void*) -1;
    return block + sizeof(Header); // return address pointing to first byte past header
}

uint8_t LogSize(size_t size)
{
    int targetlevel = 0;
    while (size >>= 1) ++targetlevel;
    return targetlevel;
}

size_t Round(size_t size)
{
    uint8_t lg = LogSize(size);

    bool numberIsPow2 = (size != 0) && ((size & (size - 1)) == 0);
    if (!numberIsPow2) {
        ++lg;
    }
    
    return pow(2, lg);
}

int main()
{
    Header* h1 = GetBlock(11);
    Header* h2 = GetBlock(11);
    printf("Address of h1: %x\n", h1);
    printf("Address of h2: %x\n", h2);
    printf("Offset (bytes): %d\n", h1 - h2);
}