#pragma once

#include <stack>
#include <cstdlib>
#include <iostream>

template <typename Type>
class segmentedArrayAllocator {
    private :
        union block {
            block *next;
            Type data;
            block() {}
            ~block() {}
        };

        struct page {
            block *first;
            block *end;
        };
    
        block *nextFree;
        std::stack<page> pageStack;
        size_t contiguousBlocks;
    
    public :
        segmentedArrayAllocator(size_t contiguousBlocks = 256){
            this -> contiguousBlocks = contiguousBlocks;
            pageStack.emplace();
            pageStack.top().first = (block *) std::calloc(contiguousBlocks, sizeof(block));
            pageStack.top().end = pageStack.top().first + contiguousBlocks;

            nextFree = pageStack.top().first;
        }

        ~segmentedArrayAllocator(){
            while(!pageStack.empty()){
                std::free(pageStack.top().first);
                pageStack.pop();
            }   
        }

        segmentedArrayAllocator(const segmentedArrayAllocator&) = delete;
        segmentedArrayAllocator& operator=(const segmentedArrayAllocator&) = delete;    

        Type *allocate(){
            // if page filled
            if(nextFree == pageStack.top().end) [[unlikely]] {
                pageStack.emplace();
                pageStack.top().first = (block *) std::calloc(contiguousBlocks, sizeof(block));
                pageStack.top().end = pageStack.top().first + contiguousBlocks;

                nextFree = pageStack.top().first;
            }

            block *allocated = nextFree;

            if(nextFree -> next == nullptr) nextFree++; // navigating the fresh calloc
            else nextFree = nextFree -> next;

            // return raw bytes
            return &(allocated -> data);
        }

        void deallocate(Type *node_ptr){
            block *node = reinterpret_cast<block *>(node_ptr);
            node -> next = nextFree;
            nextFree = node;
        }

        segmentedArrayAllocator& operator=(segmentedArrayAllocator&& other) noexcept {
            if(this == &other) return *this; // moving to self is no-op

            while(!pageStack.empty()){
                std::free(pageStack.top().first);
                pageStack.pop();
            }   

            nextFree = other.nextFree;
            pageStack = std::move(other.pageStack);
            contiguousBlocks = other.contiguousBlocks;

            other.nextFree = nullptr;
            return *this;
        }

};

struct lazyTypeDummy {};

template <typename Type>
struct lazyTypeConstructor {
    union {Type data;};

    template<typename... Args>
    lazyTypeConstructor(Args&&... args) : data(std::forward<Args>(args)...) {};

    lazyTypeConstructor(lazyTypeDummy) {}
    ~lazyTypeConstructor() {}
};