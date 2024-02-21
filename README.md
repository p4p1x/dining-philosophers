# dining-philosophers
Simulation of the dining philosophers problem.
# How to use it?
Make sure you have installed 'ncurses.h' library. Then compile the program using a compilator of choice:
```bash
g++ -o philosophers philosophers.cpp -lncurses
```
Execute the program by passing one argument corresponding to the integer number of philosophers:
```bash
./philosophers 5
```
# Features
- Handles deadlock and starvation cases
- Uses condition variable and mutex for managing shared resources
- Utilizing 'ncurses.h' library helps with real-time visualization of excecuted program
