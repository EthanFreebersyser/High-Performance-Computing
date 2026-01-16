      PROGRAM pi
      include 'mpif.h'
      INTEGER ierr
      INTEGER rank, size
      INTEGER src, dest
      INTEGER iter
      INTEGER :: count = 0
      REAL rand
      INTEGER i
      REAL x, y
      INTEGER tmpCount
      REAL PINum

      CALL MPI_INIT(ierr)
      CALL MPI_Comm_size(MPI_COMM_WORLD, size, ierr)
      CALL MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)

      CALL RANDOM_SEED()

      IF (rank .eq. 0) THEN
            READ(*,*) iter

            DO i = 1, size-1
                  Call MPI_SEND(iter, 1, MPI_INTEGER, i, 0, MPI_COMM_WORLD, ierr)
            END DO      
      ELSE
            CALL MPI_RECV(iter, 1, MPI_INTEGER, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE, ierr)
      END IF

      DO i = 1, iter
            CALL RANDOM_NUMBER(rand)
            x = rand
            CALL RANDOM_NUMBER(rand)
            y = rand

            IF (x*x + y*y .LT. 1) THEN
                  count = count + 1
            END IF 
      END DO

      IF (rank == 0) THEN
            DO i=1,size-1
                  CALL MPI_Recv(tmpCount, 1, MPI_INTEGER, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE, ierr)
                  count = count + tmpCount
            END DO

            PINum = 4.0 * count / (size * iter)

            PRINT *, "PI: =", PINum

      ELSE 
            CALL MPI_Send(count, 1, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, ierr)
      END IF

      CALL MPI_FINALIZE(ierr)
      
      END PROGRAM pi