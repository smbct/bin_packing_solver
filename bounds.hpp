#pragma once

#include "instance.hpp"
#include "bins.hpp"

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

    private:

        Instance& _instance;


};