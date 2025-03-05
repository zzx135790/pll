//
// This is the starting code for the OpenMP coursework of XJCO3221 Parallel Computation.
//
// It requires the file 'cwk1_extra.h' to be in the local directory, which includes
// routines for displaying the state of the system etc. You are free to look at this file,
// but DO NOT MAKE ANY ALTERATIONS, as it will be replaced with a different file for assessment
// so your changes would be lost.
//
// A simple makefile is provided: please do not alter it.
//
// If you are using a compiler other than 'gcc', then you can specify this when calling make.
// For instance, to use 'gcc-14' rather than 'gcc':
//
// > make CC=gcc-14
//
// There is also an optional graphical output that uses OpenGL and GLFW, much the same as the
// examples from Lectures 3 and 13. To select this, build with
//
// > make GRAPHICS=GLFW
//
// This works on cloud-hpc1.leeds.ac.uk, but you may need to modify the location of OpenGL/GLFW
// libraries in the makefile to work on your system. Note you do not need to use this graphical
// output for this assignment; the terminal output will suffice.
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>        // So we can use 'true' and 'false' in C.
#include <time.h>           // Used to seed the pseudo-random number generator.
#include <unistd.h>         // Needed for usleep(), to add small pauses between displays.

// Extra routines for display etc. DO NOT ALTER THIS FILE as it will be replaced with another
// version for assessment.
#include "cwk1_extra.h"


//
// The current grid and a copy, stored as two-dimensional arrays of integers.
// Memory is allocated for both of these in 'allocateGridsMemory' in the extra file.
//
int **grid;
int **gridCopy;


//
// Count the number of cells in the current 'grid'.
//
// Note the loops here could range from i=1 to i<N-1, rather than i=0 to i<N,
// because the way this problem is set-up there will never be cells on the boundary.
// However, please retain the loop ranges as in the serial code below for your
// solution.
//
int numCells(int N)
{
    int total = 0;
    #pragma omp parallel for reduction(+:total)
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if(grid[i][j]) {
                total++;
            }
        }
    }
    return total;
}

//
// Add M cells to the grid at random locations, ensuring exactly M distinct cells are added in total.
//
// void initialiseGrid( int N, int M )
// {
//     int cell;
//     for( cell=0; cell<M; cell++ )
//     {
//         while( true )       // 'true' is defined in stdbool.
//         {
//             // Get a random location on the grid, not including the outer rows/columns.
//             // You may assume rand() is thread-safe for this assignment.
//             int
//                 i = 1 + (N-2) * 1.0*rand()/RAND_MAX,
//                 j = 1 + (N-2) * 1.0*rand()/RAND_MAX;

//             // Check the indices are in the allowed range, as rand() can occasionally return spurious values.
//             if( i<0 || j<0 || i>=N || j>=N ) continue;

//             // If currently occupied, return to the start and try again with new values of i and j.
//             if( grid[i][j] ) continue;

//             // If not occupied, add the cell and break from the while( true ) loop.
//             grid[i][j] = 1;
//             break;
//         }
//     }
// }

void initialiseGrid(int N, int M)
{
    #pragma omp parallel for schedule(dynamic)
    for(int cell = 0; cell < M; cell++)
    {
        int ifend = 0;
        while(!ifend)
        {
            int i = 1 + (N-2) * 1.0*rand()/RAND_MAX;
            int j = 1 + (N-2) * 1.0*rand()/RAND_MAX;

            if(i < 0 || j < 0 || i >= N || j >= N) continue;


            #pragma omp critical
            {
                if(grid[i][j] == 0) {
                    grid[i][j] = 1;
                    ifend = 1;
                }
            }
        }
    }
}


// // Iterate using the original rules of Conway's game of life.
// void iterateWithOriginalRules( int N )
// {
//     int i, j;

//     // For the purposes of this assignment, make a copy of the grid and read this copy when updating the main array grid[]
//     // (rather using pointers to alternate between the two arrays, which is not allowed for this assignment).
//     for( i=0; i<N; i++ )
//         for( j=0; j<N; j++ )
//             gridCopy[i][j] = grid[i][j];

//     // Apply the rules of Conway's Game of Life. Note that grid squares on the boundary are not updated and will always remain empty.
//     for( i=1; i<N-1; i++ )
//         for( j=1; j<N-1; j++ )
//         {
//             // Count the number of neighbours in the Moore neighbourhood in the grid copy.
//             int numNeighbours = gridCopy[i-1][j+1] + gridCopy[i][j+1] + gridCopy[i+1][j+1]
//                               + gridCopy[i-1][j  ]                    + gridCopy[i+1][j  ]
//                               + gridCopy[i-1][j-1] + gridCopy[i][j-1] + gridCopy[i+1][j-1];

//             // Apply the update rules.
//             if( grid[i][j]==1 )
//             {
//                 if( numNeighbours<2 || numNeighbours>3 ) grid[i][j] = 0;        // ... else grid[i][j] remains 1.
//             }
//             else            // i.e. if grid[i][j]==0.
//             {
//                 if( numNeighbours==3 ) grid[i][j] = 1; 
//             }
//         }
// }

void iterateWithOriginalRules(int N)
{
    // 并行化网格复制（第一层循环）
    #pragma omp parallel for
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            gridCopy[i][j] = grid[i][j];
        }
    }

    // 并行化规则应用（外层循环）
    #pragma omp parallel for
    for(int i = 1; i < N-1; i++) {
        for(int j = 1; j < N-1; j++) {
            // 保持原始邻域计算方式
            int numNeighbours = gridCopy[i-1][j+1] + gridCopy[i][j+1] + gridCopy[i+1][j+1]
                              + gridCopy[i-1][j]                     + gridCopy[i+1][j]
                              + gridCopy[i-1][j-1] + gridCopy[i][j-1] + gridCopy[i+1][j-1];

            // 保持原始更新规则
            if(grid[i][j] == 1) {
                if(numNeighbours < 2 || numNeighbours > 3) 
                    grid[i][j] = 0;  // 直接修改原网格
            } else {
                if(numNeighbours == 3) 
                    grid[i][j] = 1;
            }
        }
    }
}

// Iterate using the modified rules as explained in the coursework instructions.
void iterateWithModifiedRules(int N)
{
    // 红黑分阶段更新（先处理红色格子）
    #pragma omp parallel for
    for(int i = 1; i < N-1; i++) {
        for(int j = 1 + (i%2); j < N-1; j += 2) { // 红色阶段：奇数行偶数列，偶数行奇数列
            // 冯·诺依曼邻域计算
            int numNeighbours = grid[i-1][j] + grid[i+1][j] 
                              + grid[i][j-1] + grid[i][j+1];
            
            // 应用修改后的规则
            if(grid[i][j] == 1) {
                if(numNeighbours == 0 || numNeighbours == 3)
                    grid[i][j] = 0; // 直接更新原网格
            } else {
                if(numNeighbours == 2)
                    grid[i][j] = 1;
            }
        }
    }

    // 处理黑色格子阶段
    #pragma omp parallel for
    for(int i = 1; i < N-1; i++) {
        for(int j = 1 + ((i+1)%2); j < N-1; j += 2) { // 黑色阶段：奇数行奇数列，偶数行偶数列
            int numNeighbours = grid[i-1][j] + grid[i+1][j] 
                              + grid[i][j-1] + grid[i][j+1];
            
            if(grid[i][j] == 1) {
                if(numNeighbours == 0 || numNeighbours == 3)
                    grid[i][j] = 0;
            } else {
                if(numNeighbours == 2)
                    grid[i][j] = 1;
            }
        }
    }
}


//
// Main. Parses command line arguments and initiates the simulation.
//
int main( int argc, char **argv )
{
    // Seed the psuedo-random number generator to the current time.
    srand( time(NULL) );

    // Declarations for the variables specified on the command line.
    int N, M, numIterations, rulesVersion;

    // Parse the command line arguments. This function is defined in the extra file, and returns -1 if there was a problem.
    if( parseCommandLineArguments( argc, argv, &N, &M, &numIterations, &rulesVersion )==-1 ) return EXIT_FAILURE;
    
    // Allocate memory for two grids, 'grid' and 'gridCopy,' which are declared above. Defined in the extra file. Returns -1 if there was a problem.
    if( allocateGridsMemory( &grid, &gridCopy, N )==-1 ) return EXIT_FAILURE;

    // Initialise grid with M cells. This is defined above, in this file.
    initialiseGrid( N, M );

    // Display the initial grid to terminal. This routine is defined in the extra file.
    printf( "Initial grid.\n" );
    displayGrid( grid, N, numCells(N) );         // DO NOT ALTER THIS LINE - displayGrid() must be called here, as part of the assessment.

    // Also initialise the graphical output if selected and available.
#ifdef GLFW
    GLFWwindow* window = initialiseGLFWWindow( N, grid );
#endif

    // Iterate through the loop and apply the rules.
    int iteration = 0;
    for( iteration=0; iteration<numIterations; iteration++ )
    {
        switch( rulesVersion )
        {
            case 0  :   iterateWithOriginalRules( N ); break;
            case 1  :   iterateWithModifiedRules( N ); break;

            default:
                return EXIT_FAILURE;        // Should never happen - rulesVersion has already been checked to be 0 or 1. 
        }

        // Output the current state of the grid to terminal as text.
        printf( "\nGrid after %d iteration(s):\n", iteration+1 );
        displayGrid( grid, N, numCells(N) );        // DO NOT ALTER THIS LINE - displayGrid() must be called here, as part of the assessment.

        // If graphics selected, update the window.
#ifdef GLFW
        displayImage( N, grid );
        glfwSwapBuffers( window );
		glfwPollEvents();
 #endif

        // A short pause before the next iteration. The argument is in microseconds. usleep() is defined in unistd.h
        usleep( 200000 );
   }

    // Close the graphics window once the user presses escape or 'q'.
#ifdef GLFW
	while( !glfwWindowShouldClose(window) ) { glfwPollEvents(); }
    glfwDestroyWindow( window );
    glfwTerminate();
#endif

}


