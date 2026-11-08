/* Regression test for PStackBinSearch (BASICS/clb_pstacks.c).
 *
 * The documented contract is: "Return index of key, when found, or index
 * of the next bigger element if not." Before the fix, the function used
 * `upper = i-1` and `return lower+1`, which returned one past the
 * insertion point (and could exceed the stack size) whenever the key was
 * not present.
 */
#include <clb_pstacks.h>
#include <stdio.h>
#include <stdlib.h>

static int cmp_long_ptr(const void* a, const void* b)
{
   const long* la = (const long*) a;
   const long* lb = (const long*) b;
   return (*la < *lb) ? -1 : (*la > *lb) ? 1 : 0;
}

static int cmp_long_raw(const void* a, const void* b)
{
   long x = *(const long*) a;
   long y = *(const long*) b;
   return (x < y) ? -1 : (x > y) ? 1 : 0;
}

int main(void)
{
   int fails = 0;

   /* Fixed example: [10,20,30,40]. */
   {
      long vals[] = {10, 20, 30, 40};
      PStack_p st = PStackAlloc();
      for(long i = 0; i < 4; i++)
      {
         IntOrP e; e.p_val = (void*) &vals[i];
         push(st, e);
      }
      long keys[] = {5, 10, 15, 30, 35, 40, 45};
      for(size_t k = 0; k < 7; k++)
      {
         PStackPointer got = PStackBinSearch(st, (void*) &keys[k], 0,
                                             PStackGetSP(st), cmp_long_ptr);
         PStackPointer want = 4; /* lower_bound */
         for(long i = 0; i < 4; i++)
            if(vals[i] >= keys[k]) { want = i; break; }
         if(got == want || (got < 4 && vals[got] == keys[k]))
            printf("ok:   key=%2ld -> %ld\n", keys[k], got);
         else
         {
            printf("FAIL: key=%2ld got=%ld want=%ld\n", keys[k], got, want);
            fails++;
         }
      }
      PStackFree(st);
   }

   /* Randomised tests against the documented contract. */
   srand(12345);
   for(int t = 0; t < 100000; t++)
   {
      long n = 1 + (rand() % 64);
      long* arr = (long*) malloc(n * sizeof(long));
      for(long i = 0; i < n; i++) arr[i] = (long)(rand() % 200);
      qsort(arr, n, sizeof(long), cmp_long_raw);

      PStack_p s2 = PStackAlloc();
      for(long i = 0; i < n; i++)
      {
         IntOrP e; e.p_val = (void*) &arr[i];
         push(s2, e);
      }
      long key = (long)(rand() % 202) - 1;
      PStackPointer got = PStackBinSearch(s2, (void*) &key, 0, n, cmp_long_ptr);

      PStackPointer lb = n;
      for(long i = 0; i < n; i++)
         if(arr[i] >= key) { lb = i; break; }

      int ok = (got == lb) || (got < n && arr[got] == key);
      if(!ok)
      {
         printf("FAIL rand t=%d n=%ld key=%ld got=%ld lower_bound=%ld\n",
                t, n, key, got, lb);
         fails++;
         free(arr);
         PStackFree(s2);
         break;
      }
      free(arr);
      PStackFree(s2);
   }

   if(!fails)
      printf("ALL PASSED\n");
   else
      printf("%d FAILURES\n", fails);
   return fails ? 1 : 0;
}
