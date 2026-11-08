/* Regression test for MinHeap key updates (BASICS/clb_min_heap.c).
 *
 * Before the fix, MinHeapDecrKey() called drop_down() and
 * MinHeapIncrKey() called bubble_up(), i.e. the opposite operations:
 * a decreased key (which may violate the order wrt its parent) was
 * sifted down, and an increased key (which may violate the order wrt
 * its children) was sifted up. Both left the heap order broken.
 */
#include <clb_min_heap.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct el
{
   long val;
   long heap_idx; /* maintained via setter */
}El;

static El* els;
static int nels;

static int cmp_el(IntOrP* a, IntOrP* b)
{
   El* ea = (El*) a->p_val;
   El* eb = (El*) b->p_val;
   return (ea->val < eb->val) ? -1 : (ea->val > eb->val) ? 1 : 0;
}

static void setter(void* p, int idx)
{
   ((El*) p)->heap_idx = idx;
}

/* Count min-heap property violations (parent smaller than child). */
static long heap_violations(MinHeap_p h)
{
   long sp = MinHeapSize(h);
   long bad = 0;
   for(long i = 1; i < sp; i++)
   {
      long p = (i - 1) / 2;
      El* c = (El*) PStackElementP(h->arr, i);
      El* pa = (El*) PStackElementP(h->arr, p);
      if(c->val < pa->val) bad++;
   }
   return bad;
}

static void build(MinHeap_p h, long n)
{
   els = (El*) malloc(n * sizeof(El));
   nels = (int) n;
   for(long i = 0; i < n; i++)
   {
      els[i].val = (long)(rand() % 1000);
      els[i].heap_idx = -1;
      MinHeapAddP(h, &els[i]);
   }
}

int main(void)
{
   int fails = 0;
   srand(777);

   /* Test 1: basic pop order (the test from the comment block in
      clb_min_heap.c). */
   {
      MinHeap_p h = MinHeapAllocWithIndex(cmp_el, setter);
      build(h, 64);
      long prev = -1;
      for(long i = 0; i < 64; i++)
      {
         El* e = (El*) MinHeapPopMinP(h);
         if(e->val < prev)
         {
            printf("FAIL pop-order at %ld: %ld < %ld\n", i, e->val, prev);
            fails++;
            break;
         }
         prev = e->val;
      }
      MinHeapFree(h);
      free(els);
   }

   /* Test 2: DecrKey must move the element UP if needed. Decrease the
      element in slot 1 (child of the root) below the root's value. */
   {
      MinHeap_p h = MinHeapAllocWithIndex(cmp_el, setter);
      build(h, 200);

      El* e = (El*) PStackElementP(h->arr, 1);
      long parent_val = ((El*) PStackElementP(h->arr, 0))->val;
      e->val = parent_val - 1000; /* now strictly smaller than parent */
      MinHeapDecrKey(h, 1);
      if(heap_violations(h))
      {
         printf("FAIL DecrKey: heap property violated after decreasing key\n");
         fails++;
      }
      else
         printf("ok:   DecrKey keeps heap property\n");

      {
         El* first = (El*) MinHeapPopMinP(h);
         if(first->val != e->val)
         {
            printf("FAIL DecrKey: popmin returns %ld, expected new min %ld\n",
                   first->val, e->val);
            fails++;
         }
         else
            printf("ok:   DecrKey: new minimum popped first\n");
      }
      MinHeapFree(h);
      free(els);
   }

   /* Test 3: IncrKey must move the element DOWN if needed. Increase the
      root above all its children. */
   {
      MinHeap_p h = MinHeapAllocWithIndex(cmp_el, setter);
      build(h, 200);

      El* e = (El*) PStackElementP(h->arr, 0);
      e->val = 1000000; /* now larger than all children */
      MinHeapIncrKey(h, 0);
      if(heap_violations(h))
      {
         printf("FAIL IncrKey: heap property violated after increasing key\n");
         fails++;
      }
      else
         printf("ok:   IncrKey keeps heap property\n");
      MinHeapFree(h);
      free(els);
   }

   /* Test 4: stress DecrKey/IncrKey, then verify full pop order. */
   {
      MinHeap_p h = MinHeapAllocWithIndex(cmp_el, setter);
      build(h, 500);
      for(int t = 0; t < 5000; t++)
      {
         long i = rand() % nels;
         long newval = (long)(rand() % 5000);
         El* e = &els[i];
         long oldval = e->val;
         e->val = newval;
         if(newval < oldval)
            MinHeapDecrKey(h, e->heap_idx);
         else
            MinHeapIncrKey(h, e->heap_idx);
      }
      long prev = -1, sp = MinHeapSize(h);
      for(long i = 0; i < sp; i++)
      {
         El* e = (El*) MinHeapPopMinP(h);
         if(e->val < prev)
         {
            printf("FAIL stress pop-order at %ld: %ld < %ld\n", i, e->val, prev);
            fails++;
            break;
         }
         prev = e->val;
      }
      MinHeapFree(h);
      free(els);
   }

   if(!fails)
      printf("ALL PASSED\n");
   else
      printf("%d FAILURES\n", fails);
   return fails ? 1 : 0;
}
