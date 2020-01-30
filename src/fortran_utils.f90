! Copyright (C)Télécom SudParis
! See COPYING in top-level directory.

integer function ezt_mpi_is_in_place( i )
  include 'mpif.h'
  integer i

  ezt_mpi_is_in_place = 0

  if( loc(i) .eq. loc(mpi_in_place) ) ezt_mpi_is_in_place = 1

  return
end function ezt_mpi_is_in_place
