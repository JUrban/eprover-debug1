/* Regression test for JKISSRand/JKISSSeed (BASICS/clb_simple_stuff.c).
 *
 * Before the fix, JKISSRand(state) ignored its state argument and
 * updated/read the file-scope statics xstate/ystate/zstate/cstate
 * instead of state->xstate etc. Consequences:
 *  - JKISSSeed(state, ...) had no effect on later JKISSRand(state)
 *    calls (the Random heuristic's seed parameter was silently
 *    ignored);
 *  - every RandStateCell produced the same (shared global) sequence.
 */
#include <clb_simple_stuff.h>
#include <stdio.h>
#include <string.h>

/* Independent reference implementation of the KISS step. */
static unsigned ref_kiss(unsigned *x, unsigned *y, unsigned *z, unsigned *c)
{
   unsigned long long t;
   *x = 314527869u * *x + 1234567u;
   *y ^= *y << 5;
   *y ^= *y >> 7;
   *y ^= *y << 22;
   t = 4294584393ULL * *z + *c;
   *c = (unsigned) (t >> 32);
   *z = (unsigned) t;
   return *x + *y + *z;
}

int main(void)
{
   int fails = 0;

   /* Test 1: a seeded state must follow the KISS algorithm from its
      seeded values. */
   {
      RandStateCell s;
      unsigned x = 684291357, y = 123459876, z = 918273645, c = 129834675;
      memset(&s, 0, sizeof(s));
      s.xstate = x; s.ystate = y; s.zstate = z; s.cstate = c;

      int bad = 0;
      for(int i = 0; i < 20; i++)
      {
         unsigned got = JKISSRand(&s);
         unsigned want = ref_kiss(&x, &y, &z, &c);
         if(got != want) { bad++; break; }
      }
      if(bad)
      {
         printf("FAIL: seeded state does not follow the seeded stream\n");
         fails++;
      }
      else
         printf("ok:   seeded state follows its own stream\n");
   }

   /* Test 2: two differently seeded states must produce different
      sequences. */
   {
      RandStateCell a, b;
      memset(&a, 0, sizeof(a));
      memset(&b, 0, sizeof(b));
      a.xstate = 1; a.ystate = 2; a.zstate = 3; a.cstate = 4;
      b.xstate = 100; b.ystate = 200; b.zstate = 300; b.cstate = 400;

      int identical = 1;
      for(int i = 0; i < 10; i++)
      {
         if(JKISSRand(&a) != JKISSRand(&b)) { identical = 0; break; }
      }
      if(identical)
      {
         printf("FAIL: different seeds produce identical sequences\n");
         fails++;
      }
      else
         printf("ok:   different seeds give different streams\n");
   }

   /* Test 3: seeding twice with the same values must give the same
      stream (determinism of the state object). */
   {
      RandStateCell a, b;
      memset(&a, 0, sizeof(a));
      memset(&b, 0, sizeof(b));
      a.xstate = b.xstate = 555;
      a.ystate = b.ystate = 666;
      a.zstate = b.zstate = 777;
      a.cstate = b.cstate = 888;

      int ok = 1;
      for(int i = 0; i < 10; i++)
         if(JKISSRand(&a) != JKISSRand(&b)) { ok = 0; break; }
      if(!ok)
      {
         printf("FAIL: same seeds give different sequences\n");
         fails++;
      }
      else
         printf("ok:   same seeds give identical streams\n");
   }

   if(!fails)
      printf("ALL PASSED\n");
   else
      printf("%d FAILURES\n", fails);
   return fails ? 1 : 0;
}
