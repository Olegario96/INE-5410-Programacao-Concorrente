#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RANDNUM_W 521288629;
#define RANDNUM_Z 362436069;

unsigned int randum_w = RANDNUM_W;
unsigned int randum_z = RANDNUM_Z;

/* Função Geradora de um número aleatorio */
void srandnum(int seed) {
  unsigned int w, z;
  w = (seed * 104623) & 0xffffffff;
  randum_w = (w) ? w : RANDNUM_W;
  z = (seed * 48947) & 0xffffffff;
  randum_z = (z) ? z : RANDNUM_Z;
}

/* Função Geradora de um número aleatorio */
unsigned int randnum(void) {
  unsigned int u;
  randum_z = 36969 * (randum_z & 65535) + (randum_z >> 16);
  randum_w = 18000 * (randum_w & 65535) + (randum_w >> 16);
  u = (randum_z << 16) + randum_w;
  return (u);
}

typedef float* vector_t;

int npoints;
int dimension;
int ncentroids;
float mindistance;
int seed;
int size, rank;
// Data: todos os pontos
vector_t *data, *centroids, *gcentroids;
int *map;
int *buffer, *nbuffer;
int *dirty, *gdirty;
int too_far, gtoo_far;
int has_changed;
int gpopulation;
int init, end, tam;
MPI_Status st, stat;

/* Calcula a distância entre dois vetores */
float v_distance(vector_t a, vector_t b) {
  int i;
  float distance = 0;
  for (i = 0; i < dimension; i++)
    distance +=  pow(a[i] - b[i], 2);
  return sqrt(distance);
}

static void sync_map() {
    int i, j, k;
    if (rank !=  0) {
       if (!(buffer = malloc(tam*sizeof(int))))
           exit (1);
       for (i = init, j = 0; i < end; i++, j++)
           buffer[j] = map[i];

       MPI_Send(buffer, tam, MPI_INT, 0, 0, MPI_COMM_WORLD);
       free(buffer);
    } else {
        int initb, endb, tamb;
        for (k = 1; k < size; k++) {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);

            MPI_Get_count(&st, MPI_INT, &tamb);
            initb = (float)npoints*(float)st.MPI_SOURCE/size;
            endb = (float)npoints*(float)(st.MPI_SOURCE+1)/size;

            if (!(nbuffer = calloc(tamb, sizeof(int))))
                exit (1);

            MPI_Recv(nbuffer, tamb, MPI_INT, st.MPI_SOURCE, st.MPI_TAG, MPI_COMM_WORLD, &stat);

            for (i = initb, j = 0; i < endb; i++, j++)
                map[i] = nbuffer[j];
            free(nbuffer);
        }
    }
}

/* Calcula Centroid mais próximo de cada Ponto */
static void populate(void) {
  int i, j;
  float tmp;
  float distance;
  too_far = 0;

  for (i = init; i < end; i++) {
    distance = v_distance(centroids[map[i]], data[i]);
    /* Look for closest cluster. */
    for (j = 0; j < ncentroids; j++) {
      /* Point is in this cluster. */
      if (j == map[i]) continue;
      tmp = v_distance(centroids[j], data[i]);
      if (tmp < distance) {
        map[i] = j;
        distance = tmp;
        dirty[j] = 1;
      }
    }
    /* Cluster is too far away. */
    if (distance > mindistance)
      too_far = 1;
  }
  MPI_Allreduce(dirty, gdirty, ncentroids, MPI_INT, MPI_BOR, MPI_COMM_WORLD);
  MPI_Allreduce(&too_far, &gtoo_far, 1, MPI_INT, MPI_BOR, MPI_COMM_WORLD);
}


static void compute_centroids(void) {
  int i, j, k;
  int population;
  has_changed = 0;

  /* Compute means. */
  for (i = 0; i < ncentroids; i++) {
    if (!gdirty[i]) continue;
    memset(centroids[i], 0, sizeof(float) * dimension);
    /* Compute cluster's mean. */
    population = 0;
    for (j = init; j < end; j++) {
      if (map[j] != i) continue;
      for (k = 0; k < dimension; k++) {
        centroids[i][k] += data[j][k];
      }
      population++;
    }

    MPI_Allreduce(&population, &gpopulation, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(centroids[i], gcentroids[i], dimension, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    memcpy(centroids[i], gcentroids[i], dimension*sizeof(float));
    population = gpopulation;

    if (population > 1) {
      for (k = 0; k < dimension; k++)
        centroids[i][k] *= 1.0/population;
    }
    has_changed = 1;
  }
  memset(dirty, 0, ncentroids * sizeof(int));
  memset(gdirty, 0, ncentroids * sizeof(int));
}

int* kmeans(void) {
  int i, j, k;
  too_far = 0;
  has_changed = 0;
  if (!(map  = calloc(npoints, sizeof(int))))
    exit (1);
  if (!(dirty = malloc(ncentroids*sizeof(int))))
    exit (1);
  if (!(gdirty = calloc(ncentroids, sizeof(int))))
      exit (1);
  if (!(centroids = malloc(ncentroids*sizeof(vector_t))))
    exit (1);
  if (!(gcentroids = malloc(ncentroids*sizeof(vector_t))))
    exit (1);

  for (i = 0; i < ncentroids; i++) {
    centroids[i] = malloc(sizeof(float) * dimension);
    gcentroids[i] = malloc(sizeof(float) * dimension);
  }
  for (i = 0; i < npoints; i++)
    map[i] = -1;
  for (i = 0; i < ncentroids; i++) {
    dirty[i] = 1;
    j = randnum() % npoints;
    for (k = 0; k < dimension; k++)
      centroids[i][k] = data[j][k];
    map[j] = i;
  }
  /* Map unmapped data points. */
  for (i = 0; i < npoints; i++)
    if (map[i] < 0)
      map[i] = randnum() % ncentroids;

  do { /* Cluster data. */
    populate();
    compute_centroids();
  } while (gtoo_far && has_changed);

  sync_map();

  for (i = 0; i < ncentroids; i++) {
    free(centroids[i]);
    free(gcentroids[i]);
  }
  free(gcentroids);
  free(centroids);
  free(dirty);
  free(gdirty);

  return map;
}

int main(int argc, char **argv) {
  int i, j;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc != 6) {
    printf("Usage: npoints dimension ncentroids mindistance seed\n");
    exit (1);
  }

  npoints = atoi(argv[1]);
  dimension = atoi(argv[2]);
  ncentroids = atoi(argv[3]);
  mindistance = atoi(argv[4]);
  seed = atoi(argv[5]);

  srandnum(seed);

  init = (float)npoints*(float)rank/size;
  end = (float)npoints*(float)(rank+1)/size;
  tam = end - init;

  if (!(data = malloc(npoints*sizeof(vector_t))))
    exit(1);

  for (i = 0; i < npoints; i++) {
    data[i] = malloc(sizeof(float) * dimension);
    for (j = 0; j < dimension; j++)
      data[i][j] = randnum() & 0xffff;
  }

  map = kmeans();

  if (rank == 0) {
      for (i = 0; i < ncentroids; i++) {
        printf("\nPartition %d:\n", i);
        for (j = 0; j < npoints; j++)
          if(map[j] == i)
            printf("%d ", j);
      }
      printf("\n");
  }

  free(map);
  for (i = 0; i < npoints; i++)
    free(data[i]);
  free(data);

  MPI_Finalize();
  return (0);
}