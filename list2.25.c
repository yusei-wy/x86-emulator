#include <stdio.h>
#include <stdlib.h>

int cmp(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

int main(void) {
  int arr[] = {3, 2, 5, 4, 1};
  qsort(arr, 5, sizeof(int), cmp);
  for (int i = 0; i < 5; i++) {
    printf("%d ", arr[i]);
  }

  return 0;
}
