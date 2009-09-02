/* PR tree-optimization/21563
   Make sure VRP folds the second "if" statement.  */

/* { dg-do compile } */
/* { dg-options "-O2 -fno-tree-dominator-opts -fdump-tree-vrp1-details" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */

int
foo (int a)
{
  if (a > 1)
    if (a == 0)
      return 1;
  return 0;
}

/* { dg-final { scan-tree-dump-times "Folding predicate" 1 "vrp1"} } */
/* { dg-final { cleanup-tree-dump "vrp1" } } */
