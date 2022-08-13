#include "knapsack.hpp"

#include <iostream>
#include <list>

using namespace std;

//---------------------------------------------------------
Knapsack::Knapsack() {

}

//---------------------------------------------------------
double Knapsack::solve(vector<double>& values, vector<int>& weights, int size, vector<int>& sol) {

    // duplicates the objects such that no additional object of the same type can be added without exceeding the capacity
    vector<int> obj_indexes;
    {
        unsigned int obj_ind = 0;
        int w = 0;
        while(obj_ind < weights.size()) {
            w += weights[obj_ind];
            if(w < size) {
                obj_indexes.push_back(obj_ind);
            } else {
                w = 0;
                obj_ind ++;
            }
        }
    }

    vector<double> values_dup(obj_indexes.size(), 0);
    vector<int> weights_dup(obj_indexes.size(), 0);
    int dup_ind = 0;
    for(auto& obj_ind: obj_indexes) {
        values_dup[dup_ind] = values[obj_ind];
        weights_dup[dup_ind] = weights[obj_ind];
        dup_ind ++;
    }
    
    vector<list<int>> sol2(size+1, list<int>());

    vector<double> dp(size+1, 0.);
    for(unsigned int obj_ind = 0; obj_ind < weights_dup.size(); obj_ind ++) {
        for(int w = size; w >= 0; w --) {
            if(weights_dup[obj_ind] <= w) {
                if(dp[w-weights_dup[obj_ind]] + values_dup[obj_ind] > dp[w]) {
                    dp[w] = dp[w-weights_dup[obj_ind]] + values_dup[obj_ind];
                    sol2[w] = sol2[w-weights_dup[obj_ind]];
                    sol2[w].push_back(obj_ind);
                }   
            }
        }

    }

    sol.resize(weights.size());

    for(auto& val: sol2[size]) {
        sol[obj_indexes[val]] ++;
    }

    // debug
    // int occ_size = 0;
    // for(unsigned int obj_ind = 0; obj_ind < sol.size(); obj_ind ++) {
    //     occ_size += weights[obj_ind]*sol[obj_ind];
    //     cout << sol[obj_ind] << ": " << weights[obj_ind] << ", ";
    // }
    // cout << endl;
    // cout << "occupied size opt kp: " << occ_size << " over " << size << endl;

    return dp[size];
}

//---------------------------------------------------------
void Knapsack::test() {

    // vector<double> values = {1./5., 1./2., 1./2.};
    // vector<int> weights = {3, 6, 7};

    vector<double> values = {1./5., 2./5., 1./2.};
    vector<int> weights = {3, 6, 7};

    int size = 16;

    vector<int> solution;

    double optimal_value = solve(values, weights, size, solution);

    cout << "optimal value: " << 1.-optimal_value << endl;

    cout << "solution: ";
    for(auto& val: solution) {
        cout << val << ", ";
    }
    cout << endl;

    // objects are ordered
    // precompute the optimal values of subset of objects

    // add or not an object for a given remaining capacity

    

}