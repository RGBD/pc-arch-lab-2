#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <mpi.h>

int t_id, t_count;
char *t_host;

char *log_prefix;

int g_m = 0;
int g_n = 0;
double **g_a = 0;
double *g_b = 0;
double *g_x = 0;

double **l_a = 0;
double *l_b = 0;
double *l_x = 0;

int l_start;
int l_end;

void mpi_before(int *argc, char **argv[]);
void mpi_after();

void read_data();
void broadcast_data();
void perform();
void gather_data();
void write_data();

void print_matrix(double **a, int m, int n);

int main(int argc, char *argv[])
{
  mpi_before(&argc, &argv);

  read_data();
  broadcast_data();
  perform();
  gather_data();

  write_data();

  mpi_after();
}

void mpi_before(int *argc, char **argv[]) {
  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &t_id);
  MPI_Comm_size(MPI_COMM_WORLD, &t_count);
  t_host = (char *) malloc(80 * sizeof(char));
  gethostname(t_host, 80);
  log_prefix = (char *) malloc(80 * sizeof(char));
  snprintf(log_prefix, 80, "[%s:%d/%d]:", t_host, t_id, t_count);
}

void mpi_after() {
  MPI_Finalize();
}

void read_data() {
  if (t_id == 0) {
    FILE *fin = fopen("in.txt", "r");
    fscanf(fin, "%d%d", &g_m, &g_n);

    printf("reading matrix %dx%d\n", g_m, g_n);

    g_b = (double *) malloc(g_n * sizeof(double));
    g_x = (double *) malloc(g_m * sizeof(double));
    g_a = (double **) malloc(g_m * sizeof(double));

    for (int i = 0; i < g_m; i++) {
      g_a[i] = (double *) malloc(g_n * sizeof(double));
      for (int j = 0; j < g_n; j++) {
        fscanf(fin, "%lf", &g_a[i][j]);
      }
    }
    fscanf(fin, "%*d");
    for (int i = 0; i < g_n; i++) {
      fscanf(fin, "%lf", &g_b[i]);
    }

    /*print_matrix(g_a, g_m, g_n);*/
    /*print_matrix(&g_b, 1, g_n);*/
  } else {
    /*printf("Non master process calling read data: %d\n", t_id);*/
  }
}

void broadcast_data() {
  MPI_Bcast(&g_m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&g_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  l_start = (t_id * g_m + t_count - 1) / t_count;
  l_end = ((t_id + 1) * g_m + t_count - 1) / t_count;
  if (t_id != 0) {
    g_a = (double **) malloc(g_m * sizeof(double *));
    for (int i = l_start; i < l_end; i++) {
      g_a[i] = (double *) malloc(g_n * sizeof(double));
    }
    g_b = (double *) malloc(g_n * sizeof(double));
    g_x = (double *) malloc(g_m * sizeof(double));
  }
  MPI_Bcast(g_b, g_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  if (t_id == 0) {
    for (int i = 0; i < g_m; i++) {
      int target = i * t_count / g_m;
      if (target != 0) {
        MPI_Send(g_a[i], g_n, MPI_DOUBLE, target, 0, MPI_COMM_WORLD);
      }
    }
  } else {
    for (int i = l_start; i < l_end; i++) {
      MPI_Recv(g_a[i], g_n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
    }
  }
}

void perform() {
  /*printf("%sm=%d n=%d\n", log_prefix, g_m, g_n);*/
  /*printf("%sl_start=%d l_end=%d\n", log_prefix, l_start, l_end);*/
  /*print_matrix(&g_b, 1, g_n);*/
  /*print_matrix(g_a, g_m, g_n);*/
  for (int i = l_start; i < l_end; i++) {
    double sum = 0.0;
    for (int j = 0; j < g_n; j++) {
      sum += g_a[i][j] * g_b[j];
    }
    g_x[i] = sum;
  }
  /*print_matrix(&g_x, 1, g_m);*/
}

void gather_data() {
  if (t_id != 0) {
    printf("%ssending data from %d [%d:%d]\n",
        log_prefix, t_id, l_start, l_end);
    MPI_Send(g_x + l_start, l_end - l_start, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  } else /*(t_id == 0)*/ {
    for (int i = 1; i < t_count; i++) {
      int f_start = (i * g_m + t_count - 1) / t_count;
      int f_end = ((i + 1) * g_m + t_count - 1) / t_count;
      printf("%sreceiving data from %d [%d:%d]\n",
          log_prefix, i, f_start, f_end);
      MPI_Recv(g_x + f_start, f_end - f_start, MPI_DOUBLE, i, 0,
          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
}

void write_data() {
  if (t_id == 0) {
    FILE *fout = fopen("out.txt", "w");
    fprintf(fout, "%d\n", g_m);

    for (int i = 0; i < g_m; i++) {
      fprintf(fout, "%lf ", g_x[i]);
    }
    fprintf(fout, "\n");

    fclose(fout);
  } else {
    /*printf("Non master process calling write data: %d\n", t_id);*/
  }
}

void print_matrix(double **a, int m, int n) {
  char result[4096] = "\0";
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      sprintf(result + strlen(result), "%6.2lf", a[i][j]);
    }
    sprintf(result + strlen(result), "\n");
  }
  sprintf(result + strlen(result), "----------------\n");
  printf("%s\n%s", log_prefix, result);
}
