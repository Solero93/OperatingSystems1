#include "servicios.h"

int main() {
  int i, j, k, pid;
  for (i=0 ; i<10; i++){
	k=0;
	pid = get_pid();
	crear_proceso((void *)"espera");
	printf("yosoy: pid = %d\n", pid);
	for (j=0; j<=12121210; j++){
		k+=2*j;
	}
  }
  return 0;
}