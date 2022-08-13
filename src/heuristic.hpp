#pragma once

#include "instance.hpp"
#include "bins.hpp"

#include <list>

// data structure to store a solution for the heuristic search
class HSolution {
    
    public:

        // constructor
        HSolution(Instance& instance, Bins& bins);

        // initialize the data structure
        void init(std::vector<int>& new_sol);

        // update the solution by incrementing or decrementing the number of occurrences of a bin
        void update(unsigned int bin_ind, int delta);

        // compute the cost from the variables
        double compute_cost();

        // display the solution
        void display();

        // copy
        HSolution& operator=(const HSolution&);

    private:

        // instance data structure
        Instance& _instance;
        Bins& _bins;

    public:

        std::vector<int> to_insert; // number of objects not inserted
        int max_to_insert; // maximum value of to_insert vector
        int min_to_insert; // minimum value of to_insert vector

        int n_remaining; // number of object types which demand is not fulfilled
        double size_remaining; // remaining total object sizes to meet the demand

        std::vector<int> sol;
        unsigned int n_bins; // number of bins in the solution
        double cost; // solution cost
        bool feasible;

    
};




// implementation of a heuristic search for the bin packing problem
class HeuristicSearch {

    public:

        // constructor
        HeuristicSearch(Instance& instance, Bins& bins);

        // add bins to the solution
        void add_bins(HSolution& sol);

        // remove bins from a feasible solution until removing any bin makes it unfeasible
        void remove_bins(HSolution& solution, HSolution &best_sol);

        // solving function
        unsigned int solve();

    private:

        Instance& _instance;
        Bins& _bins;

};