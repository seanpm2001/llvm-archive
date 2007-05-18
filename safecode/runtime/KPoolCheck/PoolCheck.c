/*===- PoolCheck.cpp - Implementation of poolcheck runtime ----------------===*/
/*                                                                            */
/*                       The LLVM Compiler Infrastructure                     */
/*                                                                            */
/* This file was developed by the LLVM research group and is distributed      */
/* under the University of Illinois Open Source License. See LICENSE.TXT for  */
/* details.                                                                   */
/*                                                                            */
/*===----------------------------------------------------------------------===*/
/*                                                                            */
/* This file implements the poolcheck interface w/ metapools and opaque       */
/* pool ids.                                                                  */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#include "PoolCheck.h"
#include "PoolSystem.h"
#include "adl_splay.h"
#ifdef LLVA_KERNEL
#include <stdarg.h>
#endif
#define DEBUG(x) 

/* Flag whether we are ready to perform pool operations */
static int ready = 0;
/* Flag whether to do profiling */
/* profiling only works if this library is compiled to a .o file, not llvm */
static const int do_profile = 0;

/* Flag whether to support out of bounds pointer rewriting */
static const int use_oob = 0;

/* Flag whether to print error messages on bounds violations */
static const int do_fail = 0;

/* Statistic counters */
int stat_poolcheck=0;
int stat_poolcheckarray=0;
int stat_poolcheckarray_i=0;
int stat_boundscheck=0;
int stat_boundscheck_i=0;

extern void llva_load_lif (unsigned int enable);
extern unsigned int llva_save_lif (void);


static unsigned
disable_irqs ()
{
  unsigned int is_set;
  is_set = llva_save_lif ();
  llva_load_lif (0);
  return is_set;
}

static void
enable_irqs (int is_set)
{
  llva_load_lif (is_set);
}

#define PCLOCK() int pc_i = disable_irqs();
#define PCLOCK2() pc_i = disable_irqs();
#define PCUNLOCK() enable_irqs(pc_i);

#define maskaddr(_a) ((void*) ((unsigned)_a & ~(4096 - 1)))

static int isInCache(MetaPoolTy*  MP, void* addr) {
  addr = maskaddr(addr);
  if (!addr) return 0;
  if (MP->cache0 == addr)
    return 1;
  if (MP->cache1 == addr)
    return 2;
  if (MP->cache2 == addr)
    return 3;
  if (MP->cache3 == addr)
    return 4;
  return 0;
}

static void mtfCache(MetaPoolTy* MP, int ent) {
  void* z = MP->cache0;
  switch (ent) {
  case 2:
    MP->cache0 = MP->cache1;
    MP->cache1 = z;
    break;
  case 3:
    MP->cache0 = MP->cache1;
    MP->cache1 = MP->cache2;
    MP->cache2 = z;
    break;
  case 4:
    MP->cache0 = MP->cache1;
    MP->cache1 = MP->cache2;
    MP->cache2 = MP->cache3;
    MP->cache3 = z;
    break;
  default:
    break;
  }
  return;
}

static int insertCache(MetaPoolTy* MP, void* addr) {
  addr = maskaddr(addr);
  if (!addr) return 0;
  if (!MP->cache0) {
    MP->cache0 = addr;
    return 1;
  }
  else if (!MP->cache1) {
    MP->cache1 = addr;
    return 2;
  }
  else if (!MP->cache2) {
    MP->cache2 = addr;
    return 3;
  }
  else {
    MP->cache3 = addr;
    return 4;
  }
}

/*
 * Function: pchk_init()
 *
 * Description:
 *  Initialization function to be called when the memory allocator run-time
 *  intializes itself.
 *
 * Preconditions:
 *  1) The OS kernel is able to handle callbacks from the Execution Engine.
 */
void pchk_init(void) {

  /* initialize runtime */
  adl_splay_libinit(poolcheckmalloc);

  /*
   * Register all of the global variables in their respective meta pools.
   */
  poolcheckglobals();

  /*
   * Flag that we're ready to rumble!
   */
  ready = 1;
  return;
}

/* Register a slab */
void pchk_reg_slab(MetaPoolTy* MP, void* PoolID, void* addr, unsigned len) {
#if 0
  if (!MP) { poolcheckinfo("reg slab on null pool", (int)addr); return; }
#else
  if (!MP) { return; }
#endif
  PCLOCK();
  adl_splay_insert(&MP->Slabs, addr, len, PoolID);
  PCUNLOCK();
}

/* Remove a slab */
void pchk_drop_slab(MetaPoolTy* MP, void* PoolID, void* addr) {
  if (!MP) return;
  /* TODO: check that slab's tag is == PoolID */
  PCLOCK();
  adl_splay_delete(&MP->Slabs, addr);
  PCUNLOCK();
}

/* Register a non-pool allocated object */
void pchk_reg_obj(MetaPoolTy* MP, void* addr, unsigned len) {
#if 0
  if (!MP) { poolcheckinfo("reg obj on null pool", addr); return; }
#else
  if (!MP) { return; }
#endif
#if 0
  if (ready) poolcheckinfo2 ("pchk_reg_obj", addr, len);
#endif
  PCLOCK();
#if 0
  {
  void * S = addr;
  unsigned len, tag = 0;
  if ((ready) && (adl_splay_retrieve(&MP->Objs, &S, &len, &tag)))
    poolcheckinfo2 ("regobj: Object exists", __builtin_return_address(0), tag);
  }
#endif

  adl_splay_insert(&MP->Objs, addr, len, __builtin_return_address(0));
  PCUNLOCK();
}

/* Remove a non-pool allocated object */
void pchk_drop_obj(MetaPoolTy* MP, void* addr) {
  if (!MP) return;
  PCLOCK();
  adl_splay_delete(&MP->Objs, addr);
#if 0
  {
  void * S = addr;
  unsigned len, tag;
  if (adl_splay_retrieve(&MP->Objs, &S, &len, &tag))
    poolcheckinfo ("drop_obj: Failed to remove: 1", addr, tag);
  }
#endif
  PCUNLOCK();
}

/* Register a pool */
/* The MPLoc is the location the pool wishes to store the metapool tag for */
/* the pool PoolID is in at. */
/* MP is the actual metapool. */
void pchk_reg_pool(MetaPoolTy* MP, void* PoolID, void* MPLoc) {
  if(!MP) return;
  if(*(void**)MPLoc && *(void**)MPLoc != MP) {
    if(do_fail) poolcheckfail("reg_pool: Pool in 2 MP (inference bug a): ", (unsigned)*(void**)MPLoc, (void*)__builtin_return_address(0));
    if(do_fail) poolcheckfail("reg_pool: Pool in 2 MP (inference bug b): ", (unsigned) MP, (void*)__builtin_return_address(0));
    if(do_fail) poolcheckfail("reg_pool: Pool in 2 MP (inference bug c): ", (unsigned) PoolID, (void*)__builtin_return_address(0));
  }

  *(void**)MPLoc = (void*) MP;
}

/* A pool is deleted.  free it's resources (necessary for correctness of checks) */
void pchk_drop_pool(MetaPoolTy* MP, void* PoolID) {
  if(!MP) return;
  PCLOCK();
  adl_splay_delete_tag(&MP->Slabs, PoolID);
  PCUNLOCK();
}

/* check that addr exists in pool MP */
void poolcheck(MetaPoolTy* MP, void* addr) {
  if (!ready || !MP) return;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_poolcheck;
  PCLOCK();
  int t = adl_splay_find(&MP->Slabs, addr) || adl_splay_find(&MP->Objs, addr);
  PCUNLOCK();
  if (t)
    return;
  if(do_fail) poolcheckfail ("poolcheck failure: ", (unsigned)addr, (void*)__builtin_return_address(0));
}

/* check that src and dest are same obj or slab */
void poolcheckarray(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_poolcheckarray;
  /* try slabs first */
  void* S = src;
  void* D = dest;
  PCLOCK();
  adl_splay_retrieve(&MP->Slabs, &S, 0, 0);
  adl_splay_retrieve(&MP->Slabs, &D, 0, 0);
  PCUNLOCK();
  if (S == D)
    return;
  /* try objs */
  S = src;
  D = dest;
  PCLOCK2();
  adl_splay_retrieve(&MP->Objs, &S, 0, 0);
  adl_splay_retrieve(&MP->Objs, &D, 0, 0);
  PCUNLOCK();
  if (S == D)
    return;
  if(do_fail) poolcheckfail ("poolcheck failure: ", (unsigned)src, (void*)__builtin_return_address(0));
}

/* check that src and dest are same obj or slab */
/* if src and dest do not exist in the pool, pass */
void poolcheckarray_i(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_poolcheckarray_i;
  /* try slabs first */
  void* S = src;
  void* D = dest;
  PCLOCK();
  int fs = adl_splay_retrieve(&MP->Slabs, &S, 0, 0);
  int fd = adl_splay_retrieve(&MP->Slabs, &D, 0, 0);
  PCUNLOCK();
  if (S == D)
    return;
  if (fs || fd) { /*fail if we found one but not the other*/ 
    if(do_fail) poolcheckfail ("poolcheck failure: ", (unsigned)src, (void*)__builtin_return_address(0));
    return;
  }
  /* try objs */
  S = src;
  D = dest;
  PCLOCK2();
  fs = adl_splay_retrieve(&MP->Objs, &S, 0, 0);
  fd = adl_splay_retrieve(&MP->Objs, &D, 0, 0);
  PCUNLOCK();
  if (S == D)
    return;
  if (fs || fd) { /*fail if we found one but not the other*/
    if(do_fail) poolcheckfail ("poolcheck failure: ", (unsigned)src, (void*)__builtin_return_address(0));
    return;
  }
  return; /*default is to pass*/
}

const unsigned InvalidUpper = 4096;
const unsigned InvalidLower = 0x03;


/* if src is an out of object pointer, get the original value */
void* pchk_getActualValue(MetaPoolTy* MP, void* src) {
  if (!ready || !MP || !use_oob) return src;
  if ((unsigned)src <= InvalidLower) return src;
  void* tag = 0;
  /* outside rewrite zone */
  if ((unsigned)src & ~(InvalidUpper - 1)) return src;
  PCLOCK();
  if (adl_splay_retrieve(&MP->OOB, &src, 0, &tag)) {
    PCUNLOCK();
    return tag;
  }
  PCUNLOCK();
  if(do_fail) poolcheckfail("GetActualValue failure: ", (unsigned) src, (void*)__builtin_return_address(0));
  return tag;
}

/*
 * Function: getBounds()
 *
 * Description:
 *  Get the bounds associated with this object in the specified metapool.
 *
 * Return value:
 *  If the node is found in the pool, it returns the bounds relative to
 *  *src* (NOT the beginning of the object).
 *  If the node is not found in the pool, it returns 0x00000000.
 *  If the pool is not yet ready, it returns 0xffffffff
 */
struct node {
  void* left;
  void* right;
  char* key;
  char* end;
  void* tag;
};

static struct node not_found = {0, 0, 0, (char *)0x00000000, 0};
static struct node found =     {0, 0, 0, (char *)0xffffffff, 0};

void* getBounds(MetaPoolTy* MP, void* src) {
  if (!ready || !MP) return &found;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_boundscheck;
  /* try objs */
  void* S = src;
  unsigned len = 0;
  PCLOCK();
  int fs = adl_splay_retrieve(&MP->Objs, &S, &len, 0);
  PCUNLOCK();
  if (fs) {
    return (MP->Objs);
  }
  return &not_found;
}

/*
 * Function: getBounds_i()
 *
 * Description:
 *  Get the bounds associated with this object in the specified metapool.
 *
 * Return value:
 *  If the node is found in the pool, it returns the bounds.
 *  If the node is not found in the pool, it returns 0xffffffff.
 *  If the pool is not yet ready, it returns 0xffffffff
 */
void* getBounds_i(MetaPoolTy* MP, void* src) {
  if (!ready || !MP) return &found;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_boundscheck;
  //Try fail cache first
  PCLOCK();
#if 0
  int i = isInCache(MP, src);
  if (i) {
    mtfCache(MP, i);
    PCUNLOCK();
    return &found;
  }
#endif
  /* try objs */
  void* S = src;
  unsigned len = 0;
#if 0
  PCLOCK2();
#endif
  int fs = adl_splay_retrieve(&MP->Objs, &S, &len, 0);
  PCUNLOCK();
  if (fs) {
    return MP->Objs;
  }
  return &found;
}

/*
 * Function: boundscheck()
 *
 * Description:
 *  Perform a precise array bounds check on source and result.  If the result
 *  is out of range for the array, return 0x1 so that getactualvalue() will
 *  know that the pointer is bad and should not be dereferenced.
 */
void* pchk_bounds(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return dest;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_boundscheck;
  /* try objs */
  void* S = src;
  unsigned len = 0;
  PCLOCK();
  int fs = adl_splay_retrieve(&MP->Objs, &S, &len, 0);
  PCUNLOCK();
  if ((fs) && S <= dest && ((char*)S + len) > (char*)dest )
    return dest;
  else if (fs) {
    if (!use_oob) {
      if(do_fail) poolcheckfail ("boundscheck failure 1", (unsigned)src, (void*)__builtin_return_address(0));
      return dest;
    }
    PCLOCK2();
    if (MP->invalidptr == 0) MP->invalidptr = (unsigned char*)InvalidLower;
    ++MP->invalidptr;
    void* P = MP->invalidptr;
    PCUNLOCK();
    if ((unsigned)P & ~(InvalidUpper - 1)) {
      if(do_fail) poolcheckfail("poolcheck failure: out of rewrite ptrs", 0, (void*)__builtin_return_address(0));
      return dest;
    }
    if(do_fail) poolcheckinfo2("Returning oob pointer of ", (int)P, __builtin_return_address(0));
    PCLOCK2();
    adl_splay_insert(&MP->OOB, P, 1, dest);
    PCUNLOCK()
    return P;
  }

  /*
   * The node is not found or is not within bounds; fail!
   */
  if(do_fail) poolcheckfail ("boundscheck failure 2", (unsigned)src, (void*)__builtin_return_address(0));
  return dest;
}

/*
 * Function: uiboundscheck()
 *
 * Description:
 *  Perform a precise array bounds check on source and result.  If the result
 *  is out of range for the array, return a sentinel so that getactualvalue()
 *  will know that the pointer is bad and should not be dereferenced.
 *
 *  This version differs from boundscheck() in that it does not generate a
 *  poolcheck failure if the source node cannot be found within the MetaPool.
 */
void* pchk_bounds_i(MetaPoolTy* MP, void* src, void* dest) {
  if (!ready || !MP) return dest;
  if (do_profile) pchk_profile(__builtin_return_address(0));
  ++stat_boundscheck_i;
  /* try fail cache */
  PCLOCK();
  int i = isInCache(MP, src);
  if (i) {
    mtfCache(MP, i);
    PCUNLOCK();
    return dest;
  }
  /* try objs */
  void* S = src;
  unsigned len = 0;
  unsigned int tag;
  int fs = adl_splay_retrieve(&MP->Objs, &S, &len, &tag);
  if ((fs) && (S <= dest) && (((unsigned char*)S + len) > (unsigned char*)dest)) {
    PCUNLOCK();
    return dest;
  }
  else if (fs) {
    if (!use_oob) {
      PCUNLOCK();
#if 0
      if(do_fail) poolcheckfail ("uiboundscheck failure 1", (unsigned)S, len);
      if(do_fail) poolcheckfail ("uiboundscheck failure 2", (unsigned)S, tag);
#endif
      if (do_fail) poolcheckfail ("uiboundscheck failure 3", (unsigned)dest, (void*)__builtin_return_address(0));
      return dest;
    }
     if (MP->invalidptr == 0) MP->invalidptr = (unsigned char*)0x03;
    ++MP->invalidptr;
    void* P = MP->invalidptr;
    if ((unsigned)P & ~(InvalidUpper - 1)) {
      PCUNLOCK();
      if(do_fail) poolcheckfail("poolcheck failure: out of rewrite ptrs", 0, (void*)__builtin_return_address(0));
      return dest;
    }
    adl_splay_insert(&MP->OOB, P, 1, dest);
    PCUNLOCK();
    return P;
  }

  /*
   * The node is not found or is not within bounds; pass!
   */
  int nn = insertCache(MP, src);
  mtfCache(MP, nn);
  PCUNLOCK();
  return dest;
}


/* Profiling support */

struct pr {
  void* pc;
  unsigned count;
};

#define profile_count 4096

static struct pr profile_data[profile_count]; /* 2 pages */

static void profile_print();

void llva_profile_print ()
{
  profile_print();
}

int profile_pause = 0;

static void pchk_resize() {
  /* lacking a better time, print out the stats when resizing */
  profile_print();

  /* walk the profile_data and halve everything
     anything with a zero count then gets removed */
  int x;
  for (x = 0; x < profile_count; ++x) {
    profile_data[x].count /= 2;
    if (!profile_data[x].count)
      profile_data[x].pc = 0;
  }
}

void pchk_profile(void* pc) {
  if (profile_pause) return;

  int last_empty = -1;
  int x;
  for (x = 0; x < profile_count; ++x) {
    if (profile_data[x].pc == pc) {
      ++profile_data[x].count;
      /* prevent overflow */
      if (profile_data[x].count > 10000)
        pchk_resize();
      return;
    } else if (profile_data[x].pc == 0)
      last_empty = x;
  }
  if (last_empty != -1) {
    profile_data[x].pc = pc;
    profile_data[x].count = 1;
    return;
  }
  /* full? shrink everything and try again */
  pchk_resize();
  pchk_profile(pc);
}

/* print the top 10 sites */
static void profile_print() {
  profile_pause = 1;
  int x;
  for (x = 0; x < profile_count; ++x) {
    if (profile_data[x].count > 3000)
      poolcheckinfo2("LLVA: profile ", (int)profile_data[x].pc, 
                     (int) profile_data[x].count);
  }
  profile_pause = 0;
}
