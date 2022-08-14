# bin_packing_solver

A solving tool for the bin packing problem.

Several solving functions are implemented:

- enumeration of all the bins to improve the model strength (works only for small instances)
- two linear relaxations obtained by glpk (one with the bins, the other one with the classical lp formulation)
- a linear relaxation using a column generation approach (without bin patterns enumeration)
- a best fit heuristic implemented to obtain a (not so good) feasible solution
- a homemade heuristic search to improve the best fit solution
- an exact solver using glpk ILP solver
- an exact solver using tree search based on enumerated bins, implementing bounding/cutting procedures