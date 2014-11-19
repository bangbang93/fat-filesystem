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
  // myfputc(string[rand() % (int) (sizeof string - 1)], test_file); //use for random data

  print_fat(10);

  for (int i = 0; i < BLOCKSIZE; i++){
    myfputc('0', test_file);
  }
  for (int i = 0; i < BLOCKSIZE; i++){
    myfputc('1', test_file);
  }
  // for (int i = 0; i < BLOCKSIZE; i++){
  //   myfputc('2', test_file);
  // }
  // for (int i = 0; i < BLOCKSIZE; i++){
  //   myfputc('3', test_file);
  // }

  print_block(4, 'd');
  print_block(5, 'd');
  print_block(6, 'd');
  print_block(7, 'd');
  print_block(8, 'd');
  print_block(9, 'd');


  // myfclose(test_file);

  // print_fat(10);
  // my_file_t *test_file2 = myfopen("testfile.txt", "r");
  // move_to_data(test_file2);
  // for(int i = 0; i < 4 * BLOCKSIZE; i++){
  //   char character = myfgetc(test_file);
  //   printf("%c", character);
  // }
  // printf("\n%d\n", test_file->pos);
  // printf("\n");

  // print_block(4, 'd');
  // print_block(5, 'd');
  // print_block(6, 'd');
  // print_block(7, 'd');
  // print_block(8, 'd');

  // Save the changes made to the virtual disk
  write_disk("virtualdisk\0");

  return 0;
}
