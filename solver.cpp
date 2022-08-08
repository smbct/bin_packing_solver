#include "solver.hpp"
#include "bins.hpp"
#include "bounds.hpp"

#include <algorithm>

#include <cmath>

#include <iostream>

#include <glpk.h>

#include <set>

#include <stack>
#include <list>

using namespace std;

//---------------------------------------------------------
Solver::Solver(Instance& instance) : _instance(instance) {

}

//---------------------------------------------------------
void Solver::update_sol_add(Bin& bin, vector<int>& to_insert, int& n_obj_rem, double& length_rem) {
    
    // update the values with the bin
    for(auto& obj_ind: bin.objs) {
        if(to_insert[obj_ind] == 1) {
            n_obj_rem --;
        }
        length_rem -= _instance.objects[obj_ind].size;
        to_insert[obj_ind] --;
    }
}

//---------------------------------------------------------
void Solver::update_sol_remove(Bin& bin, vector<int>& to_insert, int& n_obj_rem, double& length_rem) {
    
    // update the values with the bin
    for(auto& obj_ind: bin.objs) {
        if(to_insert[obj_ind] == 0) {
            n_obj_rem ++;
        }
        length_rem += _instance.objects[obj_ind].size;
        to_insert[obj_ind] ++;
    }
}

//---------------------------------------------------------
unsigned int Solver::identify_next_bin(vector<int>& to_insert, Bins& bins, unsigned int starting_bin) {

    bool found = false;
    unsigned int candidate_bin = starting_bin;

    while(!found && candidate_bin < bins.bins.size()) {
        for(auto& obj_ind: bins.bins[candidate_bin].objs) {
            if(to_insert[obj_ind] > 0) {
                found = true;
                break;
            }
        }
        if(!found) {
            candidate_bin ++;
        }
    }

    return candidate_bin;

}

//---------------------------------------------------------
bool Solver::test_remaining_objects(vector<int>& to_insert, Bins& bins, unsigned int remaining_bins, unsigned int bin_index) {

    bool res = true;

    for(unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind ++) {

        if(to_insert[obj_ind] > 0) {

            unsigned int max_occ = 0; // maximum number of occurrences for this object in the considered bins
            for(unsigned int bin_ind = bin_index; bin_ind < bins.bins.size(); bin_ind ++) {
                if(bin_ind == bin_index || static_cast<unsigned int>(bins.bins[bin_ind].objs_occ[obj_ind]) > max_occ) {
                    max_occ = bins.bins[bin_ind].objs_occ[obj_ind];
                }
            }
            if(max_occ > 0) {
                unsigned int n_bins_needed = static_cast<unsigned int>(ceil(static_cast<double>(to_insert[obj_ind])/static_cast<double>(max_occ)));
                if(n_bins_needed > remaining_bins) {
                    res = false;
                    break;
                }
            } else {
                res = false;
                break;
            }
            
        }

    }

    return res;

}

//---------------------------------------------------------
unsigned int Solver::solve_bins(Bins& bins) {

    // tree search on the bin patterns

    vector<int> to_insert(_instance.n_obj(), 0);
    for(unsigned int obj_ind = 0; obj_ind < to_insert.size(); obj_ind ++) {
        to_insert[obj_ind] += _instance.objects[obj_ind].nb;
    }

    int n_obj_rem = _instance.n_obj();
    double length_rem = 0.; // linear relaxation
    for(unsigned int obj_ind = 0; obj_ind < to_insert.size(); obj_ind ++) {
        length_rem += _instance.objects[obj_ind].nb*_instance.objects[obj_ind].size;
    }

    list<unsigned int> sol;
    sol.push_back(0); // add the first bin

    // update the values with the first bin
    update_sol_add(bins.bins[0], to_insert, n_obj_rem, length_rem);

    Bounds bounds(_instance);

    

    glp_prob* prob;
    prob = glp_create_prob();
    bounds.init_glpk_relaxation_v2(prob, bins);
    unsigned int initial_lower_bound = static_cast<unsigned int>(ceil(bounds.solve_glpk_relaxation_v2(prob)));

    unsigned int initial_upper_bound = bounds.best_fit();
    // initial_upper_bound = 26; // oracle
    unsigned int best_val = initial_upper_bound;
    unsigned int max_depth = best_val-1;

    bool stop = false;

    unsigned int n_nodes = 0;
    unsigned int n_sol_visited = 0;

    while(!stop) {

        n_nodes ++;

        // if(n_nodes % 1000 == 0) {
        //     cout << "n nodes" << n_nodes << endl;
        //     cout << "current best: " << best_val << endl;
        // }

        bool backtrack = false;

        // cout << "n objects remaining: " << n_obj_rem << endl;
        // cout << "current sol: ";
        // for(auto& elt: sol) {
        //     cout << elt << ", ";
        // }
        // cout << endl;
        // cout << "best sol so far: " << best_val << endl;
        // cout << "initial bounds: " << initial_lower_bound << " <= x <= " << initial_upper_bound << endl;
        // cout << "to insert: ";
        // for(auto& elt: to_insert) {
        //     cout << elt << ", ";
        // }
        // cout << endl;
        // cout << "n sol visited: " << n_sol_visited << endl;
        // cout << endl << endl;

        if(n_obj_rem > 0) { // exploration part, a solution has not been reached

            if(sol.size() < max_depth) {

                // bounding
                // compute a bound per object
                // if a bin is intensively used, that might prevent from reaching the quantity for some objects 

                // linear relaxation v1
                // double lower_bound = static_cast<unsigned int>(sol.size())+ceil(length_rem/_instance.bin_size);

                bounds.update_glpk_relaxation_v2(prob, bins, to_insert, sol.back());
                double lower_bound = static_cast<unsigned int>(sol.size())+ceil(bounds.solve_glpk_relaxation_v2(prob));

                // cout << "current lower bound: " << lower_bound << endl;

                if(lower_bound >= best_val || !test_remaining_objects(to_insert, bins, max_depth-static_cast<unsigned int>(sol.size()), sol.back())) {
                    backtrack = true;
                    
                } else { // exploration, add a new bin

                    // as bellow, add only the bins that may be useful for filling the need
                    // unsigned int candidate_bin = identify_next_bin(to_insert, bins, sol.back());
                    unsigned int candidate_bin = sol.back();

                    if(candidate_bin < bins.bins.size()) {
                        sol.push_back(candidate_bin);
                        update_sol_add(bins.bins[sol.back()], to_insert, n_obj_rem, length_rem);
                    } else {
                        backtrack = true;
                    }
                    

                }

            } else {
                backtrack = true;
            }

        } else {

            n_sol_visited ++;

            // a solution is found
            if(sol.size() < best_val) {
                best_val = static_cast<unsigned int>(sol.size());
                max_depth = best_val-1; // update the maximum depth

                cout << "new best: " << best_val << endl;

                if(best_val == initial_lower_bound) {
                    stop = true;
                    // if the initial lower bound is equal to the new solution, it is proven to be the best solution
                }

            }

            backtrack = true;
        }


        if(backtrack) {
            while(backtrack) {

                if(sol.size() > 0) { // remove the last added

                    unsigned int last_added = sol.back();
                    sol.pop_back();
                    update_sol_remove(bins.bins[last_added], to_insert, n_obj_rem, length_rem);

                    if(last_added < bins.bins.size()-1) {

                        // look for the next bin that can contribute to the solution (only adding necessary objects)
                        // unsigned int candidate_bin = identify_next_bin(to_insert, bins, last_added+1);
                        unsigned int candidate_bin = last_added+1;

                        if(candidate_bin < bins.bins.size()) {
                            // sol.push_back(last_added+1);
                            sol.push_back(candidate_bin);
                            update_sol_add(bins.bins[candidate_bin], to_insert, n_obj_rem, length_rem);
                            backtrack = false;
                        }
                        
                    
                    }

                } else {

                    backtrack = false;
                    stop = true; // the search is terminated
                
                }
                
            }

        }

    }

    glp_delete_prob(prob);

    cout << "n nodes explored: " << n_nodes << endl;

    return best_val;

}


//---------------------------------------------------------
unsigned int Solver::solve_glpk_bins(Bins& bins) {

    glp_prob* prob;
    prob = glp_create_prob();
    glp_set_prob_name(prob, "Bin packing glpk solution");
    glp_set_obj_dir(prob, GLP_MIN);

    // variables
    // n_bins variables x

    unsigned int n_var = static_cast<int>(bins.bins.size());
    glp_add_cols(prob, n_var);
    for(unsigned int i = 0; i < n_var; i++) {
        glp_set_col_bnds(prob, i +1, GLP_LO, 0.0, 0.0);
        glp_set_col_kind(prob, i +1, GLP_IV);
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

    glp_simplex(prob, NULL);

    int res = glp_intopt(prob, NULL);

    double optimal_value = -1.;

    int state = glp_get_status(prob);
    if(state != GLP_NOFEAS) {
        optimal_value = glp_mip_obj_val(prob);
        
        cout << "optimal value with glpk v2: " << optimal_value << endl;

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