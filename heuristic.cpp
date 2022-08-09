#include "heuristic.hpp"

#include "bounds.hpp"

#include <cmath>
#include <iostream>

using namespace std;


//---------------------------------------------------------
HSolution::HSolution(Instance& instance, Bins& bins):  _instance(instance), _bins(bins), to_insert(_instance.n_obj()), sol(_bins.bins.size()) {

}

//---------------------------------------------------------
void HSolution::init(vector<unsigned int>& new_sol) {
    
    sol = new_sol;

    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        to_insert[obj_ind] = _instance.objects[obj_ind].nb;
    }

    for(unsigned int bin_ind = 0; bin_ind < _bins.bins.size(); bin_ind ++) {
        if(sol[bin_ind] > 0) {
            for(auto& obj_ind: _bins.bins[bin_ind].objs) {
                to_insert[obj_ind] --;
            }

        }
    }

    n_remaining = 0;
    size_remaining = 0.;

    // min, max to_insert, nb_remaining, remaining length, feasibility
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {

        if(obj_ind == 0 || to_insert[obj_ind] < min_to_insert) {
            min_to_insert = to_insert[obj_ind];
        }

        if(obj_ind == 0 || to_insert[obj_ind] > max_to_insert) {
            max_to_insert = to_insert[obj_ind];
        }

        if(to_insert[obj_ind] > 0) {
            n_remaining ++;
            size_remaining += to_insert[obj_ind]*_instance.objects[obj_ind].size;
        }

    }

    n_bins = 0;
    for(unsigned int bin_ind = 0; bin_ind < sol.size(); bin_ind ++) {
        n_bins += sol[bin_ind];
    }

    feasible = (n_remaining == 0);

    cost = compute_cost();

}

//---------------------------------------------------------
double HSolution::compute_cost() {
    return n_bins + (!feasible)*50.;
}

//---------------------------------------------------------
void HSolution::update(unsigned int bin_ind, int delta) {

    sol[bin_ind] += delta;
    n_bins += delta;

    // delta > 0: some bins are added
    // delta < 0: some bins are removed

    for(auto& obj_ind: _bins.bins[bin_ind].objs) {

        to_insert[obj_ind] -= delta;
        size_remaining -= delta*_instance.objects[obj_ind].size;

        if(delta > 0) {
            // objects have been removed in to_insert
            if(to_insert[obj_ind] < min_to_insert) {
                min_to_insert = to_insert[obj_ind];
            }
            if(to_insert[obj_ind] + delta > 0 && to_insert[obj_ind] <= 0) {
                n_remaining --;
                if(n_remaining <= 0) {
                    feasible = true;
                }
            }
        } else if(delta < 0) {
            // objects have been added in to_insert
            if(to_insert[obj_ind] > max_to_insert) {
                max_to_insert = to_insert[obj_ind];
            }
            if(to_insert[obj_ind] + delta <= 0 && to_insert[obj_ind] > 0) {
                n_remaining ++;
                if(n_remaining > 0) {
                    feasible = false;
                }
            }
        }

    }

    // update the cost
    compute_cost();

}


//---------------------------------------------------------
void HSolution::display() {

    cout << "sol: ";
    for(auto& val: sol) {
        cout << val << ", ";
    }
    cout << endl;

    cout << "to_insert: ";
    for(auto& val: to_insert) {
        cout << val << ", ";
    }
    cout << "n bins: " << n_bins << endl;
    cout << "feasible ? " << (feasible ? "true" : "false") << endl;
    cout << "cost: " << cost << endl;

}

//---------------------------------------------------------
LocalSearch::LocalSearch(Instance& instance, Bins& bins): _instance(instance), _bins(bins) {

}


//---------------------------------------------------------
void LocalSearch::solve() {

    Bounds bounds(_instance);
    unsigned int best_val = bounds.best_fit();

    cout << "test 1" << endl;

    bool stop = false;

    vector<unsigned int> init_sol(_bins.bins.size(), 0);

    cout << "test 2" << endl;


    HSolution solution(_instance, _bins);

    cout << "test 3" << endl;


    solution.init(init_sol);

    cout << "test 4" << endl;


    solution.display();

    while(!stop) {

        stop = true;

    }


}