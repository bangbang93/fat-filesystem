#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void cgs_d(){
  format("CS3026 Operating Systems Assessment 2014\0");
}

void cgs_c(){
  // create a new file in write mode
  my_file_t *test_file = myfopen("test_file.txt", "w");

  // the contents of this string are used when assigning random data to the file
  char string[] = "4096bytes"; //string used for filling the file with data

  // fill a 4kb file
  for (int i = 0; i < 4 * BLOCKSIZE; i++){
    // myfputc(string[rand() % (int) (sizeof string - 1)], test_file); //use for random data
    myfputc('0', test_file);
  }

  myfclose(test_file);

  // use the real fopen to write the output to a text file
  FILE *f = fopen("testfileC3_C1_copy.txt", "w");

  // open our test_file in read mode
  my_file_t *test_file2 = myfopen("test_file.txt", "r");

  // loop over it until it hits EOF
  while(1){
    char character = myfgetc(test_file2);
    if (character == EOF){
      break;
    }
    // print it to the console and write it in the real file
    // printf("%c", character);
    fprintf(f, "%c", character);
  }
  // printf("\n");
  fclose(f);
  myfclose(test_file2);
}

int main() {
  cgs_d();
  cgs_c();

  print_fat(10);

  // my_file_t *test_file = myfopen("test_file.txt", "w");
  // my_file_t *test_file2 = myfopen("test_file2.txt", "w");
  // my_file_t *test_file3 = myfopen("test_file3.txt", "w");
  // my_file_t *test_file4 = myfopen("test_file4.txt", "w");
  // my_file_t *test_file5 = myfopen("test_file.txt", "w");

  print_directory_structure(3, 0);

  // Save the changes made to the virtual disk
  write_disk("virtualdisk\0");

  return 0;
}
