! Copyright (C)Télécom SudParis
! See COPYING in top-level directory.

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
