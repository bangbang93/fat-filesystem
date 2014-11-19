#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
  //CGS D3-D1
  format("CS3026 Operating Systems Assessment 2014\0");

  // CGS C3-C1
  my_file_t *test_file = myfopen("testfile.txt", "a");
  char string[] = "4096bytes"; //string used for filling the file with data

  for (int i = 0; i < (4 * BLOCKSIZE) - 1; i++){
    // myfputc(string[rand() % (int) (sizeof string - 1)], test_file); //use for random data
    myfputc(string[1], test_file); //all zeros
  }

  // Save the changes made to the virtual disk
  write_disk("virtualdisk\0");

  return 0;
}