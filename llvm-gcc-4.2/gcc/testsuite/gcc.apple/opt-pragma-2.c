#include <stdio.h>
/* Radar 3124235 */
/* { dg-do compile { target "powerpc*-*-darwin*" } } */
/* { dg-options "-O3" } */
void f4(int);
#pragma optimization_level 0
void f1(int x) {
  printf("%d\n", x);
}
#pragma GCC optimize_for_size on
#pragma GCC optimization_level 0
void f4(int  x) {
  printf("%d\n", x);
}
#pragma GCC optimization_level 0
void f5(int x) {
#pragma GCC optimization_level 2
  printf("%d\n", x);
}
#pragma GCC optimization_level reset
void f6(int x) {
  printf("%d\n", x);
}
/* Make sure sibling call optimization is not applied. */
/* { dg-final { scan-assembler-times "b L_printf" 0 } } */
/* { dg-final { scan-assembler-times "bl L_printf" 4 } } */
