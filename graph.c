#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    fprintf(stderr, "Error!\n");
    exit(1);
  }

  // header
  fscanf(fl, "%d %[^\n]", n, list);

  if (*n > 1)
  {
    data = (Node *)malloc((*n) * sizeof(Node));
    if (data == NULL)
    {
      fprintf(stderr, "Error!\n");
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

  char *t;
  int idmax = -1;
  float max = 0;
  char *auxptr;

  int nd = find_by_id(g, id, size);
  if (nd < 0)
    return max;

  char aux[MAX];
  strcpy(aux, g[nd].neighbors);
  t = strtok_r(aux, " ", &auxptr);

  do
  {
    int d = find_by_id(g, atoi(t), size);
    if (d >= 0 && max < g[d].value)
    {
      max = g[d].value;
      idmax = atoi(t);
    }
  } while ((t = strtok_r(NULL, " ", &auxptr)));

  float r = g[nd].value;

  if (idmax != -1)
    r += get_costly(g, idmax, size);

  return (r);
}

double get_elapsed_time(struct timeval start, struct timeval end)
{
  long seconds = end.tv_sec - start.tv_sec;
  long microseconds = end.tv_usec - start.tv_usec;
  double elapsed = seconds + microseconds * 1e-6;
  return elapsed;
}

/* Main program - calls read input, computes sum for each node in the list, prints output */
int main()
{
  char fileN[] = "input";
  char computelist[MAX];
  int n = -1;
  Node *input = NULL;

  struct timeval start_time, end_time;
  double elapsed_time;

  input = read_input(fileN, &n, computelist);

  gettimeofday(&start_time, NULL);

  // process
  for (char *c = strtok(computelist, " "); c != NULL; c = strtok(NULL, " "))
    printf("%s: %f\n", c, get_costly(input, atoi(c), n));

  gettimeofday(&end_time, NULL);

  elapsed_time = get_elapsed_time(start_time, end_time);
  printf("Tempo total: %f segundos\n", elapsed_time);

  free(input);

  return 0;
}
