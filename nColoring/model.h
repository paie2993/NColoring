// model
struct vertex {

    int label;
    int color;

    int noNeighbours;
    struct vertex **neighbours;

};

typedef struct {

    int noVertices;
    int noEdges;

    struct vertex *vertices;

} graph;
