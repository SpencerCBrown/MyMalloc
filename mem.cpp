#include <cstddef>
#include <cstring>
#include <cmath>
#include <iostream>
#include <vector>

#include <sys/mman.h>

#include "freelist.h"

typedef char byte;

void* GetPage();
Header* GetBlock(int level);
void* MyMalloc(size_t size);
size_t Round(size_t size);
uint8_t LogSize(size_t size);
Header* CreateHeader(void* block, uint8_t size);
void MyFree(void* pointer);
void Coalesce(Header* header);

FreeList flists[9];

void * operator new(size_t size)
{
    std::cout << "New operator overloading " << std::endl; 
    void * p = MyMalloc(size); 
    if (p == (void*) -1)
        return nullptr;
    return p; 
} 
  
void operator delete(void * p) 
{ 
    std::cout << "Delete operator overloading " << std::endl; 
    MyFree(p);
} 

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

/* Create a struct at the start of the block */
Header* CreateHeader(void* block, uint8_t size)
{
    Header* header = (Header*) block;
    header->size = size;
    header->link = nullptr;
    header->free = true;
    return (Header*) header;
}

void* MyMalloc(size_t size)
{
    size = Round(size + 16); // nearest viable size

    // temporary
    if (size > 4096)
        return (void*) -1;

    uint8_t level = LogSize(size);
    Header* block = GetBlock(level); // pointer to header preceding actual memory block
    block->free = false;
    if (block == nullptr)
        return (void*) -1;
    
    byte* headeraddr = (byte*) block;
    void* blockaddr = (headeraddr + sizeof(Header));
    return (void*)blockaddr; // return address pointing to first byte past header
}

void MyFree(void* pointer)
{
    // get header prepending memory at pointer
    byte* address = (byte*) pointer;
    Header* header = (Header*) (address - sizeof(Header));

    // try to merge with buddy and/or return to free list
    Coalesce(header);
}

void Coalesce(Header* header)
{
    /* If a buddy block of the same size as the current block exists and is free, merge the two blocks */
    FreeList &list = flists[header->size - 4];

    /* If 4096B size block, there is no buddy */
    if (header->size == 12) {
        list.append(header);
        return;
    }

    Header* buddy = (Header*) (( (uint64_t) header ) ^ (1 << header->size) );
    if (buddy->size == header->size && buddy->free) {

        /* remove buddy from free list */
        list.remove(buddy);

        /* Create new header for newly coalesced block */
        Header* firstHeader = (header < buddy) ? header : buddy;
        Header* coalescedBlock = CreateHeader(firstHeader, ++(header->size));

        /* Make recursive call to try to coalesce the newly created block with _IT's_ buddy */
        Coalesce(coalescedBlock);
    } else {
        header->free = true;
        list.append(header);
    }
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

#include <array>

int main()
{
    // byte* data3 = (byte*) MyMalloc(3000);
    // MyFree(data3);
    // byte* data = (byte*) MyMalloc(3000);
    // byte* data2 = (byte*) MyMalloc(1);
    // MyFree(data);
    // MyFree(data2);
    // printf("%x\n", data);
    // printf("%x\n", data2);
    // printf("%x\n", data3);


    // int* nums = new int[5];
    // printf("%x\n", nums);
    // delete[] nums;
    // int* nums2 = new int[5];
    // printf("%x\n", nums2);

    // std::vector<std::string> *vec = new std::vector<std::string>();
    // printf("%d\n", sizeof(*vec));
    std::array<int, 100> *a = new std::array<int, 100>();
    printf("%d\n", sizeof(*a));

}