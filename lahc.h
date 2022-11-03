#pragma once


#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <memory>
#include <cmath>

using namespace std;

/****************************************************************************************/
#pragma region Instance

struct Rectangle{
    int w; 
    int h;
    
    Rectangle(int _w, int _h) : w(_w), h(_h) {}
    Rectangle() : w(0), h(0) {}
    int area() const { return w * h; }
    bool operator<(const Rectangle& r) const { return area() < r.area(); }
    bool operator==(const Rectangle& r) const { return w == r.w && h == r.h;}
};

struct Item{
    int index;
    Rectangle rec;

    Item(int _index, Rectangle _rec) : index(_index), rec(_rec) {}
    int width(bool rotate)  const { return (!rotate) ? rec.w : rec.h; }
    int height(bool rotate) const { return (!rotate) ? rec.h : rec.w; }
    Rectangle rectangle(bool rotate)
    {
        if(rotate)  swap(rec.w, rec.h);
        return rec;
    }
};


void build_instance(const char* filename);

#pragma endregion

/****************************************************************************************/
#pragma region Solution

struct Node {
    int type;
    // -2 represents the structure node
    // -1 represents the leftover
    // positive integer represents the item index
    // the leaves are either -1 or positive integers
    Rectangle rec;
    weak_ptr<Node> parent;
    vector<shared_ptr<Node>> children;
    bool next_cut_orientation; 
    double usage;

    Node() { usage = 0; type = -1; rec = Rectangle(0, 0); next_cut_orientation = 0; }
    Node(const Node &x) { usage = 0; type = x.type; rec = x.rec; next_cut_orientation = x.next_cut_orientation; }
    // true if horizontal, false if vertical    
    int width()     const { return rec.w; }
    int height()    const { return rec.h; }
    int area()      const { return rec.w * rec.h; }

    double cal_usage() 
    { 
        if(type == -1) return 0;
        else if(type > 0) return (double) 1;
        else {
            double res = 0;
            for(auto child : children )
            {
                res += (double) child->cal_usage() * child->rec.area() / rec.area();
            }
            return res;
        }
    }

    double value(double power)
    { 
        if(type > 0) return 0;
        else if(type == -1) return pow(rec.area(), power);
        else {
            double res = 0;
            for(auto child : children )
            {
                res += child->value(power);
            }
            return res;
        }
    }

};

shared_ptr<Node> clone(shared_ptr<Node> x)
{
    shared_ptr<Node> new_x(new Node(*x));
    if(x->children.empty()) return new_x;
    for(auto child : x->children)
    {
        shared_ptr<Node> new_child = clone(child);
        new_child->parent = new_x;
        new_x->children.push_back(new_child);
    }
    return new_x;
}


struct Solution {
    vector<shared_ptr<Node>> patterns; 
    vector<Item> excluded_items;
    vector<shared_ptr<Node>> leftovers;
    vector<int> option_cnt;
    int total_area;

    const int bin_number()      const { return patterns.size(); }
    const int excluded_area() 
    {
        int area = 0;
        for(auto item : excluded_items)
        area += item.rec.area();
        return area;
    }
    const int included_area() { return total_area - excluded_area(); }

    void add_new_bin(Rectangle b)
    {
        shared_ptr<Node> bin(new Node);
        bin->rec = b;
        patterns.push_back(bin);
    }

    Solution deep_copy()
    {
        Solution s; 
        for(auto pattern : patterns)
            s.patterns.push_back(clone(pattern));

        for(auto pattern : s.patterns)
            s.obtain_leftover(pattern);

        s.option_cnt = option_cnt;
        s.excluded_items = excluded_items;
        s.total_area = total_area;
        return s;
    }

    void obtain_leftover(shared_ptr<Node> root)
    {
        if(root->type > 0) return;
        else if(root->type == -1)
        {
            leftovers.push_back(root);
            return;
        }
        for(auto child : root->children)
        {
            obtain_leftover(child);
        }
    }

    const int unchanged_number(){
        int cnt = 0;
        for(auto pattern : patterns)
            {
                pattern->usage = pattern->cal_usage();
                cnt += (pattern->usage == 1);
            }
        return cnt;
    }

    const double value(double power)
    {
        double res = 0;
        for(auto pattern : patterns)
            res += pattern->value(power);
        return res;
    }
};

struct Insertion {
    int space_id;
    double cost;
    bool rotate;

    Insertion(int _space_id, double _cost, bool _rotate) : space_id(_space_id), cost(_cost), rotate(_rotate) {}
    bool operator<(const Insertion &other) const { return cost < other.cost ;} 
    bool operator>(const Insertion &other) const { return cost > other.cost ;} 
};


Solution recreate(Solution S, int mat_limit, vector<shared_ptr<Node>> append);

void insert(shared_ptr<Node> space, Item item, bool rotate, Solution &S);


double vertical_cost(Rectangle space, Rectangle item, double power);
double horizontal_cost(Rectangle space, Rectangle item, double power);


#pragma endregion

/****************************************************************************************/


#pragma region Ruin

Solution ruin(Solution S, int mat_limit, int average_nodes);

void get_all_removable_nodes(shared_ptr<Node> pattern, vector<shared_ptr<Node>> &removable_nodes);
void remove_node(shared_ptr<Node> node, Solution &S);

void merge_two_nodes(shared_ptr<Node> master, shared_ptr<Node> slave, Solution &S);
void exclude(shared_ptr<Node> root, Solution &S);
#pragma endregion

/****************************************************************************************/
#pragma region LAHC

void LAHC(int mat_limit, Solution &S);

int compare(pair<int, double> a, pair<int, double> b);

#pragma endregion

/****************************************************************************************/
#pragma region print

void print_item(shared_ptr<Node> node, pair<int, int> o, int bin);
void print_leftover(shared_ptr<Node> node, pair<int, int> o, int bin);
void print_pattern(vector<shared_ptr<Node>> patterns);
void print_test(shared_ptr<Node> node, pair<int, int> o, int bin);

#pragma endregion


inline int Rand(int st, int ed) // return a random number ranging from 0 to x;
{
    return st + (int) ( ed * rand() / RAND_MAX);
}