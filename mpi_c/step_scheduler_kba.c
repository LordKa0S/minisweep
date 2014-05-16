/*---------------------------------------------------------------------------*/
/*!
 * \file   step_scheduler_kba_c.h
 * \author Wayne Joubert
 * \date   Tue Jan 28 16:37:41 EST 2014
 * \brief  Definitions for managing sweep step schedule.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
/*---------------------------------------------------------------------------*/

#include "env.h"
#include "definitions.h"
#include "step_scheduler_kba.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================*/
/*---Pseudo-constructor for Step_Scheduler struct---*/

void Step_Scheduler_ctor( Step_Scheduler* step_scheduler,
                          int             nblock_z,
                          int             nblock_octant,
                          Env*            env )
{
  Insist( nblock_z > 0 && "Invalid z blocking factor supplied." );
  step_scheduler->nblock_z__          = nblock_z;
  step_scheduler->nproc_x__           = Env_nproc_x( env );
  step_scheduler->nproc_y__           = Env_nproc_y( env );
  step_scheduler->nblock_octant__     = nblock_octant;
  step_scheduler->noctant_per_block__ = NOCTANT / nblock_octant;
}

/*===========================================================================*/
/*---Pseudo-destructor for Step_Scheduler struct---*/

void Step_Scheduler_dtor( Step_Scheduler* step_scheduler )
{
}

/*===========================================================================*/
/*---Accessor: blocks along z axis---*/

int Step_Scheduler_nblock_z( const Step_Scheduler* step_scheduler )
{
  return step_scheduler->nblock_z__;
}

/*===========================================================================*/
/*---Number of block steps executed for a single octant in isolation---*/

int Step_Scheduler_nblock( const Step_Scheduler* step_scheduler )
{
  return step_scheduler->nblock_z__;
}

/*===========================================================================*/
/*---Number of octants per octant block---*/

int Step_Scheduler_noctant_per_block( const Step_Scheduler* step_scheduler )
{
  return NOCTANT / step_scheduler->nblock_octant__;
}

/*===========================================================================*/
/*---Number of kba parallel steps---*/

int Step_Scheduler_nstep( const Step_Scheduler* step_scheduler )
{
  int result;

  switch( step_scheduler->nblock_octant__ )
  {
    case 8:
      result = 8 * Step_Scheduler_nblock( step_scheduler )
                                       + 2 * ( step_scheduler->nproc_x__ - 1 )
                                       + 3 * ( step_scheduler->nproc_y__ - 1 );
      break;

    case 4:
      result = 4 * Step_Scheduler_nblock( step_scheduler )
                                       + 1 * ( step_scheduler->nproc_x__ - 1 )
                                       + 2 * ( step_scheduler->nproc_y__ - 1 );
      break;

    case 2:
      result = 2 * Step_Scheduler_nblock( step_scheduler )
                                       + 1 * ( step_scheduler->nproc_x__ - 1 )
                                       + 1 * ( step_scheduler->nproc_y__ - 1 );
      break;

    case 1:
      result = 1 * Step_Scheduler_nblock( step_scheduler )
                                       + 1 * ( step_scheduler->nproc_x__ - 1 )
                                       + 1 * ( step_scheduler->nproc_y__ - 1 );
      break;

    default:
      Assert( Bool_false );
      break;
  }

  return result;
}

/*===========================================================================*/
/*---Get information describing a sweep step---*/

Step_Info Step_Scheduler_step_info( const Step_Scheduler* step_scheduler,  
                                    const int             step,
                                    const int             octant_in_block,
                                    const int             proc_x,
                                    const int             proc_y )
{
  Assert( octant_in_block>=0 &&
          octant_in_block * step_scheduler->nblock_octant__ < NOCTANT );

  const int nblock_octant     = step_scheduler->nblock_octant__;
  const int nproc_x           = step_scheduler->nproc_x__;
  const int nproc_y           = step_scheduler->nproc_y__;
  const int nblock            = Step_Scheduler_nblock( step_scheduler );
  const int nstep             = Step_Scheduler_nstep( step_scheduler );
  const int noctant_per_block = step_scheduler->noctant_per_block__;

  int octant_key    = 0;
  int wave          = 0;
  int step_base     = 0;
  int block         = 0;
  int octant        = 0;
  int dir_x         = 0;
  int dir_y         = 0;
  int dir_z         = 0;
  int start_x       = 0;
  int start_y       = 0;
  int start_z       = 0;
  int folded_octant = 0;
  int folded_block  = 0;

  Step_Info step_info;

  const int octant_selector[NOCTANT] = { 0, 4, 2, 6, 3, 7, 1, 5 };

  const Bool_t is_folded_x = noctant_per_block >= 2;
  const Bool_t is_folded_y = noctant_per_block >= 4;
  const Bool_t is_folded_z = noctant_per_block >= 8;

  const int folded_proc_x = ( is_folded_x && ( octant_in_block & (1<<0) ) )
                          ?  ( nproc_x - 1 - proc_x )
                          :                  proc_x;

  const int folded_proc_y = ( is_folded_y && ( octant_in_block & (1<<1) ) )
                          ?  ( nproc_y - 1 - proc_y )
                          :                  proc_y;

  /*===========================================================================
    For a given step and octant_in_block, the following computes the
    octant block (i.e., octant step), from which the octant can be
    computed, and the wavefront number, starting from the relevant begin
    corner of the selected octant.
    For the nblock_octant==8 case, the 8 octants are processed in sequence,
    in the order xyz = +++, ++-, -++, -+-, --+, ---, +-+, +--.
    This order is chosen to "pack" the wavefronts to minimize
    the KBA wavefront startup latency.
    For nblock_octant=k for some smaller k, this sequence is divided into
    subsequences of length k, and each subsequence defines the schedule
    for a given octant_in_block.
    The code below is essentially a search into the first subsequence
    to determine where the requested step is located.  Locations in
    the other subsequences can be derived from this.
    NOTE: the following does not address possibility that for a single
    step, two or more octants could update the same block.
  ===========================================================================*/

  if ( Bool_true )
  {
    wave = step - ( step_base );
    octant_key = 0;
  }
  step_base += nblock;
  if ( step >= ( step_base + folded_proc_x
                           + folded_proc_y ) && ! is_folded_z )
  {
    wave = step - ( step_base );
    octant_key = 1;
  }
  step_base += nblock;
  if ( step >= ( step_base +            folded_proc_x
                           +            folded_proc_y ) && ! is_folded_y )
  {
    wave = step - ( step_base + (nproc_y-1) );
    octant_key = 2;
  }
  step_base += nblock + (nproc_y-1);
  if ( step >= ( step_base + (nproc_y-1-folded_proc_y)
                           +            folded_proc_x ) && ! is_folded_y )
  {
    wave = step - ( step_base );
    octant_key = 3;
  }
  step_base += nblock;
  if ( step >= ( step_base + (nproc_y-1-folded_proc_y)
                           +            folded_proc_x ) && ! is_folded_x )
  {
    wave = step - ( step_base + (nproc_x-1) );
    octant_key = 4;
  }
  step_base += nblock + (nproc_x-1);
  if ( step >= ( step_base + (nproc_y-1-folded_proc_y)
                           + (nproc_x-1-folded_proc_x) ) && ! is_folded_x )
  {
    wave = step - ( step_base );
    octant_key = 5;
  }
  step_base += nblock;
  if ( step >= ( step_base + (nproc_y-1-folded_proc_y)
                           + (nproc_x-1-folded_proc_x) ) && ! is_folded_x )
  {
    wave = step - ( step_base + (nproc_y-1) );
    octant_key = 6;
  }
  step_base += nblock + (nproc_y-1);
  if ( step >= ( step_base +            folded_proc_y
                           + (nproc_x-1-folded_proc_x) ) && ! is_folded_x )
  {
    wave = step - ( step_base );
    octant_key = 7;
  }

  folded_octant = octant_selector[ octant_key ];

  octant = folded_octant + octant_in_block;

  /*---Next convert the wavefront number to a block number based on
       location in the domain.  Use the equation that defines the plane.
  ---*/

  dir_x  = Dir_x( folded_octant );
  dir_y  = Dir_y( folded_octant );
  dir_z  = Dir_z( folded_octant );

  /*---Get coordinates of the starting corner block of the wavefront---*/
  start_x = dir_x==Dir_up() ? 0 : ( nproc_x - 1 );
  start_y = dir_y==Dir_up() ? 0 : ( nproc_y - 1 );
  start_z = dir_z==Dir_up() ? 0 : ( nblock  - 1 );

  /*---Get coordinate of block on this processor to be processed---*/
  folded_block = ( wave - ( start_x + folded_proc_x * dir_x )
                        - ( start_y + folded_proc_y * dir_y )
                        - ( start_z ) ) / dir_z;

  block = ( is_folded_z && ( octant_in_block & (1<<2) ) )
                          ? ( nblock - 1 - folded_block )
                          : folded_block;

  /*---Now determine whether the block calculation is active based on whether
       the block in question falls within the physical domain.
  ---*/

  step_info.is_active = block  >= 0 && block  < nblock &&
                        step   >= 0 && step   < nstep &&
                        proc_x >= 0 && proc_x < nproc_x &&
                        proc_y >= 0 && proc_y < nproc_y;

  /*---Set remaining values---*/

  step_info.block_z = step_info.is_active ? block  : -1;
  step_info.octant  = step_info.is_active ? octant : -1;

  return step_info;
}

/*===========================================================================*/
/*---Determine whether to send a face computed at step, used at step+1---*/

Bool_t Step_Scheduler_must_do_send(
  Step_Scheduler* step_scheduler,
  int             step,
  int             axis,
  int             dir_ind,
  int             octant_in_block,
  Env*            env )
{
  const int proc_x = Env_proc_x_this( env );
  const int proc_y = Env_proc_y_this( env );

  const Bool_t axis_x = axis==0;
  const Bool_t axis_y = axis==1;

  const int dir = dir_ind==0 ? Dir_up() : Dir_dn();
  const int inc_x = axis_x ? Dir_inc( dir ) : 0;
  const int inc_y = axis_y ? Dir_inc( dir ) : 0;

  /*---Get step info for processors involved in communication---*/

  const Step_Info step_info_send_source_step = Step_Scheduler_step_info(
    step_scheduler, step,   octant_in_block, proc_x,       proc_y       );

  const Step_Info step_info_send_target_step = Step_Scheduler_step_info(
    step_scheduler, step+1, octant_in_block, proc_x+inc_x, proc_y+inc_y );

  /*---Determine whether to communicate---*/

  Bool_t const do_send = step_info_send_source_step.is_active
                      && step_info_send_target_step.is_active
                      && step_info_send_source_step.octant ==
                         step_info_send_target_step.octant
                      && step_info_send_source_step.block_z ==
                         step_info_send_target_step.block_z
                      && ( axis_x ?
                           Dir_x( step_info_send_target_step.octant ) :
                           Dir_y( step_info_send_target_step.octant ) ) == dir;

  return do_send;
}

/*===========================================================================*/
/*---Determine whether to recv a face computed at step, used at step+1---*/

Bool_t Step_Scheduler_must_do_recv(
  Step_Scheduler* step_scheduler,
  int             step,
  int             axis,
  int             dir_ind,
  int             octant_in_block,
  Env*            env )
{
  const int proc_x = Env_proc_x_this( env );
  const int proc_y = Env_proc_y_this( env );

  const Bool_t axis_x = axis==0;
  const Bool_t axis_y = axis==1;

  const int dir = dir_ind==0 ? Dir_up() : Dir_dn();
  const int inc_x = axis_x ? Dir_inc( dir ) : 0;
  const int inc_y = axis_y ? Dir_inc( dir ) : 0;

  /*---Get step info for processors involved in communication---*/

  const Step_Info step_info_recv_source_step = Step_Scheduler_step_info(
    step_scheduler, step,   octant_in_block, proc_x-inc_x, proc_y-inc_y );

  const Step_Info step_info_recv_target_step = Step_Scheduler_step_info(
    step_scheduler, step+1, octant_in_block, proc_x,       proc_y       );

  /*---Determine whether to communicate---*/

  Bool_t const do_recv = step_info_recv_source_step.is_active
                      && step_info_recv_target_step.is_active
                      && step_info_recv_source_step.octant ==
                         step_info_recv_target_step.octant
                      && step_info_recv_source_step.block_z ==
                         step_info_recv_target_step.block_z
                      && ( axis_x ?
                           Dir_x( step_info_recv_target_step.octant ) :
                           Dir_y( step_info_recv_target_step.octant ) ) == dir;

  return do_recv;
}

/*===========================================================================*/

#ifdef __cplusplus
} /*---extern "C"---*/
#endif

/*---------------------------------------------------------------------------*/
