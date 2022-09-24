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
    int _t = 0;
    Rectangle r;
    
    if(!infile.is_open()) cerr << "cannot open " << filename << endl;
    
    infile >> n >> bin.w >> bin.h;

    for (int i = 0; i < n; ++i)
    {
        infile >> j >> w >> h >> d >> b >> p; 
        if(w < h) r.w = h, r.h = w; 
        else r.w = w, r.h = h;  // guarantee that w > h for every item
        if(!type[r]) items.push_back(Item(_t, r, 1)), type[r] = _t++;
        else items[type[r]].num++;
        total_area += r.area();
    }

    cerr << "Successfully build an instance from " << filename << endl;
}

int main()
{
    cout << type[Rectangle(1, 1)] <<endl;
    //build_instance("test.ins2D");
    cout << "Hello, world!" << endl;
}