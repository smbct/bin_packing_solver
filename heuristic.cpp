#include "heuristic.hpp"

#include "bounds.hpp"

#include <cmath>
#include <iostream>

using namespace std;

//---------------------------------------------------------
LocalSearch::LocalSearch(Instance& instance, Bins& bins): _instance(instance), _bins(bins) {

}

//---------------------------------------------------------
void update_sol_add(vector<int>& to_insert, unsigned int bin_ind, int delta) {

}

//---------------------------------------------------------
void LocalSearch::solve() {

    vector<int> to_insert(_instance.n_obj(), 0);
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        to_insert[obj_ind] = _instance.objects[obj_ind].nb;
    }

    int max_to_insert = -1;
    int min_to_insert = -1;

    Bounds bounds(_instance);
    unsigned int best_val = bounds.best_fit();

    // first assignment: equal number of bins
    vector<unsigned int> sol(_bins.bins.size(), 0);
    unsigned int n_bins_init = static_cast<unsigned int>(ceil(static_cast<double>(best_val)/static_cast<double>(sol.size())));

    // init the to_insert array
    for(unsigned int bin_ind = 0; bin_ind < _bins.bins.size(); bin_ind ++) {
        for(auto& elt: _bins.bins[bin_ind].objs) {
            to_insert[elt] -= sol[bin_ind];
        }
    }

    cout << "to_insert init: ";
    for(auto& elt: to_insert) {
        cout << elt << ", ";
    }
    cout << endl;

}