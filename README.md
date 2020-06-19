# ROBDD_ITE
ROBDD package for leveilzed circuits. Builds ROBDD using the ITE() operator.

All .cpp files should be compiled with -std=c++11 flag.

# Step 0 - Delete all .bdd and .sim files in src and src/simulator folders

$ cd src

$ rm *.bdd *.sim

$ cd src/simulator

$ rm *.bdd *.sim

# Step 1 - Prepare Simulation Results from src/simulator for comparison

$ cd src/simulator

$ g++ simulator.cpp -o simulator.out

$ ./simulator.out c17 c17 > c17_c17.sim

$ ./simulator.out c17 c17_random > c17_c17_random.sim

$ ./simulator.out c432 c432 > c432_c432.sim

$ ./simulator.out c432 c432_random > c432_c432_random.sim

$ ./simulator.out c432 c432 > c432_c432.sim

The src/simulator has c17.lev and c432.lev files.
It also has the following input vector files: c17.vec, c17_random.vec, c432_random.vec and c432.vec.

# Step 2 - In src directory
$ cd ..

$ g++ robdd_ite.cpp

$ ./a.out c17 c17

The output will be as such:

> Building for this circuit....

> Build completed!!

> Writing to c17.bdd

> Writing simulation to c17.sim

$ cat c17.bdd

> 5 2 13

> 1 2 3 4 5 

> -11 -12 

> 1 6 0 0

> 2 6 0 0

> 3 3 2 1

> 4 4 2 1

> 5 5 2 1

> 6 3 4 1

> 7 2 -6 1

> 8 4 1 5

> 9 3 8 5

> 10 2 1 -3

> 11 1 10 -7

> 12 2 6 -9

$ diff c17.sim simulator/c17_c17.sim 

The diff won't output anything as both files will match.

====

Similarly, for c17_random.vec.

$ ./a.out c17 c17_random

$ diff c17_random.sim simulator/c17_c17_random.sim 

====

Similarly, for c432.lev and c432.vec.

$ ./a.out c432 c432

$ diff c432.sim simulator/c432_c432.sim

====

Similarly, for c432_random.vec.

$ ./a.out c432 c432_random

$ diff c432_random.sim simulator/c432_c432_random.sim 

====

Diffs don't show any difference --> BDD's package simulation outputmatches with the naive simulator.

For other .lev circuits, pass any simulation file to only build the BDD.

./a.out c1355 c432

The .bdd file will be saved correctly, but the package may get stuck during simulation.
This is expected because no relevant input vector file was passed. (I worked on this 2 years ago and I am too lazy to add proper modes right now)
