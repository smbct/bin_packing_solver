#include <iostream>

#include "instance.hpp"

#include "bins.hpp"

#include "solver.hpp"

using namespace std;


int main() {

    cout << "hello bin packing" << endl;


    Instance instance;
    // instance.loadFromFile("instances/A/jouet1.dat");
    instance.loadFromFile("instances/A/A4.dat");

    instance.display();

    // Bins bins(instance);
    // bins.enumerate();
    // bins.display();

    Solver solver(instance);
    unsigned int heuristic_value = solver.best_fit();
    
    cout << "heuristic value: " << heuristic_value << endl;

    unsigned int linear_relaxation = solver.linear_relaxation();

    cout << "linear relaxation: " << linear_relaxation << endl;


    return 0;
}