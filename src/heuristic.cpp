#include "heuristic.hpp"

#include "bounds.hpp"

#include <cmath>
#include <iostream>

using namespace std;


//---------------------------------------------------------
HSolution::HSolution(Instance& instance, Bins& bins):  _instance(instance), _bins(bins), to_insert(_instance.n_obj()), sol(_bins.bins.size()) {

}

//---------------------------------------------------------
void HSolution::init(vector<int>& new_sol) {
    
    sol = new_sol;

    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        to_insert[obj_ind] = _instance.objects[obj_ind].nb;
    }

    for(unsigned int bin_ind = 0; bin_ind < _bins.bins.size(); bin_ind ++) {
        if(sol[bin_ind] > 0) {
            for(auto& obj_ind: _bins.bins[bin_ind].objs) {
                to_insert[obj_ind] -= sol[bin_ind];
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
    return static_cast<double>(n_bins) + 2.5*static_cast<double>(n_remaining);
}

//---------------------------------------------------------
void HSolution::update(unsigned int bin_ind, int delta) {

    sol[bin_ind] += delta;
    
    if(sol[bin_ind] <= 0 && sol[bin_ind]-delta <= 0) {
        delta = 0;
    } else {
        if(sol[bin_ind]-delta <= 0 && sol[bin_ind] >= 0) { // switch from negative to positive value
            delta += sol[bin_ind]-delta;
        } else if(sol[bin_ind]-delta >= 0 && sol[bin_ind] <= 0) { // switch from positive to negative value
            delta -= sol[bin_ind];
        }
    }

    if(delta == 0) {
        return;
    }
    
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
    cost = compute_cost();

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
    cout << endl;
    cout << "n bins: " << n_bins << endl;
    cout << "n remaining: " << n_remaining << endl;
    cout << "feasible ? " << (feasible ? "true" : "false") << endl;
    cout << "cost: " << cost << endl;
    cout << "cost recomputed: " << n_bins + n_remaining << endl;

}

//---------------------------------------------------------
HSolution& HSolution::operator=(const HSolution& other) {
    
    to_insert = other.to_insert;
    max_to_insert = other.max_to_insert;
    min_to_insert = other.min_to_insert;

    n_remaining = other.n_remaining;
    size_remaining = other.size_remaining;

    sol = other.sol;
    n_bins = other.n_bins;
    cost = other.cost;
    feasible = other.feasible;

    return *this;
}

//---------------------------------------------------------
LocalSearch::LocalSearch(Instance& instance, Bins& bins): _instance(instance), _bins(bins) {

}


//---------------------------------------------------------
void LocalSearch::solve() {

    Bounds bounds(_instance);

    vector<int> init_sol(_bins.bins.size(), 0);

    vector<Bin> best_fit_sol;
    unsigned int best_val = bounds.best_fit(&best_fit_sol);
    cout << "best fit sol size: " << best_fit_sol.size() << endl;
    for(auto& bin: best_fit_sol) {
        unsigned int bin_ind = _bins.identify_bin(bin);
        init_sol[bin_ind] ++;
    }

    HSolution solution(_instance, _bins);
    solution.init(init_sol);
    solution.display();

    HSolution best_sol(_instance, _bins);
    best_sol = solution;

    // strategy: remove as many bins as possible until removing any of the bins make the solution infeasible
    // then re-add multiple bins to start from a new point

    // randomization in removing the bins

    // randomization in adding new bins

    bool stop = false;
    while(!stop) {

        bool improvement = false;
        unsigned int best_bin = 0;
        double best_cost = solution.cost;

        for(unsigned int bin_ind = 0; bin_ind < _bins.bins.size(); bin_ind ++) {

        }

        if(improvement && best_cost < solution.cost) {
            
        } else {
            stop = true;
        }

        solution.display();
        cout << endl << endl;

        // stop = true;

    }

    cout << "best cost: " << solution.cost << endl;
    solution.display();

}