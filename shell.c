#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void cgs_d(){
  format("CS3026 Operating Systems Assessment 2014\0");
  write_disk("virtualdiskD3_D1\0");
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
  write_disk("virtualdiskC3_C1\0");
}

void cgs_b(){
  // - create a directory “/myfirstdir/myseconddir/mythirddir” in the virtual disk
  mymkdir("/myfirstdir/myseconddir/mythirddir");
  // - call mylistdir(“/myfirstdir/myseconddir”): print out the list of strings returned by this function
  char **list = mylistdir("/myfirstdir/myseconddir");
  // print_dir_list(list);
  // - write out virtual disk to “virtualdiskB3_B1_a”
  write_disk("virtualdiskB3_B1_a\0");
  // - create a file “/myfirstdir/myseconddir/testfile.txt” in the virtual disk
  // my_file_t *testfile = myfopen("/myfirstdir/myseconddir/testfile.txt", "w");
  // - call mylistdir(“/myfirstdir/myseconddir”): print out the list of strings returned by this function
  list = mylistdir("/myfirstdir/myseconddir");
  // print_dir_list(list);
  // - write out virtual disk to “virtualdiskB3_B1_b”
  write_disk("virtualdiskB3_B1_b\0");
}

int main() {
  cgs_d();
  cgs_c();
  cgs_b();

  // level A
  format("CS3026 Operating Systems Assessment 2014\0");
  write_disk("virtualdiskD3_D1\0");
  // - create a directory “/firstdir/seconddir” in the virtual disk
  mymkdir("/firstdir/seconddir");
  // - call myfopen( “/firstdir/seconddir/testfile1.txt” )
  my_file_t *test_file = myfopen("/firstdir/seconddir/testfile1.txt", "w");
  // - you may write something into the file
  char string[] = "Level A Content";
  for (int i = 0; i < strlen(string); i++){
    myfputc(string[i], test_file);
  }
  myfclose(test_file);
  // - call mylistdir(“/firstdir/seconddir”): print out the list of strings returned by this function
  print_dir_list(mylistdir("/firstdir/seconddir"));
  // - change to directory “/firstdir/seconddir”
  mychdir("/firstdir/seconddir");
  // - call mylistdir(“/firstdir/seconddir/” ) or mylistdir(“.”) to list the current dir... & print
  print_dir_list(mylistdir("/firstdir/seconddir"));
  // - call myfopen( “testfile2.txt, “w” )
  my_file_t *test_file2 = myfopen("/testfile2.txt", "w");
  // - you may write something into the file
  char string2[] = "Level A Content2";
  for (int i = 0; i < strlen(string2); i++){
    myfputc(string[i], test_file2);
  }
  // - close the file
  myfclose(test_file2);
  // - create directory “thirddir”
  mymkdir("thirddir");

  // ￼￼￼CGS A5-A1
  // - call myfopen( “thirddir/testfile3.txt, “w” )
  my_file_t *test_file3 = myfopen("thirddir/testfile3.txt", "w");
  // - you may write something into the file
  char string3[] = "Level A Content3";
  for (int i = 0; i < strlen(string2); i++){
    myfputc(string[i], test_file3);
  }
  // - close the file
  myfclose(test_file3);
  // - write out virtual disk to “virtualdiskA5_A1_a”
  write_disk("virtualdiskA5_A1_a\0");
  // - call myremove( “testfile1.txt” )
  myremove("testfile1.txt");
  // - call myremove( “testfile2.txt” )
  myremove("testfile2.txt");
  // - write out virtual disk to “virtualdiskA5_A1_b”
  write_disk("virtualdiskA5_A1_b\0");
  // - call mychdir (thirddir”)
  mychdir("thirddir");
  // - call myremove( “testfile3.txt”)
  myremove("testfile3.txt");
  // - write out virtual disk to “virtualdiskA5_A1_c”
  write_disk("virtualdiskA5_A1_c\0");
  // - call mychdir( “/firstdir/seconddir”) or mychdir(“..”)
  mychdir("/firstdir/seconddir");
  // - call myremdir( “thirddir” )
  myrmdir("thirddir");
  // - call mychdir(“/firstdir”)
  mychdir("/firstdir");
  // - call myrmdir ( “seconddir” )
  myrmdir("seconddir");
  // - call mychdir(“/”) or mychdir(“..”)
  mychdir("/");
  // - call myrmdir( “firstdir”)
  myrmdir("firstdir");
  // - write out virtual disk to “virtualdiskA5_A1_d”
  write_disk("virtualdiskA5_A1_d\0");

  // Print and Save the changes made to the virtual disk
  printf("\n--------\n");
  // print_fat(20);
  print_directory_structure(3, 0);
  write_disk("virtualdisk\0");
  return 0;
}
