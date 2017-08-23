#!/bin/bash -l
#------------------------------------------------------------------------------

module load cuda

if [ "$COMPILER" != "" -a "$COMPILER" != "nvcc" ] ; then
  COMPILER_FLAG="-ccbin;${COMPILER};"
else
  COMPILER_FLAG=""
  COMPILER_FLAGS_HOST="-Xcompiler;-fstrict-aliasing;-Xcompiler;-fargument-noalias-global;-Xcompiler;-O3;-Xcompiler;-fomit-frame-pointer;-Xcompiler;-funroll-loops;-Xcompiler;-finline-limit=100000000;"
fi

# CLEANUP
rm -rf CMakeCache.txt
rm -rf CMakeFiles

# SOURCE AND INSTALL
if [ "$SOURCE" = "" ] ; then
  SOURCE=../minisweep
fi
if [ "$INSTALL" = "" ] ; then
  INSTALL=../install
fi

if [ "$BUILD" = "" ] ; then
  BUILD=Debug
  #BUILD=Release
fi

if [ "$NM_VALUE" = "" ] ; then
  NM_VALUE=4
fi

if [ "$SWEEPER_TYPE" = "" ] ; then
  SWEEPER_TYPE=SWEEPER_KBA
fi

if [ "$BUILD" = "Release" ] ; then
  DEBUG_FLAG=";-DNDEBUG"
else
  DEBUG_FLAG=""
fi

#------------------------------------------------------------------------------

MPI_INCLUDE_DIR=$OMPI_DIR/include
MPI_LIB=$OMPI_DIR/lib/libmpi_ibm.so

cmake \
  -DCMAKE_BUILD_TYPE:STRING="$BUILD" \
  -DCMAKE_INSTALL_PREFIX:PATH="$INSTALL" \
 \
  -DCMAKE_C_COMPILER:STRING="gcc" \
  -DCMAKE_C_FLAGS:STRING="-DNM_VALUE=$NM_VALUE -D$SWEEPER_TYPE -I$MPI_INCLUDE_DIR" \
 \
  -DUSE_MPI:BOOL=ON \
  -DMPI_C_INCLUDE_PATH:STRING=$MPI_INCLUDE_DIR \
  -DMPI_C_LIBRARIES:STRING=$MPI_LIB \
 \
  -DUSE_CUDA:BOOL=ON \
  -DCUDA_NVCC_FLAGS:STRING="${COMPILER_FLAG}${COMPILER_FLAGS_HOST}${DEBUG_FLAG};-I$MPICH_DIR/include;-gencode;arch=compute_60,code=sm_60;-O3;-use_fast_math;-DNDEBUG;--maxrregcount;128;-Xptxas=-v$DEBUG_FLAG" \
  -DCUDA_HOST_COMPILER:STRING=/usr/bin/gcc \
  -DCUDA_PROPAGATE_HOST_FLAGS:BOOL=ON \
 \
  -DCMAKE_EXE_LINKER_FLAGS:STRING=$MPI_LIB \
 \
  $SOURCE

#------------------------------------------------------------------------------
