#include "servicios.h"

int main() {
  int pid = obtener_pid();
  printf("printid: pid = %d\n", pid);
  return 0;
}
