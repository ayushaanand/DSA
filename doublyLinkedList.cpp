#pragma once

#include <iostream>
#include <utility>

#include "allocator.hpp"

template <typename Type>
class doublyLinkedList {
    private : 
        struct doubleLinkNode : lazyTypeConstructor<Type> {
            doubleLinkNode *prev, *next;

            template <typename... Args>
            doubleLinkNode(doubleLinkNode *p, doubleLinkNode *n, Args&&... args)
                : prev(p), next(n), lazyTypeConstructor<Type> (std::forward<Args>(args)...) {};
        };

        segmentedArrayAllocator<doubleLinkNode> memPool;
        doubleLinkNode *head, *tail;
        size_t contiguousNodes;

        template <typename... Args>
        doubleLinkNode *createNode(doubleLinkNode *prev, doubleLinkNode *next, Args&&... args){
            doubleLinkNode *newNode = memPool.allocate();
            return new (newNode) doubleLinkNode(prev, next, std::forward<Args>(args)...);
        }

        void destroyNode(doubleLinkNode *node){
            node -> data.~Type();
            memPool.deallocate(node);
        }

    public :
        doublyLinkedList(size_t contiguousNodes = 512) : memPool(contiguousNodes), head(nullptr), tail(nullptr) {
            this -> contiguousNodes = contiguousNodes;
        }

        ~doublyLinkedList(){
            while(head != nullptr){
                head -> data.~Type();
                head = head -> next;
            }
        }

    private :
        class Iterator {
            friend class doublyLinkedList;
            private :
                doubleLinkNode *node;
            
            public :
                Iterator(doubleLinkNode *node_ptr) : node(node_ptr) {};

                Iterator prev(){
                    if(!isNull()) return Iterator(node -> prev);
                    else return Iterator(nullptr);
                }

                Iterator next(){
                    if(!isNull()) return Iterator(node -> next);
                    else return Iterator(nullptr);
                }

                int isNull(){
                    return (node == nullptr) ? 1 : 0;
                }

                Type& operator*(){
                    return node -> data;
                }

                Iterator& operator++(int){
                    Iterator old = *this;
                    node = node -> next;
                    return old;
                }

                Iterator operator++(){
                    node = node -> next;
                    return *this;
                }

                Iterator& operator--(int){
                    Iterator old = *this;
                    node = node -> prev;
                    return old;
                }

                Iterator operator--(){
                    node = node -> prev;
                    return *this;
                }
        };

    public :
        Iterator newIterator(){return Iterator(head);}

        template<typename Predicate>
        Iterator newIterator_with(Predicate condition){
            auto it = newIterator();
            while(!it.isNull() && !condition(*it)) ++it;
            return it;
        }

        Iterator newIterator(const Type& data){
            return newIterator_with([&data](const Type& val){
                return val == data;
            });
        }

        template <typename... Args> 
        void emplace_back(Args&&... args){
            doubleLinkNode *newNode = createNode(tail, nullptr, std::forward<Args>(args)...);
            if(tail == nullptr) head = newNode;
            else tail -> next = newNode;
            tail = newNode;
        }

        template <typename... Args>
        void emplace_front(Args&&... args){
            doubleLinkNode *newNode = createNode(nullptr, head, std::forward<Args>(args)...);
            if(head == nullptr) tail = newNode;
            else head -> prev = newNode;
            head = newNode;
        }

        template <typename... Args>
        void emplace_after(Iterator& it, Args&&... args){
            if(it.isNull()) return;
            doubleLinkNode *newNode = createNode(it.node, it.node -> next, std::forward<Args>(args)...);
            if(it.node == tail) tail = newNode;
            else it.node -> next -> prev = newNode;

            it.node -> next = newNode;
        }

        template <typename... Args>
        void emplace_before(Iterator& it, Args&&... args){
            if(it.isNull()) return;
            doubleLinkNode *newNode = createNode(it.node -> prev, it.node, std::forward<Args>(args)...);
            if(it.node == head) head = newNode;
            else it.node -> prev -> next = newNode;

            it.node -> prev = newNode;
        }

        void erase_front(){
            if(head == nullptr) return;

            if(tail == head) tail = nullptr;
            else head -> next -> prev = nullptr;

            doubleLinkNode *next = head -> next;
            destroyNode(head);
            head = next;
        }

        void erase_back(){
            if(tail == nullptr) return;

            if(head == tail) head = nullptr;
            else tail -> prev -> next = nullptr;

            doubleLinkNode *prev = tail -> prev;
            destroyNode(tail);
            tail = prev;
        }

        // points to next after deletion
        void erase(Iterator& it){
            if(it.isNull()) return;

            if(it.node == head) head = head -> next;
            else it.node -> prev -> next = it.node -> next;

            if(it.node == tail) tail = tail -> prev;
            else it.node -> next -> prev = it.node -> prev;

            doubleLinkNode *next = it.node -> next;
            destroyNode(it.node);
            it.node = next;
        }

        int isEmpty(){
            return (tail == nullptr) ? 1 : 0;
        }

        void untangle(){
            if(isEmpty()) return;
            segmentedArrayAllocator<doubleLinkNode> newMemPool(contiguousNodes);
            doubleLinkNode *newHead, *newTail, *prev;
            auto it = newIterator();
            newHead = newMemPool.allocate();
            newTail = newHead = new (newHead) doubleLinkNode(nullptr, nullptr, std::move(*it));
            ++it;
            while(!it.isNull()){
                prev = newTail;
                newTail = newMemPool.allocate();
                newTail = new (newTail) doubleLinkNode(prev, nullptr, *it);
                prev -> next = newTail;
                (&*it) -> ~Type();
                ++it;
            }
            memPool = std::move(newMemPool);
            head = newHead;
            tail = newTail;
        }

        void printList(std::ostream& os = std::cout){
            auto it = newIterator();
            os << "x";
            while(!it.isNull()){
                os << " <-> " << *it;
                ++it;
            }
            os << " <-> x" << std::endl;
        }
};