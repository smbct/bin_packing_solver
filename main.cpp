#include <iostream>

#include "instance.hpp"

#include "bins.hpp"

#include "solver.hpp"

using namespace std;


int main() {

    cout << "hello bin packing" << endl;


    Instance instance;
    // instance.loadFromFile("instances/A/jouet1.dat");
    // instance.loadFromFile("instances/A/A4.dat");

    instance.loadFromFile("instances/A/A10.dat");


    instance.display();

    Bins bins(instance);
    bins.enumerate();
    bins.display();

    Solver solver(instance);
    unsigned int heuristic_value = solver.best_fit();
    cout << "heuristic value: " << heuristic_value << endl;

    unsigned int linear_relaxation = solver.linear_relaxation();
    cout << "linear relaxation: " << linear_relaxation << endl;

    double optimal_value = solver.linear_relaxation_glpk(heuristic_value);
    cout << "linear relaxation with glpk: " << optimal_value << endl;

    optimal_value = solver.linear_relaxation_glpk_v2(bins);
    cout << "linear relaxation with glpk v2: " << optimal_value << endl;


    return 0;
}