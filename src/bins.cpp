#include "bins.hpp"

#include <list>
#include <iostream>

using namespace std;

//---------------------------------------------------------
Bins::Bins(Instance& instance): _instance(instance) {

}

//---------------------------------------------------------
void Bins::enumerate() {
    
    // generate all the bin patterns

    list<int> pending;
    pending.push_back(0);

    unsigned int remaining_size = _instance.bin_size-_instance.objects[0].size;

    while(!pending.empty()) {
        
        // try to add additional objects, starting from 0
        bool found = false;
        unsigned int obj_ind = pending.back();
        while(!found && obj_ind < _instance.objects.size()) {
            if(_instance.objects[obj_ind].size <= remaining_size) {
                found = true;
                pending.push_back(obj_ind);
                remaining_size -= _instance.objects[obj_ind].size;
            } else {
                obj_ind ++;
            }
        }

        // if it does not fit, remove the top object and try to add the next object type 

        if(!found) {

            // first: record the new pattern, as the bin is full
            bins.push_back(Bin());
            bins.back().objs.assign(pending.begin(), pending.end());
            bins.back().objs_occ.resize(_instance.n_obj(), 0);
            for(auto& elt: bins.back().objs) {
                bins.back().objs_occ[elt] ++;
            }
            bins.back().size = _instance.bin_size-remaining_size;


            // then backtrack
            bool backtrack = true;
            while(backtrack) {
                
                if(!pending.empty()) {

                    // remove the last object
                    unsigned int next_obj = pending.back()+1;
                    remaining_size += _instance.objects[pending.back()].size;
                    pending.pop_back();

                    bool new_found = false;
                    
                    while(!new_found && next_obj < _instance.objects.size()) {
                        if(_instance.objects[next_obj].size <= remaining_size) {
                            new_found = true;
                            pending.push_back(next_obj);
                            remaining_size -= _instance.objects[next_obj].size;
                            backtrack = false;
                            // if an object is found, stop backtracking
                        } else {
                            next_obj ++;
                        }
                    }

                } else {
                    backtrack = false;
                }
            }
            
        }

    }

}

//---------------------------------------------------------
void Bins::display() {

    cout << "Enumerated bins:" << endl;
    
    for(auto bin: bins) {

        for(unsigned int ind = 0; ind < bin.objs.size(); ind ++) {
            cout << bin.objs[ind] << ", ";
        }
        cout << ": " << (float)bin.size/(float)_instance.bin_size;

        // cout << " ; obj occ: ";
        // for(auto& elt: bin.objs_occ) {
        //     cout << elt << ", ";
        // }

        cout << endl;
    }

}

//---------------------------------------------------------
unsigned int Bins::n_bins() {
    return static_cast<unsigned int>(bins.size());
}

//---------------------------------------------------------
unsigned int Bins::identify_bin(Bin& bin) {

    unsigned int best_candidate = 0;
    unsigned int best_nb_obj = 0;
    bool init = false;

    // the bin might not be full -> match with a pattern anyway
    for(unsigned int bin_ind = 0; bin_ind < bins.size(); bin_ind ++) {

        unsigned int nb_obj = static_cast<unsigned int>(bins[bin_ind].objs.size()); 
        vector<int> occ(bins[bin_ind].objs_occ);

        bool candidate = true;

        for(auto& obj_ind: bin.objs) {
            occ[obj_ind] --;
            if(occ[obj_ind] == 0) {
                nb_obj --;
                if(nb_obj == 0) {
                    break; // the exact bin has been found
                }
            } else if(occ[obj_ind] < 0) {
                candidate = false;
                break;
            }
        }

        if(candidate) {
            if(nb_obj == 0) { // the corresponding bin has been found
                best_candidate = bin_ind;
                break;
            } else if(!init || nb_obj < best_nb_obj) { // the current bin is a candidate
                init = true;
                best_candidate = bin_ind;
                best_nb_obj = nb_obj; 
            }
        } 


    }

    return best_candidate;

}