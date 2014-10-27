#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
  format("CS3026 Operating Systems Assessment 2014");
  writedisk("virtualdisk");
  return 0;
}