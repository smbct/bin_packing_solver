#include "solver.hpp"

#include "bins.hpp"

#include <numeric>
#include <algorithm>

#include <cmath>

#include <iostream>

#include <glpk.h>

using namespace std;

//---------------------------------------------------------
Solver::Solver(Instance& instance) : _instance(instance) {

}

//---------------------------------------------------------
unsigned int Solver::best_fit() {

    vector<Bin> solution;

    vector<int> sorted_objects(_instance.n_obj());
    std::iota(sorted_objects.begin(), sorted_objects.end(), 0);

    // sort the objects according to the size
    sort(sorted_objects.begin(), sorted_objects.end(), [this](unsigned int ind_left, unsigned int ind_right) { return _instance.objects[ind_left].size > _instance.objects[ind_right].size; } );

    // for(unsigned int i = 0; i < sorted_objects.size(); i ++) {
    //     cout << sorted_objects[i] << ":" << _instance.objects[sorted_objects[i]].size << ", ";
    // }
    // cout << endl;

    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        for(unsigned int nb = 1; nb <= _instance.objects[obj_ind].nb; nb ++ ) {


            // look for the most filled bin able to contain the object
            int best_bin = -1;

            for(unsigned int bin_ind = 0; bin_ind < solution.size(); bin_ind ++) {
                if(_instance.bin_size - solution[bin_ind].size >= _instance.objects[obj_ind].size) {
                    if(best_bin == -1 || solution[bin_ind].size > solution[best_bin].size) {
                        best_bin = bin_ind;
                    }
                }
            }

            if(best_bin != -1) { // the best been is updated
                solution[best_bin].size += _instance.objects[obj_ind].size;
                solution[best_bin].objs.push_back(obj_ind); 
            } else { // a new bin is open
                solution.push_back(Bin());
                solution.back().size = _instance.objects[obj_ind].size;
                solution.back().objs.push_back(obj_ind);
            }


        }
    }

    

    return static_cast<unsigned int>(solution.size());

}


//---------------------------------------------------------
unsigned int Solver::linear_relaxation() {

    float value = 0;
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        value += static_cast<float>(_instance.objects[obj_ind].size*_instance.objects[obj_ind].nb);
    }

    value /= static_cast<float>(_instance.bin_size);

    cout << "linear relaxation (float): " << value << endl;

    return static_cast<unsigned int>(ceil(value));

}


//---------------------------------------------------------
unsigned int linear_relaxation_glpk(unsigned int upper_bound) {

    glp_prob* prob;
    prob = glp_create_prob();
    glp_set_prob_name(prob, "Bin packing lr");
    glp_set_obj_dir(prob, GLP_MIN);


    // int* ia = malloc((long unsigned int)(nbCreux+1)*sizeof(int));
    // int* ja = malloc((long unsigned int)(nbCreux+1)*sizeof(int));
    // double* ar = malloc((long unsigned int)(nbCreux+1)*sizeof(double));

    // constraints

    // an object is placed once and only once in a bin
    glp_add_rows(prob, nbCont);
    for(int i = 1; i <= pb.m; i++) {
        glp_set_row_bnds(prob, i, GLP_UP, 0.0, 0.0);
    }
    for(int i = 1; i <= pb.n; i++) {
        glp_set_row_bnds(prob, pb.m+i, GLP_FX, 1.0, 1.0);
    }

    // an object is placed once and only once in a bin


    // variables du problèmes, toutes binaires
    glp_add_cols(prob, nbVar);
    for(int i = 1; i <= nbVar; i++) {
        glp_set_col_bnds(prob, i, GLP_DB, 0.0, 1.0);
        glp_set_col_kind(prob, i, GLP_BV);
    }

    glp_delete_prob(prob);

    return 0.;

}