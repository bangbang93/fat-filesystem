#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
  format("CS3026 Operating Systems Assessment 2014\0");

  // save_file();

  my_file_t *test_file = myfopen("testfile.txt", "w");

  for (int i = 0; i < 5; i++){
    for (int j = 0; j < BLOCKSIZE; j++){
      // printf("%d\n",j);
      myfputc('c', test_file);
    }
  }

  write_disk("virtualdisk\0");

  return 0;
}