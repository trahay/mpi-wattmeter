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
#include <string.h>

static int comm_rank = -1;
static int comm_size = -1;
static char host_name[1024] = "";

void split() {
  int rank;
  int nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  char local_host_name[MPI_MAX_PROCESSOR_NAME];
  int  namelen;
  MPI_Get_processor_name(local_host_name,&namelen);
   
  size_t bytes = nprocs * sizeof(char[MPI_MAX_PROCESSOR_NAME]);
  char *host_names;

  host_names = malloc(bytes);
  strcpy(&host_names[rank*MPI_MAX_PROCESSOR_NAME], local_host_name);
 
  for (int n=0; n<nprocs; n++) { /* TODO: replace with a call to alltoall */
    MPI_Bcast(&(host_names[n*MPI_MAX_PROCESSOR_NAME]),MPI_MAX_PROCESSOR_NAME, MPI_CHAR, n, MPI_COMM_WORLD);
  }

  int color = 0;
  int found = 0;

  for (int n=0; n<nprocs && (!found) ; n++) {
    if(strcmp(&host_names[n*MPI_MAX_PROCESSOR_NAME], local_host_name) == 0) {
      if(n==rank) {
	/* i'm the local master */
	color = 1;
      }
      found = 1;     
    }
  }

  MPI_Comm local_comm;
  int local_rank=-1;
  int local_size=-1;
  MPI_Comm_split(MPI_COMM_WORLD,
		 color,
		 MPI_INFO_NULL,
		 &local_comm);
  if(color) {
    MPI_Comm_rank(local_comm, &local_rank);
    MPI_Comm_size(local_comm, &local_size);
  } else {
    MPI_Comm_free(&local_comm);
  }
  printf("(%s): My rank is %d. color=%d. local: %d/%d\n", local_host_name,
	 rank, color, local_rank, local_size);
}

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);
  split();

#if 0
  if (comm_size < 2) {
    fprintf(stderr,
            "This program requires at least 2 MPI processes, aborting...\n");
    goto out;
  }
  printf("(%s): My rank is %d\n", host_name, comm_rank);

  ping_side = !(comm_rank & 1);

  int size = 1024;
  int i;
  MPI_Barrier(MPI_COMM_WORLD);
  int data = 0;
  for (i = 0; i < iterations; i++) {
    if (!comm_rank) {
      MPI_Send(&data, 1, MPI_INTEGER, (comm_rank + 1) % comm_size, 0,
               MPI_COMM_WORLD);
      MPI_Recv(&data, 1, MPI_INTEGER,
               (comm_rank + comm_size - 1) % comm_size, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    } else {
      MPI_Recv(&data, 1, MPI_INTEGER,
               (comm_rank + comm_size - 1) % comm_size, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

      MPI_Send(&data, 1, MPI_INTEGER, (comm_rank + 1) % comm_size, 0,
               MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
#endif
 out:
  MPI_Finalize();

  return 0;
}
