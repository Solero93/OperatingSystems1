#include "servicios.h"

int main() {
  int i, j, k, pid = get_pid();
  for (i=0 ; i<50; i++){
	k=0;
	printf("yosoy: pid = %d\n", pid);
	for (j=0; j<=1212121; j++){
		k+=2*j;
	}
  }
  return 0;
}