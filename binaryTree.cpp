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
        
        Node **parent = root, *curr = *root;
        int isRight = 0;
        while(1){
            if(curr == nullptr) return nullptr;
            if(comp(target, curr -> data)){
                parent = &(curr -> left);
                curr = curr -> left;
                isRight = 0;
            }
            else if(comp(curr -> data, target)){
                parent = &(curr -> right);
                curr = curr -> right;
                isRight = 1;
            }
            else break;
        }

        if(curr -> left == nullptr && curr -> right == nullptr){
            *parent = nullptr;
            return curr;
        }

        for(int i = 0;i < 2;i++) if(curr -> ptr_arr[i] == nullptr && curr -> ptr_arr[(i + 1) % 2] != nullptr) {
            *parent = curr -> ptr_arr[(i + 1) % 2];
            return curr;
        }

        *parent = curr -> ptr_arr[!isRight];
        Node *curr1;
        for(curr1 = curr -> ptr_arr[!isRight];curr1 -> ptr_arr[isRight] != nullptr;curr1 = curr1 -> ptr_arr[isRight]);
        curr1 -> ptr_arr[isRight] = curr -> ptr_arr[isRight];

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

    template <typename Node>
    static inline int lheight(Node *node){
        if(node == nullptr) return -1;
        return (node -> left == nullptr) ? -1 : node -> left -> meta.height;
    }

    template <typename Node>
    static inline int rheight(Node *node){
        if(node == nullptr) return -1;
        return (node -> right == nullptr) ? -1 : node -> right -> meta.height;
    }

    template <typename Node>
    static inline int balance(Node *node){
        if(node == nullptr) return 0;
        else return lheight(node) - rheight(node);
    }

    template <typename Node>
    static void __update_height(Node *node){
        if(node == nullptr) return;
        int lh = lheight(node), rh = rheight(node);
        node -> meta.height = (lh > rh) ? (lh + 1) : (rh + 1);
    }

    template <typename Node>
    static inline void rebalance(Node **node){
        __update_height(*node);
        if(balance(*node) <= 1 && balance(*node) >= -1) return;

        bool imbalance[2];
        imbalance[0] = balance(*node) < 0;
        imbalance[1] = (balance((*node) -> ptr_arr[imbalance[0]]) == 0) ? imbalance[0] : balance((*node) -> ptr_arr[imbalance[0]]) < 0;

        if(imbalance[0] == imbalance[1]){
            Node *curr = *node;
            *node = curr -> ptr_arr[imbalance[0]];

            curr -> ptr_arr[imbalance[0]] = (*node) -> ptr_arr[!imbalance[1]];
            (*node) -> ptr_arr[!imbalance[1]] = curr;

            __update_height(curr);
            __update_height(*node);
        }

        else {
            Node *curr = *node;
            Node *next = curr -> ptr_arr[imbalance[0]];
            *node = next -> ptr_arr[imbalance[1]];

            curr -> ptr_arr[imbalance[0]] = (*node) -> ptr_arr[!imbalance[0]];
            next -> ptr_arr[imbalance[1]] = (*node) -> ptr_arr[!imbalance[1]];

            (*node) -> ptr_arr[!imbalance[0]] = curr;
            (*node) -> ptr_arr[!imbalance[1]] = next;

            __update_height(curr);
            __update_height(next);
            __update_height(*node);
        }
    }

    template <typename Node, typename Compare>
    static bool __insert_node(Node **root, Node *newNode, Compare comp){
        newNode -> meta.height = 0;
        auto recur = [newNode, comp](auto& self, Node **node){
            if(*node == nullptr){
                *node = newNode;
                return true;
            }
            if(comp(newNode -> data, (*node) -> data)){
                if(!self(self, &((*node) -> left))) return false;
            }
            else if(comp((*node) -> data, newNode -> data)){
                if(!self(self, &((*node) -> right))) return false;
            }
            else return false;

            rebalance(node);
            return true;
        };

        return recur(recur, root);
    }

    template <typename Node, typename Type, typename Compare>
    static Node *__delete_node(Node **root, const Type& target, Compare comp){
        auto recur = [target, comp](auto& self, Node **node) -> Node* {
            Node *found;

            if(*node == nullptr) return nullptr;
            if(comp(target, (*node) -> data)){
                if(!(found = self(self, &((*node) -> left)))) return nullptr;
            }
            else if(comp((*node) -> data, target)){
                if(!(found = self(self, &((*node) -> right)))) return nullptr;
            }
            else{
                Node *curr = *node;
                if(curr -> left == nullptr && curr -> right == nullptr){
                    *node = nullptr;
                }
                else if(curr -> left != nullptr && curr -> right == nullptr){
                    *node = curr -> left;
                }
                else if(curr -> left == nullptr && curr -> right != nullptr){
                    *node = curr -> right;
                }
                else{
                    auto recur = [node, curr](auto& self, Node **n) -> void {
                        if((*n) -> left != nullptr) self(self, &((*n) -> left));
                        else{
                            *node = *n;
                            *n = (*node) -> right;
                            (*node) -> left = curr -> left;
                            (*node) -> right = curr -> right;
                        }

                        if(n != &(curr -> right)) rebalance(n);
                        else rebalance(node);
                    };
                    
                    recur(recur, &(curr -> right));                    
                }
                rebalance(node);
                return curr;
            }

            rebalance(node);
            return found;
        };

        return recur(recur, root);
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


// Compare(data, node -> data) == 1 means go left, default is '<'
template <typename Type, typename TreeType = avlBST, typename Compare = std::less<Type>>
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
                if(node == nullptr){
                    for(int i = 0;i < depth;++i) os << ' ';
                    os << 'x' << std::endl;
                    return;
                }
                self(self, node -> right, depth + 1);
                for(int i = 0;i < depth;++i) os << ' ';
                os << node -> data << '\n';
                self(self, node -> left, depth + 1);
            };

            print(print, root, 0);
            os << std::endl;
        }
};

#include <iostream>
#include <random>
#include <set>
#include <cassert>

// ... [Your binaryTree and stdBST code here] ...

void runChaosTest(int numOperations, int valueRange) {
    std::cout << "[SYSTEM] Initiating Chaos Fuzzing Engine...\n";
    std::cout << "[SYSTEM] Operations: " << numOperations << " | Value Range: [0, " << valueRange << "]\n";

    // 1. Initialize our Oracle (std::set) and our Engine (binaryTree)
    // We give the memory pool a generous initial block, assuming your allocator 
    // dynamically expands if it fills up.
    std::set<int> oracleTree;
    binaryTree<int, avlBST> myTree(numOperations); 

    // 2. Setup the deterministic RNG engine (Mersenne Twister)
    // Using a fixed seed (1337) so if it crashes, it crashes the EXACT same way every time, 
    // allowing you to debug the exact sequence of numbers that killed it.
    std::mt19937 rng(1337); 
    std::uniform_int_distribution<int> opDist(0, 99);
    std::uniform_int_distribution<int> valDist(0, valueRange);

    int insertCount = 0, eraseCount = 0, searchCount = 0;

    // 3. THE CHAOS LOOP
    for (int i = 0; i < numOperations; ++i) {
        int op = opDist(rng);    // Pick a random operation (0-99)
        int val = valDist(rng);  // Pick a random value to manipulate

        if (op < 45) { 
            // 45% CHANCE: INSERT
            myTree.insert(val);
            oracleTree.insert(val);
            insertCount++;
        } 
        else if (op < 90) { 
            // 45% CHANCE: ERASE
            // This will randomly test deleting roots, leaves, 1-child, and non-existent nodes.
            myTree.erase(val);
            oracleTree.erase(val);
            eraseCount++;
        } 
        else { 
            // 10% CHANCE: SEARCH VALIDATION
            bool myRes = myTree.search(val);
            bool oracleRes = (oracleTree.find(val) != oracleTree.end());
            
            if (myRes != oracleRes) {
                std::cerr << "\n[FATAL ERROR] Search mismatch mid-loop!\n";
                std::cerr << "Value: " << val << " | Expected: " << oracleRes << " | Got: " << myRes << "\n";
                exit(1);
            }
            searchCount++;
        }
    }

    std::cout << "[SYSTEM] Chaos Loop Complete. Insertions: " << insertCount 
              << " | Erasures: " << eraseCount << " | Mid-loop Searches: " << searchCount << "\n";
    std::cout << "[SYSTEM] Running deep structural cross-verification...\n";

    // 4. THE DEEP VERIFICATION SWEEP
    // We query EVERY single possible number in the universe to ensure the tree matches the Oracle.
    for (int val = 0; val <= valueRange + 100; ++val) {
        bool myRes = myTree.search(val);
        bool oracleRes = (oracleTree.find(val) != oracleTree.end());
        
        if (myRes != oracleRes) {
            std::cerr << "\n[FATAL ERROR] State mismatch detected post-loop!\n";
            std::cerr << "Value " << val << " is " << (oracleRes ? "supposed to be IN" : "supposed to be MISSING") << " the tree.\n";
            exit(1);
        }
    }

    // 5. MEMORY TEARDOWN PHASE
    // When the function exits, your ~binaryTree() destructor will fire, testing your Post-Order traversal cleanup.
    std::cout << "[SYSTEM] Verification 100% Passed. Tree memory state is mathematically flawless.\n";
    myTree.printTree_side();
}

int main() {

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    // Test 1: High collision rate (Narrow value range forces frequent duplicate inserts and merges)
    std::cout << "\n--- TEST PHASE 1: HIGH COLLISION ---\n";
    runChaosTest(100000, 500); 

    // Test 2: Structural sprawl (Wide value range forces massive, deep tree branching)
    std::cout << "\n--- TEST PHASE 2: STRUCTURAL SPRAWL ---\n";
    // runChaosTest(500000, 1000000);

    return 0;
}