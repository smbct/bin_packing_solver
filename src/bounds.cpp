#include "bounds.hpp"

#include <set>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

#include <glpk.h>

#include "knapsack.hpp"

using namespace std;

//---------------------------------------------------------
Bounds::Bounds(Instance& instance) : _instance(instance) {

}

//---------------------------------------------------------
unsigned int Bounds::best_fit(vector<Bin>* solution) {

    vector<Bin> sol;

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

            for(unsigned int bin_ind = 0; bin_ind < sol.size(); bin_ind ++) {
                if(_instance.bin_size - sol[bin_ind].size >= _instance.objects[obj_ind].size) {
                    if(best_bin == -1 || sol[bin_ind].size > sol[best_bin].size) {
                        best_bin = bin_ind;
                    }
                }
            }

            if(best_bin != -1) { // the best been is updated
                sol[best_bin].size += _instance.objects[obj_ind].size;
                sol[best_bin].objs.push_back(obj_ind); 
                sol[best_bin].objs_occ[obj_ind] ++;
            } else { // a new bin is open
                sol.push_back(Bin());
                sol.back().size = _instance.objects[obj_ind].size;
                sol.back().objs.push_back(obj_ind);
                sol.back().objs_occ.resize(_instance.n_obj(), 0);
                sol.back().objs_occ[obj_ind] ++;
            }


        }
    }

    if(solution != nullptr) {
        *solution = sol;
    }

    return static_cast<unsigned int>(sol.size());

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
    /*int res = */glp_simplex(prob, &param);

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
    for(unsigned int bin_ind = 0; bin_ind < bins.bins.size(); bin_ind ++) {
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
            if(bins.bins[bin_ind].objs_occ[obj_ind] > 0) {
                ia[index] = obj_ind +1;
                ja[index] = bin_ind +1;
                ar[index] = static_cast<double>(bins.bins[bin_ind].objs_occ[obj_ind]);
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
    /*int res = */glp_simplex(prob, &param);

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


//---------------------------------------------------------
void Bounds::init_glpk_relaxation_v2(glp_prob* prob, Bins& bins) {

    // disable glpk log
    glp_term_out(0);

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
    for(unsigned int bin_ind = 0; bin_ind < bins.bins.size(); bin_ind ++) {
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

    glp_load_matrix(prob, n_sparse, ia, ja, ar);

    // free the memory
    delete[] ia;
    delete[] ja;
    delete[] ar;

}



//---------------------------------------------------------
void Bounds::update_glpk_relaxation_v2(glp_prob* prob, Bins& bins, vector<int>& to_insert, unsigned int starting_bin) {

    // some bins cannot be used
    for(unsigned int bin_ind = 0; bin_ind < starting_bin; bin_ind ++) {
        glp_set_col_bnds(prob, bin_ind+1, GLP_FX, 0.0, 0.0);
    }
    for(unsigned int bin_ind = starting_bin; bin_ind < bins.bins.size(); bin_ind ++) {
        glp_set_col_bnds(prob, bin_ind+1, GLP_LO, 0.0, 0.0);
    }

    // update the demanded objects based on the partial solution (from to_insert)

    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
        if(to_insert[obj_ind] <= 0) {
            glp_set_row_bnds(prob, obj_ind+1, GLP_FX, 0.0, 0.0);
        } else {
            glp_set_row_bnds(prob, obj_ind+1, GLP_LO, static_cast<double>(to_insert[obj_ind]), 0.0);
        }
    }
}

//---------------------------------------------------------
double Bounds::solve_glpk_relaxation_v2(glp_prob* prob) {


    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.meth = GLP_DUAL;
    parm.presolve = GLP_OFF;

    // clock_t relax_begin = clock();
    glp_simplex(prob, &parm);
    // clock_t relax_end = clock();
    // printf("temps relaxation continue (avec warm start): %f \n", (double)(relax_end-relax_begin) / CLOCKS_PER_SEC);

    double relaxation = -1;

    int state = glp_get_status(prob);
    if(state != GLP_NOFEAS) {
        relaxation = glp_get_obj_val(prob);
    }

    return relaxation;
}



// based on https://optimization.mccormick.northwestern.edu/index.php/Column_generation_algorithms
//---------------------------------------------------------
double Bounds::column_relaxation() {

    // column generation approach to compute a lower bound on the optimal value without having to enumerate all the bins
    // this implementation can be drastically improved by modifying the sub problem in glpk instead of recreating it

    vector<int> obj_size(_instance.n_obj());
    for(unsigned int obj_ind = 0; obj_ind < obj_size.size(); obj_ind ++) {
        obj_size[obj_ind] = _instance.objects[obj_ind].size;
    }

    // init the columns from a best fit solution
    
    vector<Bin> init_sol;
    /*unsigned int cost = */best_fit(&init_sol);

    // identify unique bins
    vector<vector<int>> bins;
    for(auto& bin: init_sol) {
        bool found = false;
        for(unsigned int bin_ind = 0; bin_ind < bins.size(); bin_ind ++) {
            if(bin.objs_occ == bins[bin_ind]) {
                found = true;
                break;
            }
        }
        if(!found) {
            bins.push_back(bin.objs_occ);
        }
    }

    // cout << "initial columns: " << endl;
    // for(auto& bin: bins) {
    //     for(auto& occ: bin) {
    //         cout << occ << ", ";
    //     }
    //     cout << endl;
    // }
    // cout << endl;

    bool optimal = false;

    double optimal_rmp;

    while(!optimal) {

        // solve the restricted master problem with the initial columns
        vector<double> res;
        optimal_rmp = col_rmp(bins, res);
        // cout << "rmp optimal value: " << optimal_rmp << endl;

        vector<int> new_col;

        Knapsack knapsack;
        double sub_optimal = 1.-knapsack.solve(res, obj_size, _instance.bin_size, new_col);
        if(sub_optimal < -1e-6) {
            bins.push_back(new_col);
            // cout << "kp optimal: " << sub_optimal << endl;
            // cout << "new col: ";
            // for(auto& val: new_col) {
            //     cout << val << ", ";
            // }
            // cout << endl;
            // cout << "new columns matrix: " << endl;
            // for(auto& bin: bins) {
            //     for(auto& occ: bin) {
            //         cout << occ << ", ";
            //     }
            //     cout << endl;
            // }
            // cout << endl;
            // cout << endl;
        } else {
            optimal = true;
        }

    }

    

    return optimal_rmp;
}


//---------------------------------------------------------
double Bounds::col_rmp(vector<vector<int>>& bins, vector<double>& res) {

    // create and solve the master problem
    glp_prob* prob;
    prob = glp_create_prob();
    glp_set_prob_name(prob, "Bin packing restricted master pb");
    glp_set_obj_dir(prob, GLP_MAX);


    // variables
    // n_bins variables x
    unsigned int n_var = static_cast<int>(_instance.n_obj());
    glp_add_cols(prob, n_var);
    for(unsigned int i = 0; i < n_var; i++) {
        glp_set_col_bnds(prob, i +1, GLP_LO, 0.0, 0.0);
        glp_set_col_kind(prob, i +1, GLP_CV);
    }

    // objective function coefficients: n objects    
    for(unsigned int ind = 0; ind < n_var; ind ++) {
        glp_set_obj_coef(prob, ind +1, static_cast<double>(_instance.objects[ind].nb));
    }

    // constraints: 1 constraint per bin (column)
    unsigned int n_constraints = static_cast<unsigned int>(bins.size());  
    glp_add_rows(prob, n_constraints);

    for(unsigned int ind = 0; ind < bins.size(); ind++) {
        glp_set_row_bnds(prob, ind +1, GLP_UP, 0.0, 1.0);
    }

    unsigned int n_sparse = 0;
    for(auto& bin: bins) {
        for(auto& occ: bin) {
            if(occ > 0) {
                n_sparse ++;
            }
        }
    }
    int* ia = new int[n_sparse+1]; // row
    int* ja = new int[n_sparse+1]; // col
    double* ar = new double[n_sparse+1]; // value

    unsigned int index = 1;
    for(unsigned int bin_ind = 0; bin_ind < bins.size(); bin_ind ++) {
        for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {
            if(bins[bin_ind][obj_ind] > 0) {
                ia[index] = bin_ind+1;
                ja[index] = obj_ind+1;
                ar[index] = static_cast<double>(bins[bin_ind][obj_ind]);
                index ++;
            }
        }
    }

    glp_load_matrix(prob, n_sparse, ia, ja, ar);

    glp_smcp param;
    glp_init_smcp(&param);
    param.msg_lev = GLP_MSG_OFF;
    glp_simplex(prob, &param);

    double optimal_value = -1.;

    int state = glp_get_status(prob);
    if(state != GLP_NOFEAS) {
        optimal_value = glp_get_obj_val(prob);

        res.resize(_instance.n_obj());
        for(unsigned int var_ind = 0; var_ind < n_var; var_ind ++) {
            res[var_ind] = glp_get_col_prim(prob, var_ind+1);
        }

    }

    // free the memory
    delete[] ia;
    delete[] ja;
    delete[] ar;

    glp_delete_prob(prob);

    return optimal_value;

}