/* APPLE LOCAL file v7 merge */
/* Test the `vqRshrn_nu32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vqRshrn_nu32 (void)
{
  uint16x4_t out_uint16x4_t;
  uint32x4_t arg0_uint32x4_t;

  out_uint16x4_t = vqrshrn_n_u32 (arg0_uint32x4_t, 1);
}

/* { dg-final { scan-assembler "vqrshrn\.u32\[ 	\]+\[dD\]\[0-9\]+, \[qQ\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
