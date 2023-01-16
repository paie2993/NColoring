#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct edge {
    int first;
    int second;
};

struct edge* generate(const int vertices, int *edges) {

    *edges = 0;
    int maxEdges = vertices * (vertices - 1) / 2;
    struct edge *graph = (struct edge *) malloc(maxEdges * sizeof(struct edge));
    
    for (int i = 0; i < vertices - 1; i++) {
        int noEdges = rand() % vertices - i;
        for (int j = i + 1; j < i + 1 + noEdges; j++) {
            graph[*edges] = (struct edge) { .first = i, .second = j };
            *edges = *edges + 1;
        }
    }

    return graph;
}

void printGraph(int vertices, int edges, struct edge* graph, FILE* fd) {
    fprintf(fd, "%d\n", vertices);
    fprintf(fd, "%d\n", edges);
    for (int i = 0; i < edges; i++) {
        fprintf(fd, "%d %d\n", graph[i].first, graph[i].second);
        fflush(fd);
    }
}

int main(const int argc, const char **argv) {

    if (argc != 3) {
        exit(1);
    }

    srand(time(NULL));

    const int vertices = atoi(argv[1]);
    const char *file = argv[2];

    FILE *fd = fopen(file, "w");

    int edges;
    struct edge* graph = generate(vertices, &edges);

    printGraph(vertices, edges, graph, fd);
    free(graph);
    fclose(fd);

    return 0;
}