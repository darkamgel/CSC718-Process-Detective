
#include <stdio.h>
#include <mpi.h>

#define SIZE 5

int main(int argc, char *argv[]) {
    int rank, size;
    int arr[SIZE] = {0};

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get process rank and total processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Rank 0 initializes the array
    if (rank == 0) {
        for (int i = 0; i < SIZE; i++) {
            arr[i] = (i + 1) * 10;
        }
        printf("Broadcasting array from Rank 0\n");
    }

    // Broadcast array from Rank 0 to all processes
    MPI_Bcast(arr, SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process prints the received array
    printf("Rank %d received: ", rank);
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Finalize MPI
    MPI_Finalize();
    return 0;
}


/*
Imagine training a machine learning model across four computers. One computer
 has the initial model parameters, and the others need those same parameters before beginning computation.
Instead of manually sending the data to each computer using multiple MPI_Send() calls,
 we can use a single collective operation, MPI_Bcast(), called by every participating process.
*/

// MPI_Bcast() — All processes receive the same data.
// MPI_Scatter() — Different processes receive different portions of the data.

// Rank 0 starts with the data, MPI_Bcast() distributes it, and every MPI process verifies that it received the same values.