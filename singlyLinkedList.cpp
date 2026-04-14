#pragma once

#include <utility>
#include <new>

#include "allocator.hpp"

template <typename Type>
class singlyLinkedList {
    private :
        struct singleLinkNode : lazyTypeConstructor<Type> {
            singleLinkNode *next;

            template <typename... Args>
            singleLinkNode(singleLinkNode *nxt, Args&&... args) : next(nxt), lazyTypeConstructor<Type>(std::forward<Args>(args)...) {}
        };

        segmentedArrayAllocator<singleLinkNode> memPool;
        singleLinkNode *prevHead, *tail;
        size_t contiguousNodes;

        template <typename... Args>
        singleLinkNode *createNode(singleLinkNode *next, Args&&... args){
            singleLinkNode *newNode = memPool.allocate();
            return new (newNode) singleLinkNode(next, std::forward<Args>(args)...);
        }

        void destroyNode(singleLinkNode *node){
            node -> data.~Type();
            memPool.deallocate(node);
        }

    public :
        singlyLinkedList(size_t contiguousNodes = 512) : memPool(contiguousNodes) {
            this -> contiguousNodes = contiguousNodes;
            prevHead = tail = createNode(nullptr, lazyTypeDummy{});
        }

        ~singlyLinkedList(){
            singleLinkNode *curr = prevHead -> next;
            while(curr != nullptr){
                (curr -> data).~Type();
                curr = curr -> next;
            }
        }

        singlyLinkedList(const singlyLinkedList&) = delete;
        singlyLinkedList& operator=(const singlyLinkedList&) = delete;

    private:
        class Iterator {
            friend class singlyLinkedList;
            private : 
                singleLinkNode *prevNode;
                singleLinkNode *currNode(){
                    if(prevNode == nullptr) return nullptr;
                    else return prevNode -> next;
                }

            public :
                Iterator(singleLinkNode *prevNode){
                    this -> prevNode = prevNode;
                }

                int isNull(){
                    return (prevNode == nullptr || prevNode -> next == nullptr) ? 1 : 0;
                }

                Type& operator*(){
                    return prevNode -> next -> data;
                }

                Iterator& operator++(int){
                    Iterator old = *this;
                    prevNode = prevNode -> next;
                    return old;
                }

                Iterator operator++(){
                    prevNode = prevNode -> next;
                    return *this;
                }
        };

    public : 
        Iterator newIterator() {return Iterator(prevHead);};

        template<typename Predicate>
        Iterator newIterator_with(Predicate condition){
            auto it = newIterator();
            while(!it.isNull()){
                if(condition(*it)) return it;
                else ++it;
            }            
            return it;
        }

        Iterator newIterator(const Type& data){
            return newIterator_with([&data](const Type& val){
                return val == data;
            });
        }

        template <typename... Args>
        void emplace_back(Args&&... args){
            singleLinkNode *newNode = createNode(nullptr, std::forward<Args>(args)...);

            tail -> next = newNode;
            // if(prevHead == tail) prevHead -> next = newNode;
            tail = newNode;
        }

        void push_back(const Type& data){
            emplace_back(data);
        }

        void push_back(Type&& data){
            emplace_back(std::move(data));
        }

        template <typename... Args>
        void emplace_front(Args&&... args){
            singleLinkNode *newNode = createNode(prevHead -> next, std::forward<Args>(args)...);

            prevHead -> next = newNode;
            if(prevHead == tail) [[unlikely]] tail = newNode;
        }

        void push_front(const Type& data){
            emplace_front(data);
        }

        void push_front(Type&& data){
            emplace_front(std::move(data));
        }

        template <typename... Args>
        void emplace_after(Iterator it, Args&&... args){
            if(it.isNull()) return;
            singleLinkNode *currNode = it.currNode();
            singleLinkNode *newNode = createNode(currNode -> next, std::forward<Args>(args)...);

            currNode -> next = newNode;
            if(currNode == tail) tail = newNode;
        }

        void add_after(Iterator& it, const Type& data){
            emplace_after(it, data);
        }

        void add_after(Iterator& it, Type&& data){
            emplace_after(it, std::move(data));
        }

        template <typename... Args>
        void emplace_before(Iterator& it, bool pointToNew, Args&&... args){
            if(it.isNull()) return;
            singleLinkNode *newNode = createNode(it.currNode(), std::forward<Args>(args)...);

            (it.prevNode) -> next = newNode;
            if(!pointToNew) ++it;
        }

        void add_before(Iterator& it, bool pointToNew, const Type& data){
            emplace_before(it, pointToNew, data);
        }

        void add_before(Iterator& it, bool pointToNew, Type&& data){
            emplace_before(it, pointToNew, std::move(data));
        }

        // given iterator then uses next node 
        void erase(Iterator& it){
            if(it.isNull()) return;
            singleLinkNode *currNode = it.currNode();
            (it.prevNode) -> next = currNode -> next;
            if(currNode == tail) [[unlikely]] tail = it.prevNode;
            
            destroyNode(currNode);
        }

        void erase_after(Iterator& it){
            if(it.isNull()) return;
            singleLinkNode *currNode = it.currNode();
            if(currNode -> next == nullptr) return;

            singleLinkNode *afterNode = currNode -> next;
            currNode -> next = afterNode -> next;
            if(afterNode == tail) [[unlikely]] tail = currNode;
            
            destroyNode(afterNode);            
        }

        void erase_front(){
            if(prevHead == tail) return;
            singleLinkNode *prevFront = prevHead -> next;

            prevHead -> next = prevFront -> next;
            if(tail == prevFront) tail = prevHead;

            destroyNode(prevFront);
        }

        int isEmpty(){
            return (tail == prevHead) ? 1 : 0;
        }

        void printList(std::ostream& os = std::cout){
            if(isEmpty()) os << "Empty List";
            else {
                auto it = newIterator();
                while(!it.isNull()){
                    os << *it << " -> ";
                    ++it;
                }
                os << " x ";
            }
            os << std::endl;
        }

        void untangle(){
            segmentedArrayAllocator<singleLinkNode> newMemPool(contiguousNodes);

            singleLinkNode *newTail, *newPrevHead;
            newPrevHead = newMemPool.allocate();
            newPrevHead = newTail = new (newPrevHead) singleLinkNode(nullptr, lazyTypeDummy{});

            for(auto it = newIterator();!it.isNull();++it){
                auto newNode = newMemPool.allocate();
                newNode = new (newNode) singleLinkNode(nullptr, std::move(*it));
                newTail -> next = newNode;
                newTail = newNode;
                (&*it) -> ~Type();
            }

            memPool = std::move(newMemPool);
            prevHead = newPrevHead;
            tail = newTail;
        }
};