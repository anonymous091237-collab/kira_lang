#pragma once
#include <string>

#include "../../../include/constants.hpp"

using std::string;
using std::string_view;
struct Token;

struct node {
    NodeType type;
    std::string_view name;
    // Token* token;
    bool is_constant = true;
    bool is_ptr = false;
    node* child;
    node* sibling;
};

using Node = node*;

class Tree {
   private:
    static void print_helper(Node father, int depth);
    void del_helper(Node father, int& deleted);
    void clear();

   public:
    Node root = nullptr;
    static Node* getChild(Node branch, int num = 1);
    static bool hasChild(Node branch, int num = 1);
    static int countChild(Node branch);
    static Node createNode(NodeType type,
                           std::string_view name = std::string_view{});

    static void print(Node father);
    void print();

    Tree();
    ~Tree();
};