#pragma GCC optimize(2)
#pragma GCC optimize(3,"Ofast","inline")

#include <header.h>
#include <cstdio>
#include <iomanip>
#include <map>
#include <climits>
#include <assert.h>

using namespace std;

// Constants 
#define INF 1e9

// Parameters 
const double time_limit = 5000; // 5s 
const double evaluation_power = 1.2;
const int average_removed_nodes = 8; // average remove 4 nodes every ruin 
const int length_of_history = 2000;
double start_time;
int iteration_cnt = 0;
// IO Files
ofstream outfile;
ofstream Result;

/************************************************************************/
#pragma region Instance 

Rectangle bin;
vector<Item> items;
map<Rectangle, int> type;
int total_area = 0;

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
        if(!type[r]) {type[r] = ++t;}
        items.push_back(Item(type[r], r, j));
        total_area += r.area();
    }

    cerr << "Successfully build an instance from " << filename << endl;
}

#pragma endregion 

/************************************************************************/


#pragma region Solution

void print(Node* node)
{
    assert(node != NULL);
    cerr << node << " " << node->rec.w << " " << node->rec.h << " " << node->type << endl;
    if(node->children.empty()) return;
    for(auto child : node->children)
    {
        print(child);
    }
}

Solution recreate(Solution S, int mat_limit)
{
    Solution S_ = S.deep_copy();

    // cerr << S_.excluded_items.size() << endl;
    // for(auto item : S_.excluded_items)
    // {
    //     cerr << item.index << " ";
    // }
    // cerr << endl;

    int num_excluded = S_.excluded_items.size();
    vector<bool> E_;
    vector<bool> is_included; 
    for(int i = 0; i < num_excluded; i++) is_included.push_back(0), E_.push_back(0);
    while(num_excluded > 0)
    {
        vector<Node*> spaces;
        for(auto pattern : S_.patterns)
            extract_spaces(pattern, spaces);
        

        int chosen_index = -1, minimum_cnt = INF;
        for(int i = 0; i < S_.excluded_items.size(); i++)
        {
            if(is_included[i]) continue;
            int cnt = 0;
            for(auto space : spaces)
                cnt += space->rec.able_insert(S_.excluded_items[i].rectangle(0)) + space->rec.able_insert(S_.excluded_items[i].rectangle(1));
            if(cnt < minimum_cnt) minimum_cnt = cnt, chosen_index = i;
        }
        // the above process is to choose the most restricted item

        // then we can generate all the insertion options
        Item chosen_item = S_.excluded_items[chosen_index];
        
        priority_queue<Insertion, vector<Insertion>, greater<Insertion> > insertion_options;
        for(int i = 0; i < spaces.size(); i++)
        {
            Node* space = spaces[i];
            if(space->rec.able_insert(chosen_item.rectangle(0)))
            {
                double cost = min(vertical_cost(space, chosen_item, 0, evaluation_power), horizontal_cost(space, chosen_item, 0, evaluation_power));
                insertion_options.push(Insertion(i, chosen_index, cost, 0));
            }
            if(space->rec.able_insert(chosen_item.rectangle(1)))
            {
                double cost = min(vertical_cost(space, chosen_item, 1, evaluation_power), horizontal_cost(space, chosen_item, 1, evaluation_power));
                insertion_options.push(Insertion(i, chosen_index, cost, 1));
            }
        }

        // if the item can not be inserted into the current patterns
        if(insertion_options.empty() && mat_limit / bin.area() > S_.bin_number())
        {
            S_.add_new_bin(bin);
            Node *space = S_.patterns[S_.patterns.size() - 1];
            double cost = min(vertical_cost(space, chosen_item, 0, evaluation_power), horizontal_cost(space, chosen_item, 0, evaluation_power));
            insertion_options.push(Insertion(-1, chosen_index, cost, 0));
        }
        int blink = Rand(1, 100);
        while(blink <= 5 && insertion_options.size() > 1) blink = Rand(1, 100), insertion_options.pop(); //blinks
        
        is_included[chosen_index] = 1;
        num_excluded--;
        if(insertion_options.empty()) continue;

        
        Insertion chosen_insertion = insertion_options.top();
        Node* space = NULL;
        if(chosen_insertion.space_id == -1) 
            space = S_.patterns[S_.patterns.size() - 1];
        else space = spaces[chosen_insertion.space_id];
        //cerr<< "chosen index is " << chosen_item.index << endl;
        E_[chosen_index] = 1;
        insert(space, chosen_item, chosen_insertion.rotate);
    }

    vector<Item> tmp;
    for(int i = 0; i < E_.size(); ++i)
    {
        if(!E_[i]) tmp.push_back(S_.excluded_items[i]);
    }
    S_.excluded_items = tmp;
    
    return S_;
}

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

double vertical_cost(Node *space, Item item, bool rotation, double power)
{
    return  pow(space->area(), power)
            - pow(space->height() * (space->width() - item.width(rotation)), power) 
            - pow(item.width(rotation) * (space->height() - item.height(rotation)), power);
}

double horizontal_cost(Node *space, Item item, bool rotation, double power)
{
    return  pow(space->area(), power)
            - pow(space->width() * (space->height() - item.height(rotation)), power)
            - pow(item.height(rotation) * (space->width() - item.width(rotation)), power);
}

void insert(Node *space, Item item, bool rotate)
{
    // cerr << space->rec.w << " " << space->rec.h << endl;
    // if(rotate) cerr<< "rotate ";
    // else cerr << "not rotate ";
    // cerr << item.rec.w << " " << item.rec.h << endl;
    Rectangle item_shape = item.rectangle(rotate);
        /*
             Scenario 1: Part fits exactly into Node
             ---*****          ---*****             *       ->      *
                *   *             *$$$*
                *   *     ->      *$$$*
                *   *             *$$$*
             ---*****          ---*****

             -> node gets replaced by one node on same level
         */    
        
        if(item_shape.w == space->width() && item_shape.h == space->height())
        {
            space->type = item.index;
            return; 
        }
        /*
            Scenario 2: Part has same dimensions in the direction of the current cut
             ---*****          ---*****             *       ->      $   *
                *   *             *$* *
                *   *     ->      *$* *
                *   *             *$* *
             ---*****          ---*****

             -> node splits into 2 new nodes on same level
         */

        if(space->next_cut_orientation == 1 && item_shape.h == space->height() && item_shape.h != bin.h) 
        //current cut is performed vertically
        {
            Node* remain_node = new Node;
            remain_node->parent = space->parent;
            space->parent->children.push_back(remain_node);
            remain_node->next_cut_orientation = space->next_cut_orientation;
            remain_node->rec.w = space->width() - item_shape.w;
            remain_node->rec.h = space->height();
            space->rec.w = item_shape.w;
            space->type = item.index;
            return;
        }
        if(space->next_cut_orientation == 0 && item_shape.w == space->width() && item_shape.w != bin.w) 
        //current cut is performed horizontally
        {    
            Node* remain_node = new Node;
            remain_node->parent = space->parent;
            space->parent->children.push_back(remain_node);
            remain_node->next_cut_orientation = space->next_cut_orientation;
            remain_node->rec.h = space->height() - item_shape.h;
            remain_node->rec.w = space->width();
            space->rec.h = item_shape.h;
            space->type = item.index;
            return;
        }


        /*
             Scenario 3: Part fits exactly in opposite dimension of cut
             ---*****          ---*****             *       ->      * 
                *   *             *   *                            / \
                *   *     ->      *****                           $   *
                *   *             *$$$*
             ---*****          ---*****
         */
        
        if((space->next_cut_orientation == 1 && item_shape.w == space->width()) || item_shape.w == bin.w)
        //current cut is performed vertically
        {
            Node* left_node  = new Node;
            Node* right_node = new Node;
            
            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->next_cut_orientation = 1;
            space->type = -2;
        
            left_node->parent = space;
            left_node->next_cut_orientation  = !space->next_cut_orientation;
            left_node->rec.h = item_shape.h;
            left_node->rec.w = item_shape.w;
            left_node->type = item.index;

            right_node->parent = space;
            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->rec.h = space->height() - item_shape.h;
            right_node->rec.w = space->width();
            return;
        }

        if((space->next_cut_orientation == 0 && item_shape.h == space->height()) || item_shape.h == bin.h)
        //current cut is performed horizontally
        {
            Node* left_node  = new Node;
            Node* right_node = new Node;
            
            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->next_cut_orientation = 0;
            space->type = -2;
        
            left_node->parent = space;
            left_node->next_cut_orientation  = !space->next_cut_orientation;
            left_node->rec.h = item_shape.h;
            left_node->rec.w = item_shape.w;
            left_node->type = item.index;

            right_node->parent = space;
            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->rec.h = space->height();
            right_node->rec.w = space->width() - item_shape.w;
            return;
        }

        bool orientation = (vertical_cost(space, item, rotate, evaluation_power) < horizontal_cost(space, item, rotate, evaluation_power));
        //cerr << orientation << " " << space->next_cut_orientation << endl;
          /*
             Scenario 4: Part doesn't fit exactly in any dimension

             Scenario 4.1: First cut in same direction as current orientation
             ---*****          ---*****             *       ->      *   *
                *   *             * * *                            / \
                *   *     ->      *** *                           $   *
                *   *             *$* *
             ---*****          ---*****

             This requires an extra available level
         */

        if(space->next_cut_orientation == 1 && orientation == 0)
        //current cut is performed vertically
        {
            Node *neighbor = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            neighbor->parent = space->parent;
            neighbor->next_cut_orientation = space->next_cut_orientation;
            neighbor->rec.h = space->height();
            neighbor->rec.w = space->width() - item_shape.w;

            left_child->parent = space;
            left_child->next_cut_orientation = !space->next_cut_orientation;
            left_child->rec.h = item_shape.h;
            left_child->rec.w = item_shape.w;
            left_child->type = item.index;

            right_child->parent = space;
            right_child->next_cut_orientation = !space->next_cut_orientation;
            right_child->rec.h = space->height() - item_shape.h;
            right_child->rec.w = item_shape.w;

            space->parent->children.push_back(neighbor);
            space->children.push_back(left_child);
            space->children.push_back(right_child);
            space->rec.w = item_shape.w;
            space->type = -2;

            return; 
        }
        if(space->next_cut_orientation == 0 && orientation == 1)
        //currrent cut is performed horizontally
        {
            Node *neighbor = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            neighbor->parent = space->parent;
            neighbor->next_cut_orientation = space->next_cut_orientation;
            neighbor->rec.h = space->height() - item_shape.h;
            neighbor->rec.w = space->width();

            left_child->parent = space;
            left_child->next_cut_orientation = !space->next_cut_orientation;
            left_child->rec.h = item_shape.h;
            left_child->rec.w = item_shape.w;
            left_child->type = item.index;

            right_child->parent = space;
            right_child->next_cut_orientation = !space->next_cut_orientation;
            right_child->rec.h = item_shape.h;
            right_child->rec.w = space->width() - item_shape.w;

            space->parent->children.push_back(neighbor);
            space->children.push_back(left_child);
            space->children.push_back(right_child);
            space->rec.h = item_shape.h;
            space->type = -2;

            return; 
        }

        /*
             Scenario 4.2: First cut in opposite of current orientation
             ---*****          ---*****             *       ->      * 
                *   *             *   *                            / \
                *   *     ->      *****                           *   *
                *   *             *$* *                          / \
             ---*****          ---*****                         $   *

         */ 
        if(space->next_cut_orientation == 1 && orientation == 1)
        //current cut is performed vertically
        {
            Node *left_node = new Node;
            Node *right_node = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            left_child->parent = left_node;
            left_child->next_cut_orientation = space->next_cut_orientation;
            left_child->rec = item_shape;
            left_child->type = item.index;

            right_child->parent = left_node;
            right_child->next_cut_orientation = space->next_cut_orientation;
            right_child->rec.w = space->width() - item_shape.w;
            right_child->rec.h = item_shape.h;

            left_node->next_cut_orientation = !space->next_cut_orientation;
            left_node->parent = space;
            left_node->children.push_back(left_child);
            left_node->children.push_back(right_child);
            left_node->rec.h = item_shape.h;
            left_node->rec.w = space->width();
            left_node->type = -2;

            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->parent = space;
            right_node->rec.h = space->height() - item_shape.h;
            right_node->rec.w = space->width();

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;

            return;
        }
        if(space->next_cut_orientation == 0 && orientation == 0)
        //current cut is performed horizontally
        {
            Node *left_node = new Node;
            Node *right_node = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            left_child->parent = left_node;
            left_child->next_cut_orientation = space->next_cut_orientation;
            left_child->rec = item_shape;
            left_child->type = item.index;

            right_child->parent = left_node;
            right_child->next_cut_orientation = space->next_cut_orientation;
            right_child->rec.w = item_shape.w;
            right_child->rec.h = space->height() - item_shape.h;

            left_node->next_cut_orientation = !space->next_cut_orientation;
            left_node->parent = space;
            left_node->children.push_back(left_child);
            left_node->children.push_back(right_child);
            left_node->rec.h = space->height();
            left_node->rec.w = item_shape.w;
            left_node->type = -2;

            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->parent = space;
            right_node->rec.h = space->height();
            right_node->rec.w = space->width()  - item_shape.w;

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;

            return;
        }         
}

#pragma endregion 

/************************************************************************/

#pragma region ruin

Solution ruin(Solution S, int mat_limit, int average_nodes)
{
    Solution S_ = S.deep_copy();
    int max_bound = 2 * average_nodes;
    int _i = average_nodes == 0 ? 0 : (rand() % (max_bound - 1) + 1);
    //cerr<<_i <<endl;
    while(_i > 0 || S_.bin_number() * bin.area() >= mat_limit)
    {
        if(S_.bin_number() == 0) break;
        if(S_.bin_number() == S_.unchanged_number()) break;
        int choose_pattern_index = Rand(0, S_.patterns.size() - 1);
        vector<Node*> removable_nodes;
        // avoid changing full filled bins 
        while(S_.patterns[choose_pattern_index]->usage() == 1) 
            choose_pattern_index = Rand(0, S_.patterns.size() - 1);
        
        Node* pattern = S_.patterns[choose_pattern_index];
        get_all_removable_nodes(pattern, removable_nodes);
        int choose_node_index = Rand(0, removable_nodes.size() - 1);
        // cerr << endl;
        // cerr << "remove node type " << removable_nodes[choose_node_index] << endl;
        // cerr << "remove node " << removable_nodes[choose_node_index]->type << endl;
        // print(removable_nodes[choose_node_index]);
        
        remove_node(removable_nodes[choose_node_index], S_.excluded_items);
        if(pattern->type == -1) 
        {
            auto it = S_.patterns.begin();
            while(*it != pattern) it++;
            S_.patterns.erase(it);
            delete(pattern->parent);
            pattern->parent = NULL;
            delete(pattern);
            pattern = NULL;
        }
        _i--;
        if(S_.patterns.empty()) break;
    }
    return S_;
}

void get_all_removable_nodes(Node *pattern, vector<Node*> &removable_nodes)
{
    if(pattern->type == -1) return; 
    else if(pattern->type == -2)
    {
        removable_nodes.push_back(pattern);
        for(auto child : pattern->children)
        {
            get_all_removable_nodes(child, removable_nodes);
        }
    }
    else removable_nodes.push_back(pattern);
}

void remove_node(Node* node, vector<Item> &excluded_items)
{
    exclude(node, excluded_items);
    if(node->rec.w == bin.w && node->rec.h == bin.h) { node->type = -1; return;}
    Node* parent = node->parent;
    vector<Node*> neighbors = parent->children;
    Node* lneighbor = NULL;
    Node* rneighbor = NULL;
    for(int i = 0; i < neighbors.size(); i++)
    {
        if(neighbors[i] == node)
        {
            if(i > 0) lneighbor = neighbors[i - 1];
            if(i < neighbors.size() - 1) rneighbor = neighbors[i + 1];
            break;
        }
    }
    node->type = -1;
    node->children.clear();
    if(lneighbor != NULL) merge_two_nodes(lneighbor, node);
    if(rneighbor != NULL) merge_two_nodes(node, rneighbor);
    if(node->rec.w == parent->rec.w && node->rec.h == parent->rec.h) 
    {
        auto it = parent->children.begin();
        while(*it != node) it++;
        parent->children.erase(it);
        delete(node);
        node = NULL;
        parent->type = -1;
    }
}

void merge_two_nodes(Node* master, Node* slave)
{
    if(master->type != -1 || slave->type != -1) return;
    if(master->next_cut_orientation == 0)
    //current cut is performed horizontally
    //merge height
        master->rec.h += slave->rec.h;
    else //merge width
        master->rec.w += slave->rec.w;
    auto it = master->parent->children.begin();
    while(*it != slave)  it++;
    master->parent->children.erase(it);
    delete(slave);
    slave = NULL;
}

void exclude(Node* root, vector<Item> &excluded_items)
{
    //cerr << "bug occur" << root->type << endl;
    if(root->type == -1) return;
    else if(root->type == -2) 
    {
        for(auto child : root->children)
        {
            exclude(child, excluded_items);
        }
    }
    else 
    { 
        excluded_items.push_back(items[root->type - 1]);
    }
}


#pragma endregion 


/************************************************************************/


#pragma region Local Search

Solution local_search(Solution S, int mat_limit, int history_length, Solution S_best)
{
    vector<Solution> S_history;
    for(int i = 0; i < history_length; ++i)
        S_history.push_back(S.deep_copy());
    int iteration = 0; 
    Solution S_local_optimum = S.deep_copy();

    while(clock() - start_time < time_limit)
    {
        //cerr << clock() - start_time << " " << iteration << " " << S_best.bin_number() << " " << mat_limit << endl;
        //cout << S_local_optimum.excluded_items.size() << " " << S_local_optimum.bin_number() << endl;
        Solution S_;
        S_ = ruin(S_local_optimum, mat_limit, average_removed_nodes);
        S_ = recreate(S_, mat_limit);
        iteration_cnt++;
        int index = iteration % history_length;
        if(S_.dominate(S_history[index], evaluation_power) || S_.dominate(S_local_optimum, evaluation_power))
        {
            // cout << "local optimum is modified " << S_local_optimum.excluded_items.size() << endl;
            // if(S_local_optimum.excluded_items.size()) cout << " " << S_local_optimum.excluded_items[0].index << endl;
            S_local_optimum = S_;
            if(S_local_optimum.dominate(S_history[index], evaluation_power))
                S_history[index] = S_local_optimum;
            iteration++;
        }
        if(S_local_optimum.excluded_items.empty())
        {
            //cout << "best solution is modified " << S_best.bin_number() << " " << S_local_optimum.bin_number() << endl;
            //cerr << "value " << S_best.value(evaluation_power) << " " << S_local_optimum.value(evaluation_power) << endl; 
            S_best = S_local_optimum;
            mat_limit = S_best.bin_number() * bin.area();
            Solution S_next;
            S_next = ruin(S_best, mat_limit, 0);
            S_best = local_search(S_next, mat_limit, history_length, S_best);
        }
    }
    return S_best;
}



#pragma endregion



/************************************************************************/

#pragma region Print

int cnt = 0;

void print_item(Node *node, Coord o, int bin)
{
    //assert(cnt <= items.size());
    if(node->type == -1) return;
    if(node->type > 0) { cnt++; outfile << node->type << " " <<  bin << " " << node->width() << " " << node->height() << " " <<  o.x << " " << o.y << endl;}
    for(auto child : node->children)
    {
        if(node->next_cut_orientation == 0) print_item(child, o, bin), o.x += child->width();
        else print_item(child, o, bin), o.y += child->height(); 
    }
}

void print_leftover(Node *node, Coord o, int bin)
{
    if(node->type > 0) return;
    if(node->type == -1) {outfile << node->type << " " <<  bin << " " << node->width() << " " << node->height() << " " <<  o.x << " " << o.y << endl;}
    for(auto child : node->children)
    {
        if(node->next_cut_orientation == 0) print_leftover(child, o, bin), o.x += child->width();
        else print_leftover(child, o, bin), o.y += child->height(); 
    }
}

void print_pattern(vector<Node*> patterns)
{
    for(int i = 0; i < patterns.size(); i++)
    {
        print_item(patterns[i], Coord(0, 0), i);
    }
    outfile << endl;
    for(int i = 0; i < patterns.size(); i++)
    {
        print_leftover(patterns[i], Coord(0, 0), i);
    }
}

void print_solution(Solution best_solution)
{
    outfile << bin.w << " " << bin.h << " ";
    outfile << best_solution.bin_number() << endl;
    print_pattern(best_solution.patterns);
}

#pragma endregion

/************************************************************************/


int main(int argc, char* argv[])
{
    start_time = clock();
    build_instance(argv[1]);
    
    srand(time(NULL));
    //string outfile_name = "Solution/Sol_";
    string outfile_name = "Test/GDRR_";
    string infile_name = std::string(argv[1]);
    if(infile_name[0] != 't') 
        infile_name = infile_name.substr(9);
    outfile_name += infile_name;
    outfile.open(outfile_name.c_str());


    int mat_limit = bin.area() * items.size();
    Solution S, best_solution;
    S.excluded_items = items;
    S.total_area = total_area;

    //initial_solution = recreate(S, mat_limit);
    //print_solution(initial_solution);

    best_solution = local_search(S, mat_limit, length_of_history, S);
    Result.open("result.csv", std::ios::app);
    
    Result << best_solution.bin_number() << endl;
    print_solution(best_solution);
    // cerr << iteration_cnt << endl;

    //cerr << best_solution.value(evaluation_power) << endl;
    //cerr << initial_solution.patterns[0]->usage() << endl;
    //print(initial_solution.patterns[11]);
    // while(clock() - start_time < time_limit)
    // {
    //     S = ruin(S, mat_limit, average_removed_nodes);
    //     S = recreate(S, mat_limit);
    //     iteration_cnt++;
    // }
    cerr << iteration_cnt << endl;
    outfile.close();
    Result.close();
    return 0;
}
