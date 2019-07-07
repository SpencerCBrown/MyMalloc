#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>

#include <sys/mman.h>

typedef char byte;
struct Header;

void* GetPage();
Header* GetBlock(int level);
void* MyMalloc(size_t size);
size_t Round(size_t size);
uint8_t LogSize(size_t size);
Header* CreateHeader(void* block, uint8_t size);

struct Header {
    Header* link; // 8 bytes on x64
    uint8_t size;
};

Header* list_pop(Header*& list)
{
    Header* head = list;
    if (head != nullptr) {
        list = head->link;
    }
    return head;
}

void list_append(Header*& list, Header* node)
{
    if (list == nullptr) {
        list = node;
        return;
    }

    Header* iter = list;
    while (iter->link != nullptr) {
        iter = iter->link;
    }
    iter->link = node;
    node->link = nullptr;
}

Header* CreateHeader(void* block, uint8_t size)
{
    Header* header = (Header*) block;
    header->size = size;
    header->link = nullptr;
    return (Header*) header;
}

/* log powers: 2^4 .. 2^12 - requests for < 16B will be rounded, to allow for max header size (64 arch) */
Header* freeLists[9] = {nullptr};

// functions
void* GetPage()
{
    void* addr = mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return addr;
}

/* level should be the log_2 of the rounded size of the requested block. I.E. for a 16B request level will be 4 */
Header* GetBlock(int level)
{
    /* 4 is the first level, so for 0-index array... */
    int adjusted_level = level - 4;

    /* Get pointer to head of list for this level */
    Header* listL = freeLists[adjusted_level];

    /* Get available block node header from list, or null if list is empty */
    Header* block = list_pop(listL);

    /* No blocks of the needed size */
    if (block == nullptr) {

        /* Special case - need to map another page of memory from the OS */
        if (level == 12) { // BASE CASE of recursion
            void* page = GetPage();
            block = CreateHeader(page, 12);
        } else {
            /* No blocks of needed size, recursively call GetBlock to get a block of the next higher level */
            Header* nextLevelHigherBlock = GetBlock(level + 1);

            // split higher block, assign one half to `block` and insert other half to free list
            size_t blockSizeInBytes = pow(2, nextLevelHigherBlock->size);
            byte* midAddr = ((byte*)nextLevelHigherBlock) + (blockSizeInBytes / 2);
            Header* leftBuddy = CreateHeader((void*) nextLevelHigherBlock, level);
            Header* rightBuddy = CreateHeader((void*) midAddr, level);
            block = leftBuddy;
            list_append(listL, rightBuddy);
            freeLists[adjusted_level] = listL;
        }
    }

    return block;
}

void* MyMalloc(size_t size)
{
    size = Round(size); // nearest viable size
    uint8_t level = LogSize(size);
    Header* block = GetBlock(level); // pointer to header preceding actual memory block
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