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

void FreeList::remove(Header* node)
{
    Header* h = head;
    if (h == node)
        head = nullptr;
    while (h) {
        if (h->link == node) {
            h->link = h->link->link;
            break;
        }
        h = h->link;
    }
}