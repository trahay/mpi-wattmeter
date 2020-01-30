/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright (C) CNRS, INRIA, Université Bordeaux 1, Télécom SudParis
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdarg.h>

// Debugging part, print out only if debugging level of the system is verbose or more
int _debug = -77;

void debug(char *fmt, ...) {
  if (_debug == -77) {
    char *buf = getenv("EZTRACE_DEBUG");
    if (buf == NULL)
      _debug = 0;
    else
      _debug = atoi(buf);
  }
  if (_debug >= 0) { // debug verbose mode
    va_list va;
    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    va_end(va);
  }
}
// end of debugging part

#define LEN      1024
#define LOOPS    10

static int comm_rank = -1;
static int comm_size = -1;
static char host_name[1024] = "";

static unsigned char *main_buffer = NULL;

/* fill the buffer */
void fill_buffer(unsigned char* buffer, int buffer_size) {
  int i;
  for (i = 0; i < buffer_size; i++)
    buffer[i] = 'a' + (i % 26);
}

/* fill the buffer with 0 */
void zero_buffer(unsigned char* buffer, int buffer_size) {
  int i;
  for (i = 0; i < buffer_size; i++)
    buffer[i] = 0;
}

/* check wether the buffer contains errornous data
 * return 1 if it contains an error
 */
int check_buffer(unsigned char* buffer, int buffer_size) {
  int i;
  for (i = 0; i < buffer_size; i++)
    if (buffer[i] != 'a' + (i % 26))
      return 1;
  return 0;
}

void compute(unsigned usec) {
  double t1, t2;
  t1 = MPI_Wtime();
  do {
    t2 = MPI_Wtime();
  } while ((t2 - t1) * 1e6 < usec);
}

int main(int argc, char **argv) {
  int ping_side;
  int rank_dst;
  int start_len = LEN;
  int iterations = LOOPS;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  if (gethostname(host_name, 1023) < 0) {
    perror("gethostname");
    exit(1);
  }

  if (comm_size < 2) {
    fprintf(stderr,
            "This program requires at least 2 MPI processes, aborting...\n");
    goto out;
  }

  debug("(%s): My rank is %d\n", host_name, comm_rank);

  ping_side = !(comm_rank & 1);

  main_buffer = malloc(start_len);
  fill_buffer(main_buffer, start_len);

  int size = 1024;
  int i;
  MPI_Barrier(MPI_COMM_WORLD);
  for (i = 0; i < iterations; i++) {
    if (!comm_rank) {
      MPI_Send(main_buffer, size, MPI_CHAR, (comm_rank + 1) % comm_size, 0,
               MPI_COMM_WORLD);
      MPI_Recv(main_buffer, size, MPI_CHAR,
               (comm_rank + comm_size - 1) % comm_size, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    } else {
      MPI_Recv(main_buffer, size, MPI_CHAR,
               (comm_rank + comm_size - 1) % comm_size, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      compute(100);
      MPI_Send(main_buffer, size, MPI_CHAR, (comm_rank + 1) % comm_size, 0,
               MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  out: free(main_buffer);
  MPI_Finalize();

  return 0;
}
