
#ifndef CLUSTER_H_
#define CLUSTER_H_

/* Chapter 2 */
double clusterdistance (int nrows, int ncolumns, double** data, int** mask,
  double weight[], int n1, int n2, int index1[], int index2[], char dist,
  char method, int transpose);
double** distancematrix (int ngenes, int ndata, double** data,
  int** mask, double* weight, char dist, int transpose);

/* Chapter 3 */
int getclustercentroids(int nclusters, int nrows, int ncolumns,
  double** data, int** mask, int clusterid[], double** cdata, int** cmask,
  int transpose, char method);
void getclustermedoids(int nclusters, int nelements, double** distance,
  int clusterid[], int centroids[], double errors[]);
void kcluster (int nclusters, int ngenes, int ndata, double** data,
  int** mask, double weight[], int transpose, int npass, char method, char dist,
  int clusterid[], double* error, int* ifound);
void kmedoids (int nclusters, int nelements, double** distance,
  int npass, int clusterid[], double* error, int* ifound);

/* Chapter 4 */
typedef struct {int left; int right; double distance;} Node;
/*
 * A Node struct describes a single node in a tree created by hierarchical
 * clustering. The tree can be represented by an array of n Node structs,
 * where n is the number of elements minus one. The integers left and right
 * in each Node struct refer to the two elements or subnodes that are joined
 * in this node. The original elements are numbered 0..nelements-1, and the
 * nodes -1..-(nelements-1). For each node, distance contains the distance
 * between the two subnodes that were joined.
 */

Node* treecluster (int nrows, int ncolumns, double** data, int** mask,
  double weight[], int transpose, char dist, char method, double** distmatrix);
void cuttree (int nelements, Node* tree, int nclusters, int clusterid[]);

/* Chapter 5 */
void somcluster (int nrows, int ncolumns, double** data, int** mask,
  const double weight[], int transpose, int nxnodes, int nynodes,
  double inittau, int niter, char dist, double*** celldata,
  int clusterid[][2]);

/* Chapter 6 */
int pca(int m, int n, double** u, double** v, double* w);

/* Utility routines, currently undocumented */
void sort(int n, const double data[], int index[]);
double mean(int n, double x[]);
double median (int n, double x[]);

double* calculate_weights(int nrows, int ncolumns, double** data, int** mask,
  double weights[], int transpose, char dist, double cutoff, double exponent);

#endif /* CLUSTER_H_ */
