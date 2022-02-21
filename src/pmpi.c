#include "mpi.h"

/* This is the root process */
#define ROOT 0

/* process information */
int numproc, rank, len;

int MPK_Init(int *argc, char ***argv)
{
    int value = PMPI_Init(argc, argv);
    /* get the number of procs in the comm */
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    /* get my rank in the comm */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

int MPK_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    return PMPI_Send(buf, count, datatype, dest, tag, comm);
}

int MPK_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    return PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
}

int MPK_Finalize(void)
{
    return PMPI_Finalize();
}