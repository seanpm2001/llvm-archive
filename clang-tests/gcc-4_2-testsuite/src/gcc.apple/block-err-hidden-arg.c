/* APPLE LOCAL file radar 6404979 */
/* Error on use of _self. */
/* { dg-options "-mmacosx-version-min=10.5 -fblocks" { target *-*-darwin* } } */
/* { dg-do compile } */

void foo (void*);

int main(void)
{
	foo (^{ _self; });  /* { dg-error "use of undeclared identifier \\'_self\\'" } */
	return 0;
}

