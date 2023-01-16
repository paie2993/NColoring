#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include "threadedSort.h"

const int THREADS = 8;

// copy section left-right from source array into the destination array
void copy(const int left, const int right, const int *source, int *destination);

// merge to sorted halves of an array into the destination array
void merge(const int leftNo, const int rightNo, const int *left, const int *right, int *destination);


typedef struct {
    const int no;
    int *array;
} args;



// a merge-sort algorithm. Sorts the given array descendingly
void *sort(void *arguments) {

    args *real = (args *) arguments;

    const int no = real->no;
    int *array = real->array;

    if (no <= 0) {
        printf("Can't sort array with 0 or less elements\n");
        exit(1);
    }

    if (no == 1) {
        return NULL; // an array of length 1 is already sorted
    }

    // bounds                                
    int middle = no / 2;        
    
    int leftStart = 0;          
    int leftEnd = middle;       
    int leftLength = leftEnd - leftStart;

    int rightStart = middle;    
    int rightEnd = no;          
    int rightLenght = rightEnd - rightStart;

    // left and right side
    int *left = (int *) malloc(leftLength * sizeof(int));
    int *right = (int *) malloc(rightLenght * sizeof(int));
    
    copy(leftStart, leftEnd, array, left);
    copy(rightStart, rightEnd, array, right);
    
    // sort each laft

    args leftArguments = (args) { .no = leftLength, .array = left };
    args rightArguments = (args) { .no = rightLenght, .array = right };

    pthread_t t1, t2;
    pthread_create(&t1, NULL, sort, (void *)&leftArguments);
    pthread_create(&t2, NULL, sort, (void *)&rightArguments);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // merge the halves into the original array
    merge(leftLength, rightLenght, left, right, array);

    free(left);
    free(right);

    return NULL;
}




// copy section left-right from source array into the destination array
void copy(const int left, const int right, const int *source, int *destination) {
    int destinationLength = right - left;
    for (int i = 0; i < destinationLength; i++) {
        destination[i] = source[i + left];
    }
}



// merge to sorted halves of an array into the destination array
void merge(const int leftNo, const int rightNo, const int *left, const int *right, int *destination) {

    int i = 0; // left index
    int j = 0; // right index
    int k = 0; // destination index

    while (i < leftNo && j < rightNo) {
        if (left[i] >= right[j]) { // descending ordering
            destination[k++] = left[i++];
        } else {
            destination[k++] = right[j++];
        }
    }

    while (i < leftNo) {
        destination[k++] = left[i++];
    }

    while (j < rightNo) {
        destination[k++] = right[j++];
    }
}


void threadedSort(const int no, int *array) {
    args arguments = (args) { .no = no, .array = array };
    sort((void *)&arguments);    
}
