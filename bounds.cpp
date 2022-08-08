#include "bounds.hpp"

#include <set>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

#include <glpk.h>

using namespace std;

//---------------------------------------------------------
Bounds::Bounds(Instance& instance) : _instance(instance) {

}

//---------------------------------------------------------
unsigned int Bounds::best_fit() {

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
double Bounds::linear_relaxation() {

    double value = 0;
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        value += static_cast<double>(_instance.objects[obj_ind].size*_instance.objects[obj_ind].nb);
    }

    value /= static_cast<double>(_instance.bin_size);

    // cout << "linear relaxation (float): " << value << endl;

    return value;

}

//---------------------------------------------------------
double Bounds::linear_relaxation_glpk(unsigned int upper_bound) {

    unsigned int n_bins = upper_bound;
    unsigned int n_objects = 0;
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        n_objects += _instance.objects[obj_ind].nb;
    }

    glp_prob* prob;
    prob = glp_create_prob();
    glp_set_prob_name(prob, "Bin packing lr");
    glp_set_obj_dir(prob, GLP_MIN);

    // variables
    // n_objects*n_bins variables x
    // upper_bound bin variables y

    unsigned int n_var = n_objects*n_bins + n_bins;
    glp_add_cols(prob, n_var);
    for(unsigned int i = 0; i < n_var; i++) {
        glp_set_col_bnds(prob, i +1, GLP_DB, 0.0, 1.0);
        glp_set_col_kind(prob, i +1, GLP_CV);
    }

    // objective function coefficients     
    for(unsigned int x_ind = 0; x_ind < n_objects*n_bins; x_ind ++) {
        glp_set_obj_coef(prob, x_ind +1, 0.);
    }
    for(unsigned int bin_ind = 0; bin_ind < n_bins; bin_ind ++) {
        glp_set_obj_coef(prob, n_objects*n_bins+bin_ind +1, 1.);
    }


    // constraints

    unsigned int n_constraints = n_objects + n_bins;  
    glp_add_rows(prob, n_constraints);

    // an object is placed once and only once in a bin
    for(unsigned int i = 0; i < n_objects; i++) {
        glp_set_row_bnds(prob, i +1, GLP_FX, 1.0, 1.0);
    }

    // the sum of the object sizes do not exceed the bin size
    for(unsigned int i = 0; i < n_bins; i++) {
        glp_set_row_bnds(prob, n_objects+i +1, GLP_UP, 0.0, 0.0);
    }

    // sparse constraint matrix

    unsigned int n_sparse = n_objects*n_bins + n_bins*(n_objects+1);

    int* ia = new int[n_sparse+1]; // row
    int* ja = new int[n_sparse+1]; // col
    double* ar = new double[n_sparse+1]; // value

    // an object is placed once and only once in a bin
    int index = 1;
    for(unsigned int obj_ind = 0; obj_ind < n_objects; obj_ind ++) {
        for(unsigned int bin_ind = 0; bin_ind < n_bins; bin_ind ++) {

            ia[index] = obj_ind +1;
            ja[index] = n_bins*obj_ind+bin_ind +1;
            ar[index] = 1.;
            index += 1;
        }
    }

    // the sum of the object sizes do not exceed the bin size
    for(unsigned int bin_ind = 0; bin_ind < n_bins; bin_ind ++) {
        int temp_ind = 0; // index relative to object quantities
        for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
            for(unsigned int obj_nb = 1; obj_nb <= _instance.objects[obj_ind].nb; obj_nb ++) {
                ia[index] = n_objects + bin_ind +1; 
                ja[index] = n_bins*temp_ind+bin_ind +1;
                ar[index] = static_cast<double>(_instance.objects[obj_ind].size);
                index ++;
                temp_ind ++;
            }
        }
        ia[index] = n_objects + bin_ind +1; 
        ja[index] = n_objects*n_bins+bin_ind +1;
        ar[index] = 0.;
        ar[index] = -static_cast<double>(_instance.bin_size);
        index ++;
    }

    // debug
    // vector<vector<double>> cst(n_constraints);
    // for(auto i = 0; i < cst.size(); i ++) {
    //     cst[i].resize(n_var, 0.);
    // }
    // for(int i = 1; i <= n_sparse; i ++) {
    //     int row = ia[i]-1;
    //     int col = ja[i]-1;
    //     double val = ar[i];
    //     cst[row][col] = val;
    // }
    // for(auto row: cst) {
    //     for(auto elt: row) {
    //         cout << elt << " ";
    //     }
    //     cout << endl;
    // }

    glp_load_matrix(prob, n_sparse, ia, ja, ar);

    // glp_write_lp(prob, NULL, "bin_packing.lp");

    // solve the problem
    glp_smcp param;
    glp_init_smcp(&param);
    param.msg_lev = GLP_MSG_OFF;
    int res = glp_simplex(prob, &param);

    double optimal_value = -1.;

    int state = glp_get_status(prob);
    if(state != GLP_NOFEAS) {
        optimal_value = glp_get_obj_val(prob);
        // cout << "linear relaxation with glpk: " << optimal_value << endl;
    }


    // free the memory

    delete[] ia;
    delete[] ja;
    delete[] ar;

    glp_delete_prob(prob);

    return optimal_value;
}


//---------------------------------------------------------
double Bounds::linear_relaxation_glpk_v2(Bins& bins) {


    glp_prob* prob;
    prob = glp_create_prob();
    glp_set_prob_name(prob, "Bin packing lr v2");
    glp_set_obj_dir(prob, GLP_MIN);

    // variables
    // n_bins variables x

    unsigned int n_var = static_cast<int>(bins.bins.size());
    glp_add_cols(prob, n_var);
    for(unsigned int i = 0; i < n_var; i++) {
        glp_set_col_bnds(prob, i +1, GLP_LO, 0.0, 0.0);
        glp_set_col_kind(prob, i +1, GLP_CV);
    }

    // objective function coefficients     
    for(unsigned int ind = 0; ind < n_var; ind ++) {
        glp_set_obj_coef(prob, ind +1, 1.);
    }


    // constraints: 1 constraint per object type
    unsigned int n_constraints = _instance.n_obj();  
    glp_add_rows(prob, n_constraints);

    // the sum of the object sizes do not exceed the bin size
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind++) {
        glp_set_row_bnds(prob, obj_ind +1, GLP_LO, static_cast<double>(_instance.objects[obj_ind].nb), 0.0);
    }

    // sparse constraint matrix
    unsigned int n_sparse = 0;
    for(unsigned int bin_ind = 0; bin_ind <= bins.bins.size(); bin_ind ++) {
        set<int> unique_objs;
        unique_objs.insert(bins.bins[bin_ind].objs.begin(), bins.bins[bin_ind].objs.end());
        n_sparse += static_cast<unsigned int>(unique_objs.size()); // one coefficient per object per bin
    }

    int* ia = new int[n_sparse+1]; // row
    int* ja = new int[n_sparse+1]; // col
    double* ar = new double[n_sparse+1]; // value

    int index = 1;

    // an object is placed once and only once in a bin
    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        for(unsigned int bin_ind = 0; bin_ind < bins.bins.size(); bin_ind ++) {
            unsigned int n_occ = 0;
            for(auto& obj: bins.bins[bin_ind].objs) {
                if(static_cast<unsigned int>(obj) == obj_ind) {
                    n_occ ++;
                }
            }
            if(n_occ > 0) {
                ia[index] = obj_ind +1;
                ja[index] = bin_ind +1;
                ar[index] = static_cast<double>(n_occ);
                index += 1;
            }
        }

    }

    // debug
    // vector<vector<double>> cst(n_constraints);
    // for(auto i = 0; i < cst.size(); i ++) {
    //     cst[i].resize(n_var, 0.);
    // }
    // for(int i = 1; i <= n_sparse; i ++) {
    //     int row = ia[i]-1;
    //     int col = ja[i]-1;
    //     double val = ar[i];
    //     cst[row][col] = val;
    // }
    // for(auto row: cst) {
    //     for(auto elt: row) {
    //         cout << elt << " ";
    //     }
    //     cout << endl;
    // }

    glp_load_matrix(prob, n_sparse, ia, ja, ar);

    // glp_write_lp(prob, NULL, "bin_packing.lp");

    // solve the problem
    glp_smcp param;
    glp_init_smcp(&param);
    param.msg_lev = GLP_MSG_OFF;
    int res = glp_simplex(prob, &param);

    double optimal_value = -1.;

    int state = glp_get_status(prob);
    if(state != GLP_NOFEAS) {
        optimal_value = glp_get_obj_val(prob);
        // cout << "linear relaxation with glpk v2: " << optimal_value << endl;

        // cout << "bin usage: ";
        // for(unsigned int var_ind = 0; var_ind < n_var; var_ind ++) {
        //     cout << glp_get_col_prim(prob, var_ind+1) << ", ";
        // }
        // cout << endl;

    }


    // free the memory

    delete[] ia;
    delete[] ja;
    delete[] ar;

    glp_delete_prob(prob);


    return optimal_value;

}