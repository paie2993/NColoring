#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "model.h"
#include "threads/threadedSort.h"
#include "distributed/distributedSort.h"


void printVertices(FILE *output, const int noVertices, const struct vertex *vertices);


// read the graph
void readGraph(FILE *input, graph *myGraph) {

    int vertices;
    int edges;

    fscanf(input, "%d", &vertices);
    fscanf(input, "%d", &edges);

    myGraph->noVertices = vertices;
    myGraph->noEdges = edges;
    myGraph->vertices = (struct vertex *) malloc(vertices * sizeof(struct vertex));

    for (int i = 0; i < vertices; i++) {
        myGraph->vertices[i] = (struct vertex) {
            .label = i, 
            .color = -1, 
            .noNeighbours = 0, 
            .neighbours = (struct vertex **) malloc((vertices - 1) * sizeof(struct vertex *)) 
        };
    }

    for (int i = 0; i < edges; i++) {
        int first;
        int second;
        fscanf(input, "%d %d", &first, &second);
        struct vertex *firstVertex = myGraph->vertices + first;
        struct vertex *secondVertex = myGraph->vertices + second;
        firstVertex->neighbours[firstVertex->noNeighbours++] = secondVertex;
        secondVertex->neighbours[secondVertex->noNeighbours++] = firstVertex;
    }

    // reallocate neighbours vector of each vertex of the graph
    for (int i = 0; i < vertices; i++) {
        struct vertex *currentVertex = myGraph->vertices + i;
        currentVertex->neighbours = (struct vertex **) realloc(currentVertex->neighbours, currentVertex->noNeighbours * sizeof(struct vertex *));
    } 
}




// deallocate graph
void deallocateGraph(graph *myGraph) {
    int noVertices = myGraph->noVertices;
    for (int i = 0; i < noVertices; i++) {
        free(myGraph->vertices[i].neighbours);
    }
    free(myGraph->vertices);
}





// neighbourhood check
bool areNeighbours(const struct vertex *first, const struct vertex *second) {
    for (int i = 0; i < first->noNeighbours; i++) {
        const struct vertex *currentNeighbour = first->neighbours[i];
        if (currentNeighbour->label == second->label) {
            return true;
        }
    }
    return false;
}






///////////////////////////////////////////////////////////////////////////////////////////////////
// extract only the labels of the vertices
void labels(const int noVertices, const struct vertex *vertices, int *labels) {
    for (int i = 0; i < noVertices; i++) {
        labels[i] = vertices[i].label;
    }
}

// based on the list of sorted vertex label, also sort the actual vertices
void orderSortedVertices(const int noVertices, struct vertex **vertices, const int *sortedLabels) {
    for (int i = 0; i < noVertices; i++) {
        int currentLabel = sortedLabels[i];
        struct vertex *currentVertex = vertices[currentLabel];
        struct vertex *swappedVertex = vertices[i];
        vertices[i] = currentVertex;
        vertices[currentLabel] = swappedVertex;
    }
}

// send sort request to sorting engine
void sortThreaded(const int no, int *labels) {
    threadedSort(no, labels);
}

void sortDistributed(const int no, int *labels) {
    ///
}





// pointer to vertex-pointers (because we want to modify the vertices' color)
// assume the vertices are ordered descendingly based on their degree
void color(const int noVertices, struct vertex **vertices) {
    int currentColor = 0;

    for (int i = 0; i < noVertices; i++) {
        struct vertex *currentVertex = vertices[i];
        if (currentVertex->color != -1) {
            continue;
        }
        currentVertex->color = currentColor;
        
        for (int j = i + 1; j < noVertices; j++) {
            struct vertex *secondaryVertex = vertices[j];
            if (areNeighbours(currentVertex, secondaryVertex)) {
                continue;
            }
            if (secondaryVertex->color != -1) {
                continue;
            }
            secondaryVertex->color = currentColor;
        }

        currentColor++;
    }
}


int main(const int argc, char **argv) {

    MPI_Init(0, 0);

    if (argc != 4) {
        exit(1);
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    // 1. read graph
    graph myGraph;
    readGraph(input, &myGraph);




    // 2. sort vertices descending by degree
    int *vertexLabels = (int *) malloc(myGraph.noVertices * sizeof(int));
    labels(myGraph.noVertices, myGraph.vertices, vertexLabels);

    struct vertex **sortedVertices = (struct vertex **) malloc(myGraph.noVertices * sizeof(struct vertex *));
    for (int i = 0; i < myGraph.noVertices; i++) {
        sortedVertices[i] = myGraph.vertices + i;
    }

    if (strcmp(argv[3], "-t") == 0) {
        sortThreaded(myGraph.noVertices, vertexLabels);
    } else if (strcmp(argv[3], "-m") == 0) {
        sortDistributed(myGraph.noVertices, vertexLabels);
    }
    
    orderSortedVertices(myGraph.noVertices, sortedVertices, vertexLabels);
    free(vertexLabels);
    // sortVertices(myGraph.noVertices, sortedVertices);



    // 3. color the vertices following the algorithm Welsh-Powell
    color(myGraph.noVertices, sortedVertices);
    free(sortedVertices);


    // 4. output
    printVertices(output, myGraph.noVertices, myGraph.vertices);
    deallocateGraph(&myGraph);


    
    fclose(output);
    fclose(input);

    MPI_Finalize();
    return 0;
}





// print the result
void printVertices(FILE *output, const int noVertices, const struct vertex *vertices) {
    for (int i = 0; i < noVertices; i++) {
        const struct vertex currentVertex = vertices[i];
        fprintf(output, "%d %d\n", currentVertex.label, currentVertex.color);
        
    }
    fflush(output);
}