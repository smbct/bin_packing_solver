#include "solver.hpp"
#include "bins.hpp"
#include "bounds.hpp"

#include <algorithm>

#include <cmath>

#include <iostream>

#include <glpk.h>

#include <set>

#include <stack>
#include <list>

using namespace std;

//---------------------------------------------------------
Solver::Solver(Instance& instance) : _instance(instance) {

}

//---------------------------------------------------------
void Solver::update_sol_add(Bin& bin, vector<int>& to_insert, int& n_obj_rem, double& length_rem) {
    
    // update the values with the bin
    for(auto& obj_ind: bin.objs) {
        if(to_insert[obj_ind] == 1) {
            n_obj_rem --;
        }
        length_rem -= _instance.objects[obj_ind].size;
        to_insert[obj_ind] --;
    }
}

//---------------------------------------------------------
void Solver::update_sol_remove(Bin& bin, vector<int>& to_insert, int& n_obj_rem, double& length_rem) {
    
    // update the values with the bin
    for(auto& obj_ind: bin.objs) {
        if(to_insert[obj_ind] == 0) {
            n_obj_rem ++;
        }
        length_rem += _instance.objects[obj_ind].size;
        to_insert[obj_ind] ++;
    }
}


//---------------------------------------------------------
unsigned int Solver::solve_bins(Bins& bins) {

    // tree search on the bin patterns

    vector<int> to_insert(_instance.n_obj(), 0);
    for(unsigned int obj_ind = 0; obj_ind < to_insert.size(); obj_ind ++) {
        to_insert[obj_ind] += _instance.objects[obj_ind].nb;
    }

    int n_obj_rem = _instance.n_obj();
    double length_rem = 0.; // linear relaxation
    for(unsigned int obj_ind = 0; obj_ind < to_insert.size(); obj_ind ++) {
        length_rem += _instance.objects[obj_ind].nb*_instance.objects[obj_ind].size;
    }

    list<unsigned int> sol;
    sol.push_back(0); // add the first bin

    // update the values with the first bin
    update_sol_add(bins.bins[0], to_insert, n_obj_rem, length_rem);

    Bounds bounds(_instance);

    unsigned int best_val = bounds.best_fit();

    unsigned int max_depth = best_val;

    bool stop = false;

    unsigned int n_nodes = 0;

    while(!stop) {

        n_nodes ++;

        // if(n_nodes % 1000 == 0) {
        //     cout << "n nodes" << n_nodes << endl;
        //     cout << "current best: " << best_val << endl;
        // }

        bool backtrack = false;

        cout << n_obj_rem << endl;
        cout << "current sol: ";
        for(auto& elt: sol) {
            cout << elt << ", ";
        }
        cout << endl;

        if(n_obj_rem > 0) { // exploration part

            if(sol.size() < max_depth) {

                // bounding

                // linear relaxation v1
                double lower_bound = static_cast<unsigned int>(sol.size())+ceil(length_rem/_instance.bin_size);

                if(lower_bound >= best_val) {
                    backtrack = true;
                    
                } else { // exploration, add a new bin
                    sol.push_back(sol.back());
                    update_sol_add(bins.bins[sol.back()], to_insert, n_obj_rem, length_rem);
                }

            } else {
                backtrack = true;
            }

        } else {

            cout << sol.size() << endl;

            // a solution is found
            if(sol.size() < best_val) {
                best_val = static_cast<unsigned int>(sol.size());
                max_depth = best_val; // update the maximum depth

                cout << "new best: " << best_val << endl;

            }

            backtrack = true;
        }


        if(backtrack) {
            while(backtrack) {

                if(sol.size() > 0) { // remove the last added

                    unsigned int last_added = sol.back();
                    
                    sol.pop_back();
                    update_sol_remove(bins.bins[last_added], to_insert, n_obj_rem, length_rem);

                    if(last_added < bins.bins.size()-1) {
                        sol.push_back(last_added+1);
                        update_sol_add(bins.bins[last_added+1], to_insert, n_obj_rem, length_rem);
                        backtrack = false;
                    }

                } else {

                    backtrack = false;
                    stop = true; // the search is terminated
                
                }
                
            }

        }

    }

    cout << "n nodes explored: " << n_nodes << endl;

    return best_val;

}