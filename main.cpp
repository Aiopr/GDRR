#include <cstdio>
#include <vector>
#include <map>
#include <iomanip>
#include <gdrr.h>

using namespace std;

//Parameters:
double time_limit;
int average_nodes_removed;
double evaluation_power;

#pragma region data structure for instance 

Rectangle bin;
vector<Item> items;
map<Rectangle, int> type;
int total_area = 0;

#pragma endregion


void build_instance(const char *filename)
{
    ifstream infile(filename);
    int n;
    int j, w, h, d, b, p;
    int t = 0;
    Rectangle r;
    
    if(!infile.is_open()) cerr << "cannot open " << filename << endl;
    
    infile >> n >> bin.w >> bin.h;

    for (int i = 0; i < n; ++i)
    {
        infile >> j >> w >> h >> d >> b >> p; 
        if(w < h) r.w = h, r.h = w; 
        else r.w = w, r.h = h;  // guarantee that w > h for every item
        if(!type[r]) items.push_back(Item(t, r, 1)), type[r] = t++;
        else items[type[r]].num++;
        total_area += r.area();
    }

    cerr << "Successfully build an instance from " << filename << endl;
}


// Removable children: items and structure nodes
// void get_removable_children(Node *node, vector<Node*> &removable_children)
// {
//     if(node->children.empty())  return;
//     for(auto child : node->children)
//     {
//         if(child->type == -1) continue; // skip all the leftover nodes
//         removable_children.push_back(child);
//         get_removable_children(child, removable_children);
//     }
// }


// Solution ruin (Solution S, int mat_limit, int mu)
// {
//     int _i = Rand(1, 2 * mu - 1);
//     while(_i > 0 || S.included_area() >= mat_limit)
//     {
//         // randomly choose a pattern
//         int chosen_pattern_index = Rand(0, S.patterns.size() - 1);
//         Node *chosen_pattern = S.patterns[chosen_pattern_index];
        
//         // store the items and structure nodes from the chosen_pattern
//         vector<Node*> removable_children;
//         get_removable_children(chosen_pattern, removable_children);

//         //randomly delete a node from the chose pattern
//         int chosen_node_index = Rand(0, removable_children.size() - 1);
//         remove_node(removable_children[chosen_node_index]);
//     }
// }


// Solution recreate(Solution S, int mat_limit)
// {
//     vector<Item> not_included_items = S.excluded_items;
//     // while(!not_included_items.empty())
//     // {

//     // }
// }



int main()
{
        build_instance("test.ins2D");
        // //srand(time(NULL));
        // fstream outfile("test.out");
        // outfile << bin.w << " " << bin.h << endl;
        // for (int i = 0; i < items.size(); ++i)
        // {
        //     outfile << setw(3) << items[i].type << "th item "
        //     << setw(3) <<items[i].rec.w << " " << setw(3) << items[i].rec.h
        //     << " has " << setw(3) << items[i].num << " copies" << endl;
        // }
        // outfile.close();


        return 0;
}