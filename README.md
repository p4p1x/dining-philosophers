# dining-philosophers
Simulation of the dining philosophers problem.
## Usage
Make sure you have installed 'ncurses.h' library. Then compile the program using a compiler of choice:
```bash
g++ -o philosophers philosophers.cpp -lncurses
```
Execute the program by passing one argument corresponding to the integer number of philosophers:
```bash
./philosophers 5
```
## Features
- Handles deadlock and starvation cases
- Uses condition variable and mutex for managing shared resources
- Utilizing 'ncurses.h' library helps with real-time visualization of excecuted program
## Future changes
- Implement usage of vectors and smart pointers usage instead of arrays and raw pointers for improved memory management.

