#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
  format("CS3026 Operating Systems Assessment 2014\0");

  // save_file();

  write_disk("virtualdisk\0");

  return 0;
}