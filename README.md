# MovHex: Optimal Route Calculator

## Project Overview
Movhex is a transportation company that operates a fleet of vehicles across a wide geographical area. To minimize operational expenses, this project implements a specialized program to calculate optimal travel routes for their vehicles. 

The program models the world map as a rectangular grid of hexagonal tiles. Each hexagon connects to up to six neighboring tiles. Vehicles can travel by land between adjacent hexagons or via dynamically established one-way air routes. Every tile has an associated exit cost, representing the difficulty or expense of traveling through it. 

## Implementation Details
The project is implemented in **C** and relies on efficient algorithms and data structures to ensure high performance:

*   **Coordinate System:** While the map is instantiated as a standard 2D grid using offset coordinates (column and row), internally, the program leverages **Cubic Coordinates** (`q`, `r`, `s`). This approach significantly simplifies the mathematical calculation of distances between any two hexagonal tiles (similar to Manhattan distance).
*   **Pathfinding Algorithm:** To evaluate the most cost-effective path between two points, the program utilizes **Dijkstra's Algorithm**. 
*   **Priority Queue (Min-Heap):** Dijkstra's algorithm is backed by a custom-built Min-Heap Priority Queue. This ensures that the node with the lowest accumulated travel cost is always processed next, drastically optimizing the search time.
*   **Dynamic Memory Management:** The map matrix and the air routes are dynamically allocated (`malloc`) and properly freed to prevent memory leaks, scaling efficiently based on the initialized grid dimensions.

## Supported Commands
The program operates by reading a sequence of commands from standard input (`stdin`). 

*   `init <columns> <rows>`: Initializes (or resets) the map with the specified dimensions. All tiles start with an exit cost of 1.
*   `change_cost <x> <y> <v> <radius>`: Modifies the exit cost of hexagons within a certain `radius` of the tile at `(x, y)`. The adjustment is scaled based on the distance from the center tile.
*   `toggle_air_route <x1> <y1> <x2> <y2>`: Toggles a unidirectional air route from the first tile to the second. The cost of a new route is calculated as the floored average of existing outgoing air routes plus the origin's land exit cost.
*   `travel_cost <xp> <yp> <xd> <yd>`: Calculates and prints the minimum cost required to travel from the starting tile to the destination tile.

## How to Compile and Execute
The program is written in standard C. Because it uses the `<math.h>` library for functions like `floor`, `fmin`, and `fmax`, you must link the math library during compilation.

### Compilation
Open your terminal and compile the source code using `gcc`:

```bash
gcc -o movhex MovHex.c -lm
```

### Execution
You can run the program interactively or feed it a text file containing a list of commands.

**Interactive Mode:**
```bash
./movhex
```
*(Type commands and press Enter to see the immediate output. Press `Ctrl+D` to send EOF and exit).*

**File Input Mode:**
If you have a file named `input.txt` with your commands, you can redirect it to the program:
```bash
./movhex < input.txt
```
