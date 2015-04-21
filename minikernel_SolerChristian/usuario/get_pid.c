#include "servicios.h"

int main() {
  int pid = get_pid();
  printf("get_pid: pid = %d\n", pid);
  return 0;
}
