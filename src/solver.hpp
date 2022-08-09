#pragma once

#include "instance.hpp"
#include "bins.hpp"

#include <list>

class Solver {

    public:

        // constructor
        Solver(Instance& instance);

        // update the solution variables after adding a bin
        void update_sol_add(Bin& bin, std::vector<int>& to_insert, int& n_obj_rem, double& length_rem);

        // update the solution variables after removing a bin
        void update_sol_remove(Bin& bin, std::vector<int>& to_insert, int& n_obj_rem, double& length_rem);

        // look for a "useful" bin to add in the solution considering remaining objects
        unsigned int identify_next_bin(std::vector<int>& to_insert, Bins& bins, unsigned int starting_bin);

        // rest if the object demands may be fullfilled considering a partial assignment
        bool test_remaining_objects(std::vector<int>& to_insert, Bins& bins, unsigned int remaining_bins, unsigned int bin_index);

        // solve the instance using the bins enumeration
        unsigned int solve_bins(Bins& bins);

        // solve the instance using glpk and the bins
        unsigned int solve_glpk_bins(Bins& bins);


    private:

        Instance& _instance;

};