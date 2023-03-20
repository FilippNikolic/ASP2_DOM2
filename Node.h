//
// Created by nfili on 24/11/2022.
//

#ifndef MAIN_CPP_NODE_H
#define MAIN_CPP_NODE_H

#include "ID.h"

class Podatak;
class ID;

class Node {
public:
    Node *next;
    int max;
    int pos;
    Node *parent;
    ID *niz = new ID[4];
    explicit Node(int m);
    Node();
    Node(Node& node){
        max = node.max;
        parent = node.parent;
        next = node.next;
        for (int i = 0; i < max-1; ++i) {
            niz[i].id = node.niz[i].id;
            niz[i].right = node.niz[i].right;
            niz[i].left = node.niz[i].left;
        }
    }

    Node(const Node& k){
        pos = k.pos;
        max = k.max;

    }




};


#endif //MAIN_CPP_NODE_H
