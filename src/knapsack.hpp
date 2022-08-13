#pragma once

#include <vector>

// a simple knapsack solver based on dynamic programming
class Knapsack {

    public:

        // constructor
        Knapsack();

        // solve the instance
        double solve(std::vector<double>& values, std::vector<int>& weights, int size, std::vector<int>& sol);

        // test the solver
        void test();

    private:


};