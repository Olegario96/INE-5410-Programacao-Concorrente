#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define RANDNUM_W 521288629
#define RANDNUM_Z 362436069
#define NUM_THREADS nthreads

unsigned int randum_w = RANDNUM_W;
unsigned int randum_z = RANDNUM_Z;

void srandnum(int seed) {
  unsigned int w, z;
  w = (seed * 104623) & 0xffffffff;
  randum_w = (w) ? w : RANDNUM_W;
  z = (seed * 48947) & 0xffffffff;
  randum_z = (z) ? z : RANDNUM_Z;
}

unsigned int randnum(void) {
  unsigned int u;
  randum_z = 36969 * (randum_z & 65535) + (randum_z >> 16);
  randum_w = 18000 * (randum_w & 65535) + (randum_w >> 16);
  u = (randum_z << 16) + randum_w;
  return (u);
}

struct parameters {
  int begin;
  int end;
};

// Onde há "vector_t" será um ponteiro para float
typedef float* vector_t;

// Número de pontos
int npoints;
// Número de dimensões
int dimension;
// Número de centroides (ou partições que serão feitas)
int ncentroids;
// Distância mínima aceitável

float mindistance;

int seed;

int nthreads;
// Ponteiro-ponteiro de data e centroid
// Data-> coordenadas
//
vector_t *data, *centroids;
// Associa um ponto a um centroide (enumera)
int *map;
// Flag para recalcular o centroide
int *dirty;
// "Avisa" se o centroide está muito longe (i.e. flag)
int too_far;
// "Avisa"
int has_changed;

// Cálculo da distância entre dois pontos
float v_distance(vector_t a, vector_t b) {
  int i;
  float distance = 0;
  for (i = 0; i < dimension; i++)
    distance +=  pow(a[i] - b[i], 2);
  return sqrt(distance);
}

static void *populate_parallel(void *arg) {
//  int ratio = (npoints) / NUM_THREADS;
  struct parameters *param = (struct parameters*) arg;
  int i, j;
  float tmp;
  float distance;
  for (i = param->begin; i < param->end; i++) {
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

}

static void populate(void) {
  pthread_t *threads = malloc(NUM_THREADS*sizeof(pthread_t));
  struct parameters *param = malloc(NUM_THREADS*sizeof(struct parameters));
  too_far = 0;
  for (int i = 0; i < NUM_THREADS; ++i) {
    param[i].begin = i*(npoints/NUM_THREADS);
    param[i].end = param[i].begin + (npoints/NUM_THREADS);
    pthread_create(&threads[i], NULL, *populate_parallel, (void *)&param[i]);
  }

  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_join(threads[i], NULL);
  }

  free(threads);
  free(param);
}

static void *compute_centroids_parallel(void *arg) {
  struct parameters *param = (struct parameters*) arg;
  int i, j, k;
  int population;
  /* Compute means. */
  for (i = param->begin; i < param->end; i++) {
    if (!dirty[i]) continue;
    memset(centroids[i], 0, sizeof(float) * dimension);
    /* Compute cluster's mean. */
    population = 0;
    for (j = 0; j < npoints; j++) {
      if (map[j] != i) continue;
      for (k = 0; k < dimension; k++)
        centroids[i][k] += data[j][k];
      population++;
    }
    if (population > 1) {
      for (k = 0; k < dimension; k++)
        centroids[i][k] *= 1.0/population;
    }
    has_changed = 1;
  }
}

static void compute_centroids(void) {
  pthread_t *threads = malloc(NUM_THREADS*sizeof(pthread_t));
  struct parameters *param = malloc(NUM_THREADS*sizeof(struct parameters));
  has_changed = 0;

  for (int i = 0; i < NUM_THREADS; ++i) {
    param[i].begin = i*(ncentroids/NUM_THREADS);
    param[i].end = param[i].begin + (ncentroids/NUM_THREADS);
    pthread_create(&threads[i], NULL, *compute_centroids_parallel, (void *)&param[i]);
  }

  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_join(threads[i], NULL);
  }
  memset(dirty, 0, ncentroids * sizeof(int));

  free(threads);
  free(param);
}

int* kmeans(void) {
  int i, j, k;
  too_far = 0;
  has_changed = 0;

  if (!(map  = calloc(npoints, sizeof(int))))
    exit (1);
  if (!(dirty = malloc(ncentroids*sizeof(int))))
    exit (1);
  if (!(centroids = malloc(ncentroids*sizeof(vector_t))))
    exit (1);

  for (i = 0; i < ncentroids; i++)
    centroids[i] = malloc(sizeof(float) * dimension);
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
  } while (too_far && has_changed);

  for (i = 0; i < ncentroids; i++)
    free(centroids[i]);
  free(centroids);
  free(dirty);

  return map;
}

int main(int argc, char **argv) {
  int i, j, tmp;

  if (argc != 7) {
    printf("Usage: npoints dimension ncentroids mindistance seed nthreads\n");
    exit (1);
  }

  // Definição através do usuário
  // Função atoi converte de char para int
  npoints = atoi(argv[1]);
  dimension = atoi(argv[2]);
  ncentroids = atoi(argv[3]);
  mindistance = atoi(argv[4]);
  seed = atoi(argv[5]);
  nthreads = atoi(argv[6]);

  srandnum(seed);

  if (!(data = malloc(npoints*sizeof(vector_t))))
    exit(1);

  for (i = 0; i < npoints; i++) {
    data[i] = malloc(sizeof(float) * dimension);
    for (j = 0; j < dimension; j++)
      data[i][j] = randnum() & 0xffff;
  }

  map = kmeans();

  free(map);
  for (i = 0; i < npoints; i++)
    free(data[i]);
  free(data);

  return (0);
}
