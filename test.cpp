#include <iostream>
#include <cstdio>
#include <vector>
using namespace std;


struct Node {
    int type; 
    // -2 represents structure node
    // -1 represents the leftover
    // positive integer represents the item type
    // the leaves are either -1 or positive integer 
    Node* parent = NULL;
    vector<Node*> children;

    Node() {parent = NULL; children.clear(); type = -1;}
    Node(const Node &x) {type = x.type; parent = NULL;children.clear();}
};

void extract_spaces(Node *root, vector<Node*> &spaces)
{
    if(root->type > 0) return;
    else if(root->type == -2)
    {
        for(auto child : root->children)
            extract_spaces(child, spaces);
    }
    else spaces.push_back(root);
} 

Node* clone(Node *x)
{
    Node *new_x = new Node(*x);
    for(auto child : x->children)
    {
        Node *new_x_child = clone(child);
        new_x_child->parent = new_x;
        new_x->children.push_back(new_x_child);
    }
    return new_x;
}

void get(Node* node, vector<Node*> & nodes_to_delete)
{
    for(auto child : node->children)
        get(child, nodes_to_delete);
    nodes_to_delete.push_back(node);
}

void Delete(Node* node)
{
    for(auto child : node->children)
        Delete(child);
    node->parent = NULL;
    delete node; 
    node = NULL;
}



void function(Node* x, vector<Node*> &spaces)
{
    spaces.push_back(x);
}
int main ()
{
    vector<Node*> patterns;
    Node* x1 = new Node;
    Node* x2 = new Node;
    Node* x3 = new Node;
    x1->children.push_back(x2);
    x1->children.push_back(x3);
    x1->parent = NULL;
    x2->parent = x1;
    x3->parent = x1;
    
    cout << x2->parent << endl;
    // vector<Node*> nodes_to_delete;
    // get(x1, nodes_to_delete);
    // for(int i = 0; i < nodes_to_delete.size(); i++)
    // {
    //     nodes_to_delete[i]->parent = NULL;
    //     delete nodes_to_delete[i]; 
    //     nodes_to_delete[i] = NULL;
    // }

    Delete(x1);
    // x2->parent = NULL;
    // delete x1;
    // x1 = NULL;
    
    //cout<< x1->children.size() << endl;
    //x1 = NULL;
    //delete x2;
    cout << x1 << " " << x1->children.size() << endl;
    cout << x2->parent << endl;
//     x1->type = 1;
//     x2->type = 2;
//     x3->type = 3;
//     patterns.push_back(x1);
//     patterns.push_back(x2);
//     patterns.push_back(x3);
//     cout<<patterns[1]->type<<endl;
//     auto it = patterns.begin();
//     while(*it != x2) it++;
//     patterns.erase(it);
//     cout<<patterns[1]->type<<endl;
 }