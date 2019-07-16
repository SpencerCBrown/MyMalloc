#include <cstdint>

/* sizeof(Header) is max 16B, so to be safe the minimum allocation size will be 2^4 = 16. */
struct Header {
    Header* link; // 8 bytes on x64
    uint8_t size;
    bool free;
};

class FreeList
{
public:
    FreeList() : head(nullptr) {}

    void append(Header*);
    Header* popHead();
    Header* peekHead() const {return head;}
    void remove(Header*);
private:
    Header* head;
};