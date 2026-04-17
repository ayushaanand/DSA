#pragma once
#include "allocator.hpp"

#include <functional>
#include <map>
#include <vector>
#include <iostream>

struct stdBST {
    struct MetaData {};


    // trying to pass allocator, should memPool be passed or createNode, if memPool, how construct creteNode locally
    template <typename Node, typename Type, typename Compare, typename Allocator>
    static void __insert_node(Node **root, const Type& data, Compare comp, Allocator& memPool){
        if(*root == nullptr){
            *root = createNode(nullptr, nullptr, MetaData{}, data);
            return;
        }

        // return <Node *parent, bool isOnParentsRight>
        auto __search_recur = [comp, data](auto& self, Node *node) -> std::pair<Node *, int> {
            if(comp(data, node -> data)){
                if(node -> left == nullptr) return {node, 0};
                else return self(self, node -> left);
            }
            else if(comp(node -> data, comp)){
                if(node -> right == nullptr) return {node, 1};
                else return self(self, node -> right);
            }
            else return {nullptr, 0};
        };

        std::pair<Node *, bool> __search_res = __search_recur(__search_recur, *root);
        if(__search_res.first == nullptr) return;
        else __search_res.first -> ptr_arr[__search_res.second] = createNode(nullptr, nullptr, MetaData{}, data);
    }

    template<typename Node, typename Type, typename Compare, typename Deallocator>
    static void __delete_node(Node **root, const Type& data, Compare comp, Deallocator& destroyNode){
        if(*root == nullptr) return;
        
        // root exception for deletion, a dummy node for general logic
        Node root_parent(*root, nullptr, MetaData{}, lazyTypeDummy{});

        Node *parent = &root_parent, *curr = *root;
        int isRight = 0;
        while(1){
            if(comp(data, curr -> data)){
                parent = curr;
                curr = curr -> left;
                isRight = 0;
            }
            else if(comp(curr -> data, data)){
                parent = curr;
                curr = curr -> right;
                isRight = 1;
            }
            else break;
        }

        for(int i = 0;i < 2;i++) if(curr -> ptr_arr[i] == nullptr) {
            parent -> ptr_arr[isRight] = curr -> ptr_arr[i];
            return;
        }

        bool rng;
        parent -> ptr_arr[isRight] = curr -> ptr_arr[rng];
        Node *curr1;
        for(curr1 = curr -> ptr_arr[rng];curr1 -> ptr_arr[(rng + 1) % 2] != nullptr;curr1 = curr1 -> ptr_arr[(rng + 1) % 2]);
        curr1 -> ptr_arr[(rng + 1) % 2] = curr -> ptr_arr[(rng + 1) % 2];

        destroyNode(curr);
        *root = root_parent -> left;
    }
};


// Compare(data, node -> data) == 1 means go left, default is '<'
template <typename Type, typename Compare = std::less<Type>, typename TreeType = stdBST>
class binaryTree {
    private:
        struct Node : lazyTypeConstructor<Type>{
            union{
                Node *ptr_arr[2];
                struct {
                    Node *left, *right;
                };
            };
            typename TreeType::MetaData meta;

            template <typename... Args>
            Node(Node *l, Node *r, TreeType::MetaData m, Args&&... args) : left(l), right(r), meta(m), lazyTypeConstructor<Type> (std::forward<Args>(args)...) {};            
        };
        
        segmentedArrayAllocator<Node> memPool;
        Node *root;
        size_t contiguousNodes;
        Compare comp;

        template <typename... Args>
        Node *createNode(Node *left, Node *right, TreeType::Metadata& meta, Args&&... args){
            Node *newNode = memPool.allocate();
            return new (newNode) Node(left, right, meta, std::forward<Args>(args)...);
        }

        void destroyNode(Node *node){
            node -> data.~Type();
            memPool.deallocate(node);
        }

    public :
        
        binaryTree(size_t contiguousNodes = 256) : memPool(contiguousNodes), root(nullptr){
            this -> contiguousNodes = contiguousNodes;
        }

        ~binaryTree(){
            auto free_recur = [](auto& self, Node *node){
                if(node == nullptr) return;
                node -> data.~Type();
                self(self, node -> left);
                self(self, node -> right);
            };

            free_recur(free_recur, root);
        }

        int isEmpty(){
            return (root == nullptr) ? 1 : 0;
        }

        void insert(const Type& data){
            TreeType::__insert_node<Node, Type, Compare, segmentedArrayAllocator<Type>> (&root, data, comp, memPool);
        }
        
        void printTree_side(std::ostream& os = std::cout){
            auto print = [&os](auto& self, Node *node, int depth){
                if(node == nullptr) return;
                self(self, node -> right, depth + 1, os);
                for(int i = 0;i < depth;++i) os << '\t';
                os << node -> data << std::endl;
                self(self, node -> left, depth + 1, os);
            };

            print(print, root, 0);
        }

        // void printTree_vert(std::ostream& os){
        //     std::vector<int> char_matrix;

        //     auto idx = [&char_matrix]()

        //     auto print = [&char_matrix](Node *node, int passedLeft, std::pair<int, int> pos){
        //         if()
        //     };
        // }
};