#pragma once

#include "instance.hpp"

// a full bin
class Bin {

    public:
        Bin() { }
        std::vector<int> objs; // objects in the bin
        std::vector<int> objs_occ; // n occurrences for each objects
        int size; // size filled in the bin

};

class Bins {

    public:

        // default constructor
        Bins(Instance& instance);

        // enumerate all the possible bins
        void enumerate();

        // get the number of bins
        unsigned int n_bins();

        // display the bins
        void display();

        // return the index of the bin corresponding to the bin given in argument
        unsigned int identify_bin(Bin& bin);

    public:

        std::vector<Bin> bins;

    private:

        Instance& _instance;

};