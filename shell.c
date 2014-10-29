#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
  format("CS3026 Operating Systems Assessment 2014");
  write_disk("virtualdisk");
  return 0;
}