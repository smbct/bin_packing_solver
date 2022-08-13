#include "heuristic.hpp"

#include "bounds.hpp"

#include <cmath>
#include <iostream>

using namespace std;

//---------------------------------------------------------
HSolution::HSolution(Instance &instance, Bins &bins) : _instance(instance), _bins(bins), to_insert(_instance.n_obj()), sol(_bins.bins.size())
{
}

//---------------------------------------------------------
void HSolution::init(vector<int> &new_sol)
{

    sol = new_sol;

    for (unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind++)
    {
        to_insert[obj_ind] = _instance.objects[obj_ind].nb;
    }

    for (unsigned int bin_ind = 0; bin_ind < _bins.bins.size(); bin_ind++)
    {
        if (sol[bin_ind] > 0)
        {
            for (auto &obj_ind : _bins.bins[bin_ind].objs)
            {
                to_insert[obj_ind] -= sol[bin_ind];
            }
        }
    }

    n_remaining = 0;
    size_remaining = 0.;

    // min, max to_insert, nb_remaining, remaining length, feasibility
    for (unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind++)
    {

        if (obj_ind == 0 || to_insert[obj_ind] < min_to_insert)
        {
            min_to_insert = to_insert[obj_ind];
        }

        if (obj_ind == 0 || to_insert[obj_ind] > max_to_insert)
        {
            max_to_insert = to_insert[obj_ind];
        }

        if (to_insert[obj_ind] > 0)
        {
            n_remaining++;
            size_remaining += to_insert[obj_ind] * _instance.objects[obj_ind].size;
        }
    }

    n_bins = 0;
    for (unsigned int bin_ind = 0; bin_ind < sol.size(); bin_ind++)
    {
        n_bins += sol[bin_ind];
    }

    feasible = (n_remaining == 0);

    cost = compute_cost();
}

//---------------------------------------------------------
double HSolution::compute_cost()
{
    return static_cast<double>(n_bins) + 2.5 * static_cast<double>(n_remaining);
}

//---------------------------------------------------------
void HSolution::update(unsigned int bin_ind, int delta)
{

    if (delta == 0) {
        return;
    }

    sol[bin_ind] += delta;

    n_bins += delta;

    for (auto &obj_ind : _bins.bins[bin_ind].objs) {

        to_insert[obj_ind] -= delta;
        size_remaining -= delta * _instance.objects[obj_ind].size;

        if (delta > 0) {

            // objects have been removed in to_insert

            if (to_insert[obj_ind] + delta > 0 && to_insert[obj_ind] <= 0) {
                n_remaining--;
                if (n_remaining <= 0) {
                    feasible = true;
                }
            }
        } else if (delta < 0) {
            // objects have been added in to_insert

            if (to_insert[obj_ind] + delta <= 0 && to_insert[obj_ind] > 0) {

                n_remaining++;

                if (n_remaining > 0) {
                    feasible = false;
                }
            }
        }
    }

    for (unsigned int obj_ind = 0; obj_ind < to_insert.size(); obj_ind++) {
        
        if (obj_ind == 0 || to_insert[obj_ind] <= min_to_insert) {
            min_to_insert = to_insert[obj_ind];
        } if (obj_ind == 0 || to_insert[obj_ind] >= max_to_insert) {
            max_to_insert = to_insert[obj_ind];
        }
    }

    // update the cost
    cost = compute_cost();
}

//---------------------------------------------------------
void HSolution::display()
{

    cout << "sol: ";
    for (auto &val : sol) {
        cout << val << ", ";
    }
    cout << endl;

    cout << "to_insert: ";
    for (auto &val : to_insert) {
        cout << val << ", ";
    }
    cout << endl;
    cout << "n bins: " << n_bins << endl;
    cout << "n remaining: " << n_remaining << endl;
    cout << "feasible ? " << (feasible ? "true" : "false") << endl;
    cout << "cost: " << cost << endl;
}

//---------------------------------------------------------
HSolution &HSolution::operator=(const HSolution &other)
{

    to_insert = other.to_insert;
    max_to_insert = other.max_to_insert;
    min_to_insert = other.min_to_insert;

    n_remaining = other.n_remaining;
    size_remaining = other.size_remaining;

    sol = other.sol;
    n_bins = other.n_bins;
    cost = other.cost;
    feasible = other.feasible;

    return *this;
}

//---------------------------------------------------------
HeuristicSearch::HeuristicSearch(Instance &instance, Bins &bins) : _instance(instance), _bins(bins)
{
}

//---------------------------------------------------------
void HeuristicSearch::add_bins(HSolution &sol)
{

    // add the bin maximizing the number of obj. for one of the minimal object

    for (unsigned int it = 1; it <= 50; it++) {

        vector<unsigned int> ind_max;
        for (unsigned int obj_ind = 0; obj_ind < sol.to_insert.size(); obj_ind++) {

            if (sol.to_insert[obj_ind] == sol.max_to_insert) {
                ind_max.push_back(obj_ind);
            }
        }

        unsigned int obj_ind = ind_max[rand() % ind_max.size()];
        vector<unsigned int> bin_obj;
        for (unsigned int bin_ind = 0; bin_ind < _bins.bins.size(); bin_ind++) {
            if (_bins.bins[bin_ind].objs_occ[obj_ind] > 0) {
                bin_obj.push_back(bin_ind);
            }
        }

        unsigned int bin_ind = bin_obj[rand() % bin_obj.size()];

        sol.update(bin_ind, 1);
    }
}

//---------------------------------------------------------
void HeuristicSearch::remove_bins(HSolution &solution, HSolution &best_sol) {

    bool stop = false;

    while (!stop) {

        int most_used_bin = 0;
        int least_used_bin = 0;

        for (unsigned int bin_ind = 0; bin_ind < _bins.n_bins(); bin_ind++) {

            if (bin_ind == 0 || solution.sol[bin_ind] > most_used_bin) {
                most_used_bin = solution.sol[bin_ind];
            }
            if (bin_ind == 0 || solution.sol[bin_ind] < least_used_bin) {
                solution.sol[bin_ind] = least_used_bin;
            }
        }

        vector<double> bin_scores(_bins.bins.size(), 0.);

        double max_score = 0.;
        double min_score = 0.;

        for (unsigned int bin_ind = 0; bin_ind < bin_scores.size(); bin_ind++) {
            auto &bin = _bins.bins[bin_ind];

            if (solution.sol[bin_ind] == 0) {
                bin_scores[bin_ind] = -1.;
                continue;
            }

            bool feasible = true;
            for (unsigned int obj_ind = 0; obj_ind < _instance.n_obj(); obj_ind++) {
                if (bin.objs_occ[obj_ind] > 0 && solution.to_insert[obj_ind] + bin.objs_occ[obj_ind] > 0) {
                    bin_scores[bin_ind] = -1.;
                    feasible = false;
                    break;
                }
            }
            if (!feasible) {
                continue;
            }

            double object_score = 0.;
            for (auto &obj_ind : bin.objs) {
                object_score += 1. - static_cast<double>(solution.to_insert[obj_ind] - solution.min_to_insert) / static_cast<double>(solution.max_to_insert - solution.min_to_insert);
            }
            object_score /= static_cast<double>(bin.objs.size());

            double gap_score = static_cast<double>(_instance.bin_size - bin.size) / static_cast<double>(_instance.bin_size);

            double bin_score = static_cast<double>(solution.sol[bin_ind] - least_used_bin) / static_cast<double>(most_used_bin - least_used_bin);

            bin_scores[bin_ind] = object_score * 2. + gap_score * 1. + bin_score * 2.;

            if (bin_ind == 0 || bin_scores[bin_ind] < min_score) {
                min_score = bin_scores[bin_ind];
            }
            if (bin_ind == 0 || bin_scores[bin_ind] > max_score) {
                max_score = bin_scores[bin_ind];
            }
        }

        // select a bin to remove
        vector<unsigned int> bin_indexes;
        for (unsigned int bin_ind = 0; bin_ind < _bins.n_bins(); bin_ind++) {
            
            if (bin_scores[bin_ind] < 0.) {
                continue;
            }
            
            double score = bin_scores[bin_ind] - min_score;
            score /= (max_score - min_score);
            bin_indexes.insert(bin_indexes.end(), static_cast<unsigned int>(score * 20), bin_ind);
        }

        if (!bin_indexes.empty()) {
            unsigned int bin_ind = bin_indexes[rand() % bin_indexes.size()];

            solution.update(bin_ind, -1);

            // solution.display();
            // cout << endl << endl;
        } else {

            if (solution.n_bins < best_sol.n_bins) {
                // cout << "new best: " << solution.n_bins << endl;
                best_sol = solution;
            }

            stop = true;
        }
    }
}

//---------------------------------------------------------
unsigned int HeuristicSearch::solve()
{

    Bounds bounds(_instance);

    vector<int> init_sol(_bins.bins.size(), 0);

    vector<Bin> best_fit_sol;
    unsigned int best_val = bounds.best_fit(&best_fit_sol);
    // cout << "best fit sol size: " << best_fit_sol.size() << endl;
    for (auto &bin : best_fit_sol) {
        unsigned int bin_ind = _bins.identify_bin(bin);
        init_sol[bin_ind]++;
    }

    HSolution solution(_instance, _bins);
    solution.init(init_sol);
    // solution.display();

    HSolution best_sol(_instance, _bins);
    best_sol = solution;

    add_bins(solution);
    // solution.display();

    // strategy: remove as many bins as possible until removing any of the bins make the solution infeasible
    // then re-add multiple bins to start from a new point

    // score the bins: objects that are the least needed, smallest bin gap, most used bins
    // then perform a weighted random selection of the bin to insert

    unsigned int nb_it = 0;

    while (nb_it < 1000) {

        remove_bins(solution, best_sol);

        solution.init(best_sol.sol);
        // solution.init(init_sol);

        add_bins(solution);

        nb_it ++;
    }

    // cout << "best solution: " << endl;
    // best_sol.display();
    // cout << endl << endl;

    return best_sol.n_bins;

}