#include "servicios.h"

int main() {
  int i, pid = get_pid();
  for (i=0 ; i<10; i++){
	printf("printid: pid = %d\n", pid);
  }
  return 0;
}
