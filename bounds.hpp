#pragma once

#include "instance.hpp"
#include "bins.hpp"

#include <glpk.h>

// functions for bounding the bin packing problem
class Bounds {

    public:

        // constructor
        Bounds(Instance& instance);

        // best fit heuristic
        unsigned int best_fit();

        // linear relaxation
        double linear_relaxation();

        // linear relaxation v2
        double linear_relaxation_v2(Bins& bins);

        // linear relaxation (glpk)
        double linear_relaxation_glpk(unsigned int upper_bound);

        // linear relaxation (glpk)
        double linear_relaxation_glpk_v2(Bins& bins);

        // init the glp_prob structure for the relaxation problem
        void init_glpk_relaxation_v2(glp_prob* prob, Bins& bins);

        // update the relaxation glpk problem
        void update_glpk_relaxation_v2(glp_prob* prob, Bins& bins, std::vector<int>& to_insert, unsigned int starting_bin);

        // solve the already initialized glpk relaxation problem
        double solve_glpk_relaxation_v2(glp_prob* prob);

    private:

        Instance& _instance;


};