/* APPLE LOCAL file ARM 5683689 */

/* Verify that the correct builtin definition is generated by default
   when generating code for the ARM architecture.  */

/* { dg-do compile { target arm*-*-darwin* } } */
/* { dg-skip-if "Not valid with -mmacosx-version-min" { *-*-darwin* } { "-mmacosx-version-min=*" } { "" } } */

#if (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ != 20000)
#error TEST FAILS
#endif

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#error TEST FAILS
#endif

int main(void)
{
  return 0;
}
