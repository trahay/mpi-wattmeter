#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

static int N=4000;
#define NB_STEPS 50

/* uncomment this line to activate the debug mode */
//#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF //printf
#endif

double **cur_step;
double **next_step;

int comm_rank;
int comm_size;

/* initialize matrixes */
void init() {
  int i, j;

  cur_step = malloc(sizeof(double*) * N);
  next_step = malloc(sizeof(double*) * N);

  srand(time(NULL));
  for(i=0; i<N; i++) {
    cur_step[i] = calloc(N, sizeof(double));
    next_step[i] = calloc(N, sizeof(double));
  }

  int posx = rand()%N;
  int posy = rand()%N;
  cur_step[posx][posy] = N*comm_size*10000;
}

/* dump the matrix in f */
void print_matrix(FILE* f,double** matrix)
{
  int i, j;
  for(i=1; i<N-1; i++) {
    for(j=1; j<N-1; j++) {
      fprintf(f, "%.2f  ", matrix[i][j]);
    }
    fprintf(f, "\n");
  }
}

/* dump all the submatrixes in a file */
void print_all(FILE* f)
{
  int i, j;
  double ** tmp_matrix = NULL;

  if(! comm_rank) {
    /* master node gather the submatrixes and write */
    tmp_matrix = malloc(sizeof(double*)*N);
    for(i=0; i<N; i++) {
      tmp_matrix[i] = malloc(sizeof(double)*N);
    }

    /* first, print the local matrix */
    print_matrix(f, cur_step);

    for(i=0; i<comm_size; i++) {
      if(i != comm_rank) {
	/* wake up the slave */
	MPI_Send(&i, 1, MPI_INT, i, 999, MPI_COMM_WORLD);
	/* receive the remote matrix */
	for(j=1;j<N-1;j++) {
	  MPI_Recv(tmp_matrix[j], N, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	print_matrix(f, tmp_matrix);
      }
    }
  } else {
    int i;
    /* wait for the master node */
    MPI_Recv(&i, 1, MPI_INT, 0, 999, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    /* send the matrix */
    for(i=1;i<N-1;i++) {
      MPI_Send(cur_step[i], N, MPI_DOUBLE, 0, comm_rank, MPI_COMM_WORLD);
    }

  }

}


void compute()
{
  int i, j;
  MPI_Request r1, r2;

#pragma omp parallel for private(i, j)
  for(i=1; i< N-1; i++) {

    for(j=1; j< N-1; j++) {
      next_step[i][j] = (cur_step[i-1][j] + cur_step[i+1][j] +
			 cur_step[i][j-1] + cur_step[i][j+1] +
			 cur_step[i][j]) / 5;
    }

    if(i == 1 && comm_rank>0) {
      /* there's an upper neighbour */
      MPI_Isend(next_step[1], N, MPI_DOUBLE, comm_rank-1, 0, MPI_COMM_WORLD, &r1);
    }

    if(i == N-2 &&   comm_rank<comm_size-1) {
      MPI_Isend(next_step[N-2], N, MPI_DOUBLE, comm_rank+1, 0, MPI_COMM_WORLD, &r2);
    }
  }

  DEBUG_PRINTF("rank #%d: compute done\n", comm_rank);

  if(comm_rank>0) {
    MPI_Recv(next_step[0], N, MPI_DOUBLE, comm_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Wait(&r1, MPI_STATUS_IGNORE);
  }

  if(comm_rank<comm_size-1) {
    /* there's a lower neighbour */
    MPI_Recv(next_step[N-1], N, MPI_DOUBLE, comm_rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Wait(&r2, MPI_STATUS_IGNORE);
  }

  DEBUG_PRINTF("rank #%d: receive_borders done\n", comm_rank);
  double ** tmp = cur_step;
  cur_step = next_step;
  next_step = tmp;
}


int main(int argc, char**argv) {

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  if(argc>1){
    N = atoi(argv[1]);
  }

  struct timeval t1, t2;
  if(!comm_rank)
    printf("Initialization (problem size: %d)\n", N);
  init();

  MPI_Barrier(MPI_COMM_WORLD);
  if(!comm_rank)
    printf("Start computing (%d steps)\n", NB_STEPS);

  gettimeofday(&t1, NULL);
  int i;

  for(i=0; i< NB_STEPS; i++) {
    if(!comm_rank)
      printf("STEP %d...\n", i);
    compute();
  }

  gettimeofday(&t2, NULL);
  MPI_Barrier(MPI_COMM_WORLD);

  if(comm_rank==0) {
    double total_time = ((t2.tv_sec-t1.tv_sec)*1e6 + (t2.tv_usec - t1.tv_usec))/1e6;
    printf("%d steps in %lf sec (%lf sec/step)\n",
	   NB_STEPS, total_time, total_time/NB_STEPS);
  }

  if(argc>2){
    if(!comm_rank)
      printf("dumping the result data in %s\n", argv[2]);
    FILE *f = fopen(argv[2], "w");
    print_all(f);
  }

  MPI_Finalize();
  return 0;
}
