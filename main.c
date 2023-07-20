//#include "maquina_ponteiro.h"
#include "maquina_fixa.h"
#include <stdio.h>
#include <time.h>

int main() {
  clock_t start, end;
  double cpu_time_used;

  start = clock();

  maquina_fixa();

  end = clock();

  cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("\nFunction execution time: %f seconds\n", cpu_time_used);

  return 0;
}
