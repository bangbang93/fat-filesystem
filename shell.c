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

  print_fat(10);

  for (int i = 0; i < 4 * BLOCKSIZE; i++){
    // myfputc(string[rand() % (int) (sizeof string - 1)], test_file); //use for random data
    myfputc('0', test_file);
  }

  myfclose(test_file);

  print_fat(10);

  FILE *f = fopen("testfileC3_C1_copy.txt", "w");
  my_file_t *test_file2 = myfopen("testfile.txt", "r");
  move_to_data(test_file2);
  while(1){
    char character = myfgetc(test_file2);
    if (character == EOF){
      break;
    }
    printf("%c", character);
    fprintf(f, "%c", character);
  }
  printf("\n");
  fclose(f);
  myfclose(test_file2);

  // Save the changes made to the virtual disk
  write_disk("virtualdisk\0");

  return 0;
}
