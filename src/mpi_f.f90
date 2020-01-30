! Copyright (C)Télécom SudParis
! See COPYING in top-level directory.

subroutine MPI_COMM_SPAWN(CMD, ARGV, M, I, R, C, INTERCOMM, A, E)
call MPIF_COMM_SPAWN(CMD, ARGV, M, I, R, C, INTERCOMM, A, E)
return
end

subroutine MPI_INIT(error)
call MPIF_INIT(error)
return
end

subroutine MPI_Init_thread(REQUIRED, PROVIDED, IERROR)
call MPIF_Init_thread(REQUIRED, PROVIDED, IERROR)
return
end

subroutine MPI_Finalize(IERROR)
call MPIF_Finalize(IERROR)
return
end

subroutine MPI_Barrier(COMM, IERROR)
call MPIF_Barrier(COMM, IERROR)
return
end

subroutine MPI_Comm_size(COMM, SIZE, IERROR)
call MPIF_Comm_size(COMM, SIZE, IERROR)
return
end

subroutine MPI_Comm_rank(COMM, RANK, IERROR)
call MPIF_Comm_rank(COMM, RANK, IERROR)
return
end

subroutine MPI_Comm_create(COMM, GROUP, NEWCOMM, IERROR)
call MPIF_Comm_create(COMM, GROUP, NEWCOMM, IERROR)
return
end

subroutine MPI_Comm_create_group(COMM, GROUP, TAG, NEWCOMM, IERROR)
call MPIF_Comm_create_group(COMM, GROUP, TAG, NEWCOMM, IERROR)
return
end

subroutine MPI_Comm_split(COMM, COLOR, KEY, NEWCOMM, IERROR)
call MPIF_Comm_split(COMM, COLOR, KEY, NEWCOMM, IERROR)
return
end

subroutine MPI_Comm_dup(COMM, NEWCOMM, IERROR)
call MPIF_Comm_dup(COMM, NEWCOMM, IERROR)
return
end

subroutine MPI_Comm_dup_with_info(COMM, info, NEWCOMM, IERROR)
call MPIF_Comm_dup_with_info(COMM, info, NEWCOMM, IERROR)
return
end

subroutine MPI_COMM_SPLIT_TYPE(COMM, SPLIT_TYPE, KEY, INFO, NEWCOMM, IERROR)
call MPIF_COMM_SPLIT_TYPE(COMM, SPLIT_TYPE, KEY, INFO, NEWCOMM, IERROR)
return
end

subroutine MPI_Intercomm_create(LOCAL_COMM, LOCAL_LEADER, PEER_COMM, REMOTE_LEADER, TAG, NEWINTERCOMM, IERROR)
call MPIF_Intercomm_create(LOCAL_COMM, LOCAL_LEADER, PEER_COMM, REMOTE_LEADER, TAG, NEWINTERCOMM, IERROR)
return
end

subroutine MPI_INTERCOMM_MERGE(INTERCOMM, HIGH, NEWINTRACOMM, IERROR)
call MPIF_INTERCOMM_MERGE(INTERCOMM, HIGH, NEWINTRACOMM, IERROR)
return
end

subroutine MPI_CART_SUB(COMM, REMAIN_DIMS, COMM_NEW, IERROR)
call MPIF_CART_SUB(COMM, REMAIN_DIMS, COMM_NEW, IERROR)
return
end

subroutine MPI_CART_CREATE(COMM_OLD, NDIMS, DIMS, PERIODS, REORDER, COMM_CART, IERROR)
call MPIF_CART_CREATE(COMM_OLD, NDIMS, DIMS, PERIODS, REORDER, COMM_CART, IERROR)
return
end

subroutine MPI_GRAPH_CREATE(COMM_OLD, NNODES, INDEX, EDGES, REORDER, COMM_GRAPH, IERROR)
call MPIF_GRAPH_CREATE(COMM_OLD, NNODES, INDEX, EDGES, REORDER, COMM_GRAPH, IERROR)
return
end

subroutine MPI_DIST_GRAPH_CREATE(COMM_OLD, N, SOURCES, DEGREES, DESTINATIONS, WEIGHTS,&
                INFO, REORDER, COMM_DIST_GRAPH, IERROR)
call MPIF_DIST_GRAPH_CREATE(COMM_OLD, N, SOURCES, DEGREES, DESTINATIONS, WEIGHTS,&
                INFO, REORDER, COMM_DIST_GRAPH, IERROR)
return
end

subroutine MPI_DIST_GRAPH_CREATE_ADJACENT(COMM_OLD, INDEGREE, SOURCES, SOURCEWEIGHTS, &
     OUTDEGREE, DESTINATIONS, DESTWEIGHTS, INFO, REORDER, COMM_DIST_GRAPH, IERROR)
call  MPIF_DIST_GRAPH_CREATE_ADJACENT(COMM_OLD, INDEGREE, SOURCES, SOURCEWEIGHTS, &
     OUTDEGREE, DESTINATIONS, DESTWEIGHTS, INFO, REORDER, COMM_DIST_GRAPH, IERROR)
return
end


subroutine MPI_Send(BUF, COUNT, DATATYPE, DEST, TAG, COMM, IERROR)
call MPIF_Send(BUF, COUNT, DATATYPE, DEST, TAG, COMM, IERROR)
return
end

subroutine MPI_Recv(BUF, COUNT, D, SRC, TAG, COMM, STATUS, IERROR)
call MPIF_Recv(BUF, COUNT, D, SRC, TAG, COMM, STATUS, IERROR)
return
end

subroutine MPI_Sendrecv(SBUF, SCOUNT, STYPE, DST, STAG, RBUF, RCOUNT, RTYPE, SRC, RTAG, COMM, STATUS, ERROR)
call MPIF_Sendrecv(SBUF, SCOUNT, STYPE, DST, STAG, RBUF, RCOUNT, RTYPE, SRC, RTAG, COMM, STATUS, ERROR)
return
end

subroutine MPI_Sendecv_replace(BUF, COUNT, TYPE, DST, STAG, SRC, RTAG, COMM, STATUS, ERROR)
call MPIF_Sendrecv_replace(BUF, COUNT, TYPE, DST, STAG, SRC, RTAG, COMM, STATUS, ERROR)
return
end

subroutine MPI_Bsend(BUF, COUNT,DATATYPE, DEST, TAG, COMM, IERROR)
call MPIF_Bsend(BUF, COUNT,DATATYPE, DEST, TAG, COMM, IERROR)
return
end

subroutine MPI_Ssend(BUF, COUNT, D, DEST, TAG, COMM, IERROR)
call MPIF_Ssend(BUF, COUNT, D, DEST, TAG, COMM, IERROR)
return
end

subroutine MPI_Rsend(BUF, COUNT, D, DEST, TAG, COMM, IERROR)
call MPIF_Rsend(BUF, COUNT, D, DEST, TAG, COMM, IERROR)
return
end

subroutine MPI_Isend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
call MPIF_Isend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
return
end

subroutine MPI_Ibsend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
call MPIF_Ibsend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
return
end

subroutine MPI_Issend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
call MPIF_Issend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
return
end

subroutine MPI_Irsend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
call MPIF_Irsend(BUF, COUNT, D, DST, TAG, COMM, REQ, IERROR)
return
end

subroutine MPI_Irecv(B, CNT, D, SRC, TAG, COMM, REQ, IERROR)
call MPIF_Irecv(B, CNT, D, SRC, TAG, COMM, REQ, IERROR)
return
end

subroutine MPI_Wait(REQUEST, STATUS, IERROR)
call MPIF_Wait(REQUEST, STATUS, IERROR)
return
end

subroutine MPI_WAITALL(COUNT, R, S, IERROR)
call MPIF_Waitall(COUNT, R, S, IERROR)
return
end

subroutine MPI_WAITANY(COUNT, R, S, INDEX, IERROR)
call MPIF_Waitany(COUNT, R, S, INDEX, IERROR)
return
end

subroutine MPI_WAITSOME(IC, R, OC, INDEX, S, IERROR)
call MPIF_Waitsome(IC, R, OC, INDEX, S, IERROR)
return
end

subroutine MPI_TESTSOME(IC, R, OC, INDEX, S, IERROR)
call MPIF_Testsome(IC, R, OC, INDEX, S, IERROR)
return
end


subroutine MPI_PROBE(S, T, C, STAT, IERROR)
call MPIF_PROBE(S, T, C, STAT, IERROR)
return
end

subroutine MPI_IPROBE(S, T, C, FLAG, STAT, IERROR)
call MPIF_IPROBE(S, T, C, FLAG, STAT, IERROR)
return
end


subroutine MPI_Test(REQUEST, FLAG, STATUS, IERROR)
call MPIF_Test(REQUEST, FLAG, STATUS, IERROR)
return
end

subroutine MPI_Testall(C, REQUEST, INDEX, STATUS, IERROR)
call MPIF_Testall(C, REQUEST, INDEX, STATUS, IERROR)
return
end

subroutine MPI_Testany(C, REQUEST, INDEX, F, STATUS, IERROR)
call MPIF_Testany(C, REQUEST, INDEX, F, STATUS, IERROR)
return
end

subroutine MPI_Get(O_A, O_C, O_D, T_R, T_DS, T_C, T_D, W, ERROR)
call MPIF_Get(O_A, O_C, O_D, T_R, T_DS, T_C, T_D, W, ERROR)
return
end

subroutine MPI_Put(O_A, O_C, O_D, T_R, T_DS, T_C, T_D, W, ERROR)
call MPIF_Put(O_A, O_C, O_D, T_R, T_DS, T_C, T_D, W, ERROR)
return
end

subroutine MPI_BCast(BUFFER, COUNT, DATATYPE, ROOT, COMM, IERROR)
call MPIF_BCast(BUFFER, COUNT, DATATYPE, ROOT, COMM, IERROR)
return
end

subroutine MPI_Gather(SBUF,SCNT,STYPE,RBUF,RCNT, RTYPE, R, C, E)
call MPIF_Gather(SBUF, SCNT, STYPE, RBUF, RCNT, RTYPE, R, C, E)
return
end

subroutine MPI_Gatherv(SBUF,SCNT,STYPE,RBUF,RCNT,DSP,RTYPE,R,C,E)
call MPIF_Gatherv(SBUF,SCNT,STYPE,RBUF,RCNT,DSP,RTYPE,R,C,E)
return
end

subroutine MPI_Scatter(SBUF,SCNT,STYPE,RBUF,RCNT,RTYPE,R,C,E)
call MPIF_Scatter(SBUF, SCNT, STYPE, RBUF, RCNT, RTYPE, R, C, E)
return
end

subroutine MPI_Scatterv(SBUF,SCNT,DSP,SDTYP,RBUF,RCNT,RTYP,R,C,E)
call  MPIF_Scatterv(SBUF,SCNT,DSP,SDTYP,RBUF, RCNT, RTYP, R, C, E)
return
end

subroutine MPI_Allgather(SBUF, SCNT, STYP, RBUF, RCNT, RTYP, C, E)
call MPIF_Allgather(SBUF, SCNT, STYP, RBUF, RCNT, RTYP, C, E)
return
end

subroutine MPI_Allgatherv(SB, SC, ST, RB, RC, D, RT, C, IERROR)
call MPIF_Allgatherv(SB, SC, ST, RB, RC, D, RT, C, IERROR)
return
end

subroutine MPI_Alltoall(SB, SC, ST, RB, RC, RT, COMM, IERROR)
call MPIF_Alltoall(SB, SC, ST, RB, RC, RT, COMM, IERROR)
return
end

subroutine MPI_Alltoallv(SB, SC, SD, ST, RB, RC, RD, RT, C, ERROR)
call MPIF_Alltoallv(SB, SC, SD, ST, RB, RC, RD, RT, C, ERROR)
return
end

subroutine MPI_Reduce(SBUF, RBUF, CNT, D, OP, ROOT, COMM, IERROR)
call MPIF_Reduce(SBUF, RBUF, CNT, D, OP, ROOT, COMM, IERROR)
return
end

subroutine MPI_Allreduce(SBUF, RBUF, CNT, D, OP, COMM, IERROR)
call MPIF_Allreduce(SBUF, RBUF, CNT, D, OP, COMM, IERROR)
return
end

subroutine MPI_Reduce_scatter(SBUF, RBUF, RCNT, D, OP, C, IERROR)
call MPIF_Reduce_scatter(SBUF, RBUF, RCNT, D, OP, C, IERROR)
return
end

subroutine MPI_Scan(SBUF, RBUF, CNT, D, OP, COMM, IERROR)
call  MPIF_Scan(SBUF, RBUF, CNT, D, OP, COMM, IERROR)
return
end subroutine MPI_Scan

subroutine MPI_CANCEL(REQUEST, IERROR)
call MPIF_Cancel(REQUEST, IERROR)
return
end subroutine MPI_CANCEL

subroutine MPI_TYPE_SIZE(TYPE, SIZE, IERROR)
  call MPIF_TYPE_SIZE(TYPE, SIZE, IERROR)
  return
end subroutine MPI_TYPE_SIZE

