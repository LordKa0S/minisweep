#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

/* This is the root process */
#define ROOT 0

/* process information */
int numproc, rank;
int *sends;
int *matrix = NULL;

int MPI_Init(int *argc, char ***argv)
{
    int value = PMPI_Init(argc, argv);
    /* get the number of procs in the comm */
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    /* get my rank in the comm */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == ROOT)
    {
        matrix = malloc(numproc * numproc * sizeof(int));
    }
    sends = (int *)malloc(numproc * sizeof(int));
    memset(sends, 0, numproc * sizeof(int));
    return value;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    *(sends + dest) += 1;
    return PMPI_Send(buf, count, datatype, dest, tag, comm);
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
    *(sends + dest) += 1;
    return PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
}

int MPI_Finalize(void)
{
    MPI_Gather(sends, numproc, MPI_INTEGER, matrix, numproc, MPI_INTEGER, ROOT, MPI_COMM_WORLD);
    if (rank == ROOT)
    {
        FILE *fp = fopen("matrix.data", "w");
        int proc;
        for (proc = 0; proc < numproc; proc++)
        {
            fprintf(fp, "%d ", proc);
            int dest;
            for (dest = 0; dest < numproc; dest++)
            {
                fprintf(fp, "%d ", *(matrix + proc + dest));
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    return PMPI_Finalize();
}