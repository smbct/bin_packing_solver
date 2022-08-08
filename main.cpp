#include <iostream>

#include "instance.hpp"

#include "bins.hpp"

#include "solver.hpp"

#include <cmath>

using namespace std;

#include "bounds.hpp"

int main() {

    cout << "hello bin packing" << endl;


    Instance instance;
    // instance.loadFromFile("instances/A/jouet1.dat");
    // instance.loadFromFile("instances/A/A4.dat");

    // instance.loadFromFile("instances/A/A10.dat");

    instance.loadFromFile("instances/A/A6.dat");


    instance.display();

    Bins bins(instance);
    bins.enumerate();
    bins.display();
    // cout << endl;

    Bounds bounds(instance);

    unsigned int heuristic_value = bounds.best_fit();
    cout << "heuristic value: " << heuristic_value << endl << endl;

    double linear_relaxation = bounds.linear_relaxation();
    cout << "linear relaxation: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;

    double optimal_value = bounds.linear_relaxation_glpk(heuristic_value);
    cout << "linear relaxation with glpk: " << optimal_value << " (" << ceil(optimal_value) << ")" << endl << endl;

    optimal_value = bounds.linear_relaxation_glpk_v2(bins);
    cout << "linear relaxation with glpk v2: " << optimal_value << " (" << ceil(optimal_value) << ")" << endl << endl;


    Solver solver(instance);
    unsigned int best_val = solver.solve_bins(bins);
    cout << "optimal value: " << best_val << endl;

    return 0;
}