/* Verify that overloaded built-ins for vec_add with __int128
   inputs produce the right results.  */

/* { dg-do compile } */
/* { dg-require-effective-target powerpc_p8vector_ok } */
/* { dg-options "-maltivec -mvsx -mpower8-vector" } */
/* { dg-additional-options "-maix64" { target powerpc-ibm-aix* } } */

#include "altivec.h"

vector signed __int128
test1 (vector signed __int128 x, vector signed __int128 y)
{
  return vec_add (x, y);
}

vector unsigned __int128
test2 (vector unsigned __int128 x, vector unsigned __int128 y)
{
  return vec_add (x, y);
}

/* { dg-final { scan-assembler-times "vadduqm" 2 } } */
