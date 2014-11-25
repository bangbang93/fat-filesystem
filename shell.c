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

void cgs_b(){
  // - create a directory “/myfirstdir/myseconddir/mythirddir” in the virtual disk
  mymkdir("/myfirstdir/myseconddir/mythirddir");
  // - call mylistdir(“/myfirstdir/myseconddir”): print out the list of strings returned by this function
  char **list = mylistdir("/myfirstdir/myseconddir");
  print_dir_list(list);
  // - write out virtual disk to “virtualdiskB3_B1_a”
  write_disk("virtualdiskB3_B1_a\0");
  // - create a file “/myfirstdir/myseconddir/testfile.txt” in the virtual disk
  my_file_t *testfile = myfopen("/myfirstdir/myseconddir/testfile.txt", "w");
  // - call mylistdir(“/myfirstdir/myseconddir”): print out the list of strings returned by this function
  list = mylistdir("/myfirstdir/myseconddir");
  print_dir_list(list);
  // - write out virtual disk to “virtualdiskB3_B1_b”
  write_disk("virtualdiskB3_B1_b\0");
}

int main() {
  cgs_d();
  cgs_c();
  cgs_b();

  // Print and Save the changes made to the virtual disk
  // print_fat(20);
  printf("--------\n");
  print_directory_structure(3, 0);
  write_disk("virtualdisk\0");
  return 0;
}
