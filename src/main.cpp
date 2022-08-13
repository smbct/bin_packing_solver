#include <iostream>
#include <cmath>
#include <sstream>
#include <ctime>

#include "instance.hpp"
#include "bins.hpp"
#include "solver.hpp"
#include "bounds.hpp"
#include "heuristic.hpp"

using namespace std;


//---------------------------------------------------------
void expe() {

    for(unsigned int ind = 4; ind <= 10; ind ++) {

        // string filename = "instances/A/A";
        string filename = "instances/B/B";


        ostringstream ss;
        ss << ind;
        filename += ss.str();

        filename += ".dat";


        cout << endl << endl;
        cout << "************************************" << endl;
        cout << "Instance " << "\"" << filename << "\"" << endl;
        cout << "************************************" << endl;
        cout << endl;

        Instance instance;
        instance.loadFromFile(filename);


        instance.display();

        Bins bins(instance);
        bins.enumerate();
        // bins.display();
        // cout << endl;

        Bounds bounds(instance);

        unsigned int heuristic_value = bounds.best_fit();
        cout << "heuristic value: " << heuristic_value << endl << endl;

        double linear_relaxation = bounds.linear_relaxation();
        cout << "linear relaxation: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;

        linear_relaxation = bounds.linear_relaxation_glpk(heuristic_value);
        cout << "linear relaxation with glpk: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;

        linear_relaxation = bounds.linear_relaxation_glpk_v2(bins);
        cout << "linear relaxation with glpk v2: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;


        Solver solver(instance);

        unsigned int optimal_value = solver.solve_glpk_bins(bins);
        
        cout << "optimal solution with glpk v2: " << optimal_value << endl << endl;


        unsigned int best_val = solver.solve_bins(bins);
        cout << "optimal value with own solver: " << best_val << endl;

    }
}


//---------------------------------------------------------
int main() {

    cout << "hello bin packing" << endl;

    // random seed init
    srand(42);

    // expe();
    // return 0;

    Instance instance;
    // instance.loadFromFile("instances/A/jouet1.dat");
    // instance.loadFromFile("instances/A/A4.dat");

    instance.loadFromFile("instances/A/A10.dat");

    // instance.loadFromFile("instances/A/A5.dat");

    // instance.loadFromFile("instances/A/A7.dat");

    instance.display();

    Bins bins(instance);
    bins.enumerate();
    bins.display();
    cout << endl;

    Bounds bounds(instance);

    unsigned int heuristic_value = bounds.best_fit();
    cout << "heuristic value: " << heuristic_value << endl << endl;

    double linear_relaxation = bounds.linear_relaxation();
    cout << "linear relaxation: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;

    linear_relaxation = bounds.linear_relaxation_glpk(heuristic_value);
    cout << "linear relaxation with glpk: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;

    linear_relaxation = bounds.linear_relaxation_glpk_v2(bins);
    cout << "linear relaxation with glpk v2: " << linear_relaxation << " (" << ceil(linear_relaxation) << ")" << endl << endl;


    // Solver solver(instance);

    // unsigned int optimal_value = solver.solve_glpk_bins(bins);
    // cout << "optimal solution with glpk v2: " << optimal_value << endl << endl;


    // unsigned int best_val = solver.solve_bins(bins);
    // cout << "optimal value: " << best_val << endl;

    cout << "Local serach:" << endl;
    LocalSearch search(instance, bins);

    search.solve();

    return 0;
}