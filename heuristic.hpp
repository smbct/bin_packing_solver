#pragma once

#include "instance.hpp"
#include "bins.hpp"

// implementation of a local search for the bin packing problem
class LocalSearch {

    public:

        // constructor
        LocalSearch(Instance& instance, Bins& bins);

        // solving function
        void solve();

    private:

        Instance& _instance;
        Bins& _bins;

};