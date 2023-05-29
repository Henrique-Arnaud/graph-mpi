#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

/* Maximum size (in char) for neighbors and compute lists */
#define MAX 100

typedef struct
{
  int id;
  float value;
  char neighbors[MAX];
} Node;

/* Reads input, allocates memory, writes graph size and node list. Returns the graph. */
Node *read_input(char *fname, int *n, char *list)
{
  Node *data = NULL;
  FILE *fl = NULL;

  if (fname == NULL)
    fl = stdin;
  else
    fl = fopen(fname, "r");

  if (fl == NULL)
  {
    fprintf(stderr, "Error opening file!\n");
    exit(1);
  }

  // header
  fscanf(fl, "%d %[^\n]", n, list);

  if (*n > 1)
  {
    data = (Node *)malloc((*n) * sizeof(Node));
    if (data == NULL)
    {
      fprintf(stderr, "Error allocating memory!\n");
      exit(1);
    }
  }

  for (int i = 0; i < *n; i++)
    fscanf(fl, "%d %f %[^\n]", &data[i].id, &data[i].value, data[i].neighbors);

  fclose(fl);

  return data;
}

/* Returns the index of the node with the given id or -1 if not found */
int find_by_id(Node g[], int id, int size)
{
  int i = 0;

  while ((i < size) && g[i].id != id)
    i++;

  if (i < size)
    return i;

  return -1;
}

/* Computes the sum for the node with the given id */
float get_costly(Node *g, int id, int size)
{
  float r = 0;
  int idmax = -1;
  float max = 0;
  char aux[MAX];
  char *t;
  char *auxptr;

  int nd;
  do
  {
    nd = find_by_id(g, id, size);
    if (nd >= 0)
    {
      r += g[nd].value;
      strcpy(aux, g[nd].neighbors);
      t = strtok_r(aux, " ", &auxptr);

      while (t != NULL)
      {
        int d = find_by_id(g, atoi(t), size);
        if (d >= 0 && max < g[d].value)
        {
          max = g[d].value;
          idmax = atoi(t);
        }
        t = strtok_r(NULL, " ", &auxptr);
      }
      id = idmax;
      max = 0;
      idmax = -1;
    }
  } while (id != -1);

  return r;
}

/* Main program - calls read input, computes sum for each node in the list, prints output */
int main(int argc, char **argv)
{
  int rank, num_procs;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  char fileN[] = "input";
  char computelist[MAX];
  int n = -1;
  Node *input = NULL;

  double start_time, end_time, total_time;

  if (rank == 0)
  {
    input = read_input(fileN, &n, computelist);
  }

  // Broadcasting the graph size (n) to all processes
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  Node *local_input = (Node *)malloc(n * sizeof(Node));

  if (rank == 0)
  {
    // Scatter the input data to all processes
    MPI_Scatter(input, n * sizeof(Node), MPI_BYTE, local_input, n * sizeof(Node), MPI_BYTE, 0, MPI_COMM_WORLD);
  }
  else
  {
    MPI_Scatter(NULL, 0, MPI_BYTE, local_input, n * sizeof(Node), MPI_BYTE, 0, MPI_COMM_WORLD);
  }

  // Process
  float *results = (float *)malloc(n * sizeof(float));
  start_time = MPI_Wtime();
  for (int i = 0; i < n; i++)
  {
    results[i] = get_costly(local_input, local_input[i].id, n);
  }
  end_time = MPI_Wtime();
  total_time = end_time - start_time;

  float *gathered_results = NULL;
  if (rank == 0)
  {
    gathered_results = (float *)malloc(n * sizeof(float));
  }

  // Gather the results to process 0
  MPI_Gather(results, n, MPI_FLOAT, gathered_results, n, MPI_FLOAT, 0, MPI_COMM_WORLD);

  if (rank == 0)
  {
    // Print the results
    for (int i = 0; i < n; i++)
    {
      printf("%d: %f\n", input[i].id, gathered_results[i]);
    }

    // Print the total time and speed-up
    printf("Total Time: %lf seconds\n", total_time);
  }

  free(input);
  free(local_input);
  free(results);
  free(gathered_results);

  MPI_Finalize();
  return 0;
}
