/*---------------------------------------------------------------------------*/
/*!
 * \file   sweeper_kba.h
 * \author Wayne Joubert
 * \date   Tue Jan 28 16:37:41 EST 2014
 * \brief  Declarations for performing a sweep, kba version.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
/*---------------------------------------------------------------------------*/

#ifndef _serial_c__sweeper_kba_h_
#define _serial_c__sweeper_kba_h_

#include "env.h"
#include "definitions.h"
#include "dimensions.h"
#include "arguments.h"
#include "pointer.h"
#include "quantities.h"
#include "step_scheduler_kba.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================*/
/*---Set up enums---*/

#ifdef USE_OPENMP_THREADS
enum{ IS_USING_OPENMP_THREADS = 1 };
#else
enum{ IS_USING_OPENMP_THREADS = 0 };
#endif

#ifdef USE_OPENMP_VO_ATOMIC
enum{ IS_USING_OPENMP_VO_ATOMIC = 1 };
#else
enum{ IS_USING_OPENMP_VO_ATOMIC = 0 };
#endif

/*===========================================================================*/
/*---Struct with pointers etc. used to perform sweep---*/

typedef struct
{
  Pointer          facexy0;

  Pointer          facexz0;
  Pointer          facexz1;
  Pointer          facexz2;

  Pointer          faceyz0;
  Pointer          faceyz1;
  Pointer          faceyz2;

  Pointer*         facesxy[1];
  Pointer*         facesxz[NDIM];
  Pointer*         facesyz[NDIM];

  P* __restrict__  v_local;

  Dimensions       dims;
  Dimensions       dims_b;
  Dimensions       dims_g;

  int              nblock_z;
  int              nblock_octant;
  int              nthread_e;
  int              nthread_octant;
  int              noctant_per_block;
  int              nsemiblock;

  Request_t        request_send_xz[NOCTANT];
  Request_t        request_send_yz[NOCTANT];
  Request_t        request_recv_xz[NOCTANT];
  Request_t        request_recv_yz[NOCTANT];

  Step_Scheduler   step_scheduler;
} Sweeper;

/*===========================================================================*/
/*---Pseudo-constructor for Sweeper struct---*/

void Sweeper_ctor( Sweeper*          sweeper,
                   Dimensions        dims,
                   const Quantities* quan,
                   Env*              env,
                   Arguments*        args );

/*===========================================================================*/
/*---Pseudo-destructor for Sweeper struct---*/

void Sweeper_dtor( Sweeper* sweeper );

/*===========================================================================*/
/*---Number of octants in an octant block---*/

static int Sweeper_noctant_per_block( const Sweeper* sweeper )
{
  return sweeper->noctant_per_block;
}

/*===========================================================================*/
/*---Is face communication done asynchronously---*/

static int Sweeper_is_face_comm_async()
{
  return Bool_true;
}

/*===========================================================================*/
/*---Determine whether to send a face now---*/

Bool_t Sweeper_must_do_send__(
  Sweeper*           sweeper,
  int                step,
  int                axis,
  int                dir_ind,
  int                octant_in_block,
  Env*               env );

/*===========================================================================*/
/*---Determine whether to receive a face now---*/

Bool_t Sweeper_must_do_recv__(
  Sweeper*           sweeper,
  int                step,
  int                axis,
  int                dir_ind,
  int                octant_in_block,
  Env*               env );

/*===========================================================================*/
/*---Communicate faces---*/

void Sweeper_communicate_faces__(
  Sweeper*         sweeper,
  int              step,
  Env*             env );

/*---------------------------------------------------------------------------*/

void Sweeper_send_faces_start__(
  Sweeper*           sweeper,
  int                step,
  Env*               env );

/*---------------------------------------------------------------------------*/

void Sweeper_send_faces_end__(
  Sweeper*           sweeper,
  int                step,
  Env*               env );

/*---------------------------------------------------------------------------*/

void Sweeper_recv_faces_start__(
  Sweeper*           sweeper,
  int                step,
  Env*               env );

/*---------------------------------------------------------------------------*/

void Sweeper_recv_faces_end__(
  Sweeper*           sweeper,
  int                step,
  Env*               env );

/*===========================================================================*/
/*---Apply boundary condition: xy face---*/

static void Sweeper_set_boundary_xy(
  const Sweeper*        sweeper,
  P* const __restrict__ facexy,
  const Quantities*     quan,
  int                   octant,
  int                   octant_in_block, 
  const int             ixmin_b,
  const int             ixmax_b,
  const int             iymin_b,
  const int             iymax_b,
  Env*                  env );

/*===========================================================================*/
/*---Apply boundary condition: xz face---*/

static void Sweeper_set_boundary_xz(
  const Sweeper*        sweeper,
  P* const __restrict__ facexz,
  const Quantities*     quan,
  int                   block_z,
  int                   octant,
  int                   octant_in_block, 
  const int             ixmin_b,
  const int             ixmax_b,
  const int             izmin_b,
  const int             izmax_b,
  Env*                  env );

/*===========================================================================*/
/*---Apply boundary condition: yz face---*/

static void Sweeper_set_boundary_yz(
  const Sweeper*        sweeper,
  P* const __restrict__ faceyz,
  const Quantities*     quan,
  int                   block_z,
  int                   octant,
  int                   octant_in_block,
  const int             iymin_b,
  const int             iymax_b,
  const int             izmin_b,
  const int             izmax_b,
  Env*                  env );

/*===========================================================================*/
/*---Selectors for faces---*/

/*---The xz and yz face arrays form a circular buffer of length three.
     Three are needed because at any step there may be a send, a receive, and a
     block-sweep-compute in-flight.
---*/

static Pointer* Sweeper_facexy__( Sweeper* sweeper, int step )
{
  assert( sweeper != NULL );
  assert( step >= 0 );
  return sweeper->facesxy[0];
}

/*---------------------------------------------------------------------------*/

static Pointer* Sweeper_facexz__( Sweeper* sweeper, int step )
{
  assert( sweeper != NULL );
  assert( step >= 0 );
  return Sweeper_is_face_comm_async() ? sweeper->facesxz[(step+3)%3]
                                      : sweeper->facesxz[0];
}

/*---------------------------------------------------------------------------*/

static Pointer* Sweeper_faceyz__( Sweeper* sweeper, int step )
{
  assert( sweeper != NULL );
  assert( step >= 0 );
  return Sweeper_is_face_comm_async() ? sweeper->facesyz[(step+3)%3]
                                      : sweeper->facesyz[0];
}

/*===========================================================================*/
/*---Select which part of v_local to use for current thread---*/

static inline P* __restrict__ Sweeper_v_local_this__( Sweeper* sweeper,
                                                      int      thread )
{
  return sweeper->v_local + sweeper->dims_b.na * NU * thread;
}

/*===========================================================================*/
/*---Thread indexers---*/

static inline int Sweeper_thread_e( const Sweeper* sweeper,
                                    Env*           env )
{
  assert( sweeper->nthread_e * sweeper->nthread_octant ==1 ||
          Env_omp_in_parallel( env ) );
  return Env_omp_thread( env ) % sweeper->nthread_e;
}

/*---------------------------------------------------------------------------*/

static inline int Sweeper_thread_octant( const Sweeper* sweeper,
                                         Env*           env )
{
  assert( sweeper->nthread_e * sweeper->nthread_octant ==1 ||
         Env_omp_in_parallel( env ) );
  return Env_omp_thread( env ) / sweeper->nthread_e;
}

/*===========================================================================*/
/*---Perform a sweep for a semiblock---*/

void Sweeper_sweep_semiblock(
  Sweeper*               sweeper,
  P* __restrict__        vo,
  const P* __restrict__  vi,
  P* __restrict__        facexy,
  P* __restrict__        facexz,
  P* __restrict__        faceyz,
  const P* __restrict__  a_from_m,
  const P* __restrict__  m_from_a,
  const Quantities*      quan,
  Env*                   env,
  const Step_Info        step_info,
  const int              octant_in_block,
  const int              ixmin,
  const int              ixmax,
  const int              iymin,
  const int              iymax,
  const int              izmin,
  const int              izmax );

/*===========================================================================*/
/*---Perform a sweep for a block---*/

void Sweeper_sweep_block(
  Sweeper*               sweeper,
  P* __restrict__        vo,
  const P* __restrict__  vi,
  P* __restrict__        facexy,
  P* __restrict__        facexz,
  P* __restrict__        faceyz,
  const P* __restrict__  a_from_m,
  const P* __restrict__  m_from_a,
  int                    step,
  const Quantities*      quan,
  Env*                   env );

/*===========================================================================*/
/*---Perform a sweep---*/

void Sweeper_sweep(
  Sweeper*               sweeper,
  P* __restrict__        vo,
  const P* __restrict__  vi,
  const Quantities*      quan,
  Env*                   env );

/*===========================================================================*/

#ifdef __cplusplus
} /*---extern "C"---*/
#endif

#endif /*---_sweeper_kba_h_---*/

/*---------------------------------------------------------------------------*/
