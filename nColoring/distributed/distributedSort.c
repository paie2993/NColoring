#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "distributedSort.h"

// this is the task of workers
void sortPart(const int no, int *array) {
    for (int i = 0; i < no - 1; i++) {
        for (int j = 0; j < no - 1 - i; j++) {
            if (array[j] <= array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}


// compute the bounds (the displacements for sending parts of array)
void compBounds(const int no, const int workers, int *bounds) {
    for (int i = 0; i < workers; i++) {
        bounds[i] = i * no / workers;
    }
}

// compute the counts (the number of elements in the receiving buffer of each worker)
void compCounts(const int no, const int workers, const int *bounds, int *counts) {
    for (int i = 0; i < workers - 1; i++) {
        counts[i] = bounds[i + 1] - bounds[i];
    }
    counts[workers - 1] = no - bounds[workers - 1];
}


// merge two arrays into a third array, keeping them sorted
void merge(const int noFirst, const int noSecond, const int *first, const int* second, int *third) {
    int i = 0;
    int j = 0;
    int k = 0;

    while (i < noFirst && j < noSecond) {
        if (first[i] >= second[j]) {
            third[k++] = first[i++];
        } else {
            third[k++] = second[j++];
        }
    }

    while (i < noFirst) {
        third[k++] = first[i++];
    }

    while(j < noSecond) {
        third[k++] = second[j++];
    }
}



void sort(const int no, int *array, const int workers, const int myRank) {

    if (no > workers) {
        printf("No point in using distributed system. Sorting locally\n");
        sortPart(no, array);
        return;
    }

    // displacements
    int bounds[workers];
    compBounds(no, workers, bounds);

    // recv counts
    int counts[workers];
    compCounts(no, workers, bounds, counts);
    
    // recv count for each worker
    int myCount = counts[myRank];

    // recv buffer
    int *recvArray = (int *) malloc(counts[myRank] * sizeof(int));


    MPI_Scatterv(array, counts, bounds, MPI_INT, recvArray, myCount, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Received\n");
    for (int i = 0; i < myCount; i++) {
         printf("%d ", recvArray[i]);
    }

    // this is done on each worker
    sortPart(myCount, recvArray);

    // gather in root all the parts
    MPI_Gatherv(recvArray, myCount, MPI_INT, array, counts, bounds, MPI_INT, 0, MPI_COMM_WORLD);


    // split the results gathered from workers /////////
    int **arrays = (int **) malloc(workers * sizeof(int *));
    for (int i = 0; i < workers; i++) {
        arrays[i] = (int *) malloc(counts[i] * sizeof(int)); 
    }

    for (int i = 0; i < workers; i++) {
        int start = bounds[i];
        int end = start + counts[i];
        for (int j = start; j < end; j++) {
            int *workerArray = arrays[i];
            workerArray[j - start] = recvArray[j];
        } 
        
    }

    // merge the results from the workers
    int *first = array;
    int noFirst = counts[0];
    for (int i = 1; i < workers; i++) {
        int *second = arrays[i];
        int noSecond = counts[i];
        merge(noFirst, noSecond, first, second, array);
        noFirst = noFirst + noSecond;
    } // in the end, array will be sorted (sorting in-place)



    for (int i = 0; i < workers; i++) {
        free(arrays[i]);
    }
    free(arrays);
    free(recvArray);
}



void distributedSort(const int no, int *array) {
    
    int workers;
    MPI_Comm_size(MPI_COMM_WORLD, &workers);

    int myRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    sort(no, array, workers, myRank);

}