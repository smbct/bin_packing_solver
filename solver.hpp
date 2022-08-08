#pragma once

#include "instance.hpp"

#include "bins.hpp"

class Solver {

    public:

        // constructor
        Solver(Instance& instance);

        // best fit heuristic
        unsigned int best_fit();

        // linear relaxation
        unsigned int linear_relaxation();

        // linear relaxation (glpk)
        unsigned int linear_relaxation_glpk(unsigned int upper_bound);

        // linear relaxation (glpk)
        unsigned int linear_relaxation_glpk_v2(Bins& bins);

    private:

        Instance& _instance;

};