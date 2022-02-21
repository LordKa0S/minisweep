#include "mpi.h"

int MPK_Init(int *argc, char ***argv){
    return PMPI_Init(argc, argv);
}

int MPK_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    return PMPI_Send(buf, count, datatype, dest, tag, comm);
}

int MPK_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request){
    return PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
}

int MPK_Finalize(void){
    return PMPI_Finalize();
}