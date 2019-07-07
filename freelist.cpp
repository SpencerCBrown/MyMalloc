#include "freelist.h"

void FreeList::append(Header* node)
{
    if (head == nullptr) {
        head = node;
    } else {
        Header* next = head->link;
        head->link = node;
        node->link = next;
    }
}

Header* FreeList::popHead()
{
    Header* hd = head;
    if (head != nullptr) {
        head = head->link;
    }
    return hd;
}