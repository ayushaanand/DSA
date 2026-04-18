#pragma once
#include "allocator.hpp"

#include <functional>
#include <utility>
#include <iostream>

struct stdBST {
    struct MetaData {};

    // returns ifInsertDone
    template <typename Node, typename Compare>
    static bool __insert_node(Node **root, Node *newNode, Compare comp){
        if(*root == nullptr){
            *root = newNode;
            return true;
        }

        Node *curr = *root;
        int isOnRight = 0;
        while(1){
            if(comp(newNode -> data, curr -> data)){
                if(curr -> left == nullptr){
                    isOnRight = 0;
                    break;
                }
                else curr = curr -> left;
            }
            else if(comp(curr -> data, newNode -> data)){
                if(curr -> right == nullptr){
                    isOnRight = 1;
                    break;
                }
                else curr = curr -> right;
            }
            else return false;
        }


        curr -> ptr_arr[isOnRight] = newNode;
        return true;
    }

    template <typename Node, typename Type, typename Compare>
    static Node *__delete_node(Node **root, const Type& target, Compare comp){
        if(*root == nullptr) return nullptr;
        
        // root exception for deletion, a dummy node for general logic
        Node root_parent(*root, nullptr, MetaData{}, lazyTypeDummy{});

        Node *parent = &root_parent, *curr = *root;
        int isRight = 0;
        while(1){
            if(curr == nullptr) return nullptr;
            if(comp(target, curr -> data)){
                parent = curr;
                curr = curr -> left;
                isRight = 0;
            }
            else if(comp(curr -> data, target)){
                parent = curr;
                curr = curr -> right;
                isRight = 1;
            }
            else break;
        }

        if(curr -> left == nullptr && curr -> right == nullptr){
            parent -> ptr_arr[isRight] = nullptr;
            *root = root_parent.left;
            return curr;
        }

        for(int i = 0;i < 2;i++) if(curr -> ptr_arr[i] == nullptr && curr -> ptr_arr[(i + 1) % 2] != nullptr) {
            parent -> ptr_arr[isRight] = curr -> ptr_arr[(i + 1) % 2];
            *root = root_parent.left;
            return curr;
        }

        int rng = !isRight;
        auto o = [rng](){
            return (rng + 1) % 2;
        };

        parent -> ptr_arr[isRight] = curr -> ptr_arr[rng];
        Node *curr1;
        for(curr1 = curr -> ptr_arr[rng];curr1 -> ptr_arr[o()] != nullptr;curr1 = curr1 -> ptr_arr[o()]);
        curr1 -> ptr_arr[o()] = curr -> ptr_arr[o()];

        *root = root_parent.left;

        return curr;
    }

    template <typename Node, typename Type, typename Compare>
    static bool __search_node(Node *root, const Type& target, Compare comp){
        Node *curr = root;
        while(curr != nullptr){
            if(comp(target, curr -> data)) curr = curr -> left;
            else if(comp(curr -> data, target)) curr = curr -> right;
            else break;
        }
        return (curr != nullptr) ? true : false;
    }
};

struct avlBST {
    struct MetaData {int height;};

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
            Node(Node *l, Node *r, typename TreeType::MetaData m, Args&&... args) : left(l), right(r), meta(m), lazyTypeConstructor<Type> (std::forward<Args>(args)...) {};            
        };
        
        segmentedArrayAllocator<Node> memPool;
        Node *root;
        size_t contiguousNodes;
        Compare comp;

        template <typename... Args>
        Node *createNode(Node *left, Node *right, typename TreeType::MetaData meta, Args&&... args){
            Node *newNode = memPool.allocate();
            return new (newNode) Node(left, right, meta, std::forward<Args>(args)...);
        };

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
                self(self, node -> left);
                self(self, node -> right);
                node -> data.~Type();
            };

            free_recur(free_recur, root);
        }

        int isEmpty(){
            return (root == nullptr) ? 1 : 0;
        }

        template <typename... Args>
        void insert(Args&&... args){
            Node *newNode = createNode(nullptr, nullptr, typename TreeType::MetaData {}, std::forward<Args>(args)...);

            bool isInsertDone = TreeType::__insert_node(&root, newNode, comp);
            if(!isInsertDone) destroyNode(newNode);
        }

        void erase(const Type& target){
            Node *target_node = TreeType::__delete_node(&root, target, comp);

            if(target_node != nullptr) destroyNode(target_node);
        }

        bool search(const Type& target){
            return TreeType::__search_node(root, target, comp);
        }
        
        void printTree_side(std::ostream& os = std::cout){
            auto print = [&os](auto& self, Node *node, int depth){
                if(node == nullptr) return;
                self(self, node -> right, depth + 1);
                for(int i = 0;i < depth;++i) os << ' ';
                os << node -> data << std::endl;
                self(self, node -> left, depth + 1);
            };

            print(print, root, 0);
        }
};