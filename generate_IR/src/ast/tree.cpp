#include "tree.hpp"

#include <iostream>
#include <stdexcept>

// #include "lexer/tokenizer.hpp"

Node Tree::createNode(NodeType type, std::string_view name) {
    return new node{
        .type = type, .name = name, .child = nullptr, .sibling = nullptr};
}

Tree::Tree() {
    if (root != nullptr)
        throw std::runtime_error("this node already exists!\n");

    root = (Node)malloc(sizeof(node));
    root->child = nullptr;
    root->sibling = nullptr;
}

Tree::~Tree() { clear(); }

void Tree::print_helper(Node father, int depth) {
    Node p = father->child;

    while (p != nullptr) {
        for (int i = 0; i < depth; i++) std::cout << "    ";
        std::string_view nodetype = NodeTypeNames[static_cast<size_t>(p->type)];
        if (!p->name.empty()) {
            std::cout << '<' << nodetype << "> " << p->name << '\n';
        } else if (nodetype == "Block") {
            std::cout << "<Block>\n";
        } else {
            std::cout << '<' << nodetype << ">\n";
        }

        print_helper(p, depth + 1);
        p = p->sibling;
    }
}
void Tree::print(Node father) {
    std::string_view nodetype =
        NodeTypeNames[static_cast<size_t>(father->type)];
    if (!father->name.empty()) {
        std::cout << '<' << nodetype << "> " << father->name << '\n';
    } else if (nodetype == "Block") {
        std::cout << "<Block>\n";
    } else {
        std::cout << '<' << nodetype << ">\n";
    }

    print_helper(father, 1);
}
void Tree::print() {
    std::string_view nodetype = NodeTypeNames[static_cast<size_t>(root->type)];
    if (!root->name.empty()) {
        std::cout << '<' << nodetype << "> " << root->name << '\n';
    } else if (nodetype == "Block") {
        std::cout << "<Block>\n";
    } else {
        std::cout << '<' << nodetype << ">\n";
    }

    print_helper(root, 1);
}

void Tree::del_helper(Node father, int& deleted) {
    Node p = father->child;
    Node next;
    while (p != nullptr) {
        del_helper(p, deleted);
        next = p->sibling;
        free(p);
        deleted++;
        p = next;
    }
}

void Tree::clear() {
    int deleted = 0;
    del_helper(root, deleted);
    free(root);
    deleted++;
    printf("nodes deleted %d\n", deleted);
}
bool Tree::hasChild(Node branch, int num)

{
    Node* p = &(branch->child);
    int i = 0;
    while (i < num) {
        if ((*p) == nullptr) {
            return false;
        }
        p = &((*p)->sibling);
        i++;
    }
    return true;
}
int Tree::countChild(Node branch) {
    int i = 0;
    Node p = (branch->child);
    while (p != nullptr) {
        i++;
        p = p->sibling;
    }
    return i;
}
Node* Tree::getChild(Node branch, int num) {
    Node* p = &(branch->child);
    int i = 1;
    while (i < num) {
        if ((*p) == nullptr) {
            throw std::runtime_error("parent has less than " +
                                     std::to_string(num) + " children!\n");
        }
        p = &((*p)->sibling);
        i++;
    }
    return p;
}