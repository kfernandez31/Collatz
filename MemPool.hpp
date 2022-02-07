#ifndef MEMPOOL
#define MEMPOOL

#include <stdlib.h>

class MemPool {
public:
    MemPool(size_t unitNum, size_t unitSize) :
            m_pMemBlock(nullptr),
            m_pAllocatedMemBlock(nullptr),
            m_pFreeMemBlock(nullptr),
            m_BlockSize(unitNum * (unitSize + sizeof(node))),
            mnodeSize(unitSize)
    {
        m_pMemBlock = malloc(m_BlockSize); //TODO: Tu oczywiście użyć mmap
        for (size_t i = 0; i < unitNum; i++) { //Link all mem unit . Create linked list.
            node *pCurUnit = (node*)((char*) m_pMemBlock + i*(unitSize+sizeof(node)));

            pCurUnit->prev = nullptr;
            pCurUnit->next = m_pFreeMemBlock;    //Insert the new unit at head.

            if (m_pFreeMemBlock != nullptr) {
                m_pFreeMemBlock->prev = pCurUnit;
            }
            m_pFreeMemBlock = pCurUnit;
        }
    }

    ~MemPool() {
        free(m_pMemBlock);
    }

    void* alloc(size_t nbytes) {
     /*   if (nbytes > mnodeSize || m_pFreeMemBlock == nullptr) {
            return nullptr;
        }*/

        node *pCurUnit = m_pFreeMemBlock;
        m_pFreeMemBlock = pCurUnit->next;  //Get a unit from free linkedlist.
        if (m_pFreeMemBlock != nullptr) {
            m_pFreeMemBlock->prev = nullptr;
        }

        pCurUnit->next = m_pAllocatedMemBlock;

        if (m_pAllocatedMemBlock != nullptr) {
            m_pAllocatedMemBlock->prev = pCurUnit;
        }
        m_pAllocatedMemBlock = pCurUnit;

        return (char*) pCurUnit + sizeof(node) ;
    }

    void dealloc(void* ptr) {
        if (m_pMemBlock < ptr && ptr < (char *)m_pMemBlock + m_BlockSize) {
            node *pCurUnit = (node*)((char*)ptr - sizeof(node) );

            node* prev = pCurUnit->prev;
            node* next = pCurUnit->next;
            if (prev != nullptr){
                m_pAllocatedMemBlock = pCurUnit->next;
            }
            else {
                prev->next = next;
            }

            if (next != nullptr) {
                next->prev = prev;
            }

            pCurUnit->next = m_pFreeMemBlock;
            if (m_pFreeMemBlock != nullptr) {
                m_pFreeMemBlock->prev = pCurUnit;
            }

            m_pFreeMemBlock = pCurUnit;
        }
    }

    inline void* operator new(size_t nbytes, MemPool& pool) {
        if (nbytes == 0) {
            nbytes = 1;
        }
        return pool.alloc(nbytes);
    }

    void operator delete(void* ptr, MemPool& pool) {
        pool.dealloc(ptr);
    }

private:
    static void* sharedMem; // Set this up to be a POSIX shared memory that accesses the shared region in memory
    static const size_t POOL_SIZE;

    typedef struct node {
        node *prev, *next;
    } node;

    void* m_pMemBlock;

    struct node* m_pAllocatedMemBlock;
    struct node* m_pFreeMemBlock;

    size_t mnodeSize;
    size_t m_BlockSize;
};

#endif /*MEMPOOL*/