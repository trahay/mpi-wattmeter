! These routines are specific to MPI3
subroutine MPI_IBarrier(COMM, REQ, IERROR)
call MPIF_IBarrier(COMM, REQ, IERROR)
return
end

subroutine MPI_IBCast(BUFFER, COUNT, DATATYPE, ROOT, COMM, REQ, IERROR)
call MPIF_IBCast(BUFFER, COUNT, DATATYPE, ROOT, COMM, REQ, IERROR)
return
end

subroutine MPI_IGather(SBUF,SCNT,STYPE,RBUF,RCNT, RTYPE, R, C, REQ, E)
call MPIF_IGather(SBUF, SCNT, STYPE, RBUF, RCNT, RTYPE, R, C, REQ, E)
return
end

subroutine MPI_IGatherv(SBUF,SCNT,STYPE,RBUF,RCNT,DSP,RTYPE,R,C,REQ,E)
call MPIF_IGatherv(SBUF,SCNT,STYPE,RBUF,RCNT,DSP,RTYPE,R,C,REQ,E)
return
end

subroutine MPI_IScatter(SBUF,SCNT,STYPE,RBUF,RCNT,RTYPE,R,C,REQ,E)
call MPIF_IScatter(SBUF, SCNT, STYPE, RBUF, RCNT, RTYPE, R, C,REQ, E)
return
end

subroutine MPI_IScatterv(SBUF,SCNT,DSP,SDTYP,RBUF,RCNT,RTYP,R,C,REQ,E)
call  MPIF_IScatterv(SBUF,SCNT,DSP,SDTYP,RBUF, RCNT, RTYP, R, C, REQ,E)
return
end

subroutine MPI_IAllgather(SBUF, SCNT, STYP, RBUF, RCNT, RTYP, C, REQ, E)
call MPIF_IAllgather(SBUF, SCNT, STYP, RBUF, RCNT, RTYP, C, REQ, E)
return
end

subroutine MPI_IAllgatherv(SB, SC, ST, RB, RC, D, RT, C, REQ,IERROR)
call MPIF_IAllgatherv(SB, SC, ST, RB, RC, D, RT, C, REQ,IERROR)
return
end

subroutine MPI_IAlltoall(SB, SC, ST, RB, RC, RT, COMM, REQ,IERROR)
call MPIF_IAlltoall(SB, SC, ST, RB, RC, RT, COMM, REQ,IERROR)
return
end

subroutine MPI_IAlltoallv(SB, SC, SD, ST, RB, RC, RD, RT, C, REQ,ERROR)
call MPIF_IAlltoallv(SB, SC, SD, ST, RB, RC, RD, RT, C, REQ,ERROR)
return
end

subroutine MPI_IReduce(SBUF, RBUF, CNT, D, OP, ROOT, COMM, REQ,IERROR)
call MPIF_IReduce(SBUF, RBUF, CNT, D, OP, ROOT, COMM, REQ,IERROR)
return
end

subroutine MPI_IAllreduce(SBUF, RBUF, CNT, D, OP, COMM, REQ,IERROR)
call MPIF_IAllreduce(SBUF, RBUF, CNT, D, OP, COMM, REQ,IERROR)
return
end

subroutine MPI_IReduce_scatter(SBUF, RBUF, RCNT, D, OP, C, REQ,IERROR)
call MPIF_IReduce_scatter(SBUF, RBUF, RCNT, D, OP, C, REQ,IERROR)
return
end

subroutine MPI_IScan(SBUF, RBUF, CNT, D, OP, COMM, REQ,IERROR)
call  MPIF_IScan(SBUF, RBUF, CNT, D, OP, COMM, REQ,IERROR)
return
end
