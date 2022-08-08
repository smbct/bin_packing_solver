#pragma once

#include "instance.hpp"

#include "bins.hpp"

class Solver {

    public:

        // constructor
        Solver(Instance& instance);

        

        void update_sol_add(Bin& bin, std::vector<int>& to_insert, int& n_obj_rem, double& length_rem);

        void update_sol_remove(Bin& bin, std::vector<int>& to_insert, int& n_obj_rem, double& length_rem);

        unsigned int solve_bins(Bins& bins);

    private:

        Instance& _instance;

};