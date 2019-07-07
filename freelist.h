#include <cstdint>

struct Header {
    Header* link; // 8 bytes on x64
    uint8_t size;
};

class FreeList
{
public:
    FreeList() : head(nullptr) {}

    void append(Header*);
    Header* popHead();
    Header* peekHead() const {return head;}
private:
    Header* head;
};