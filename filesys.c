/*filesys.c
   provides interface to virtual disk
*/

#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

diskblock_t virtual_disk[MAXBLOCKS];  // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t FAT[MAXBLOCKS];            // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t root_dir_index = 0;          // rootDir will be set by format
direntry_t *current_dir = NULL;
fatentry_t current_dir_index = 0;

void write_disk(const char *file_name)
{
  printf("write_disk> %s\n", virtual_disk[0].data);
  FILE *dest = fopen(file_name, "w");
  fwrite(virtual_disk, sizeof(virtual_disk), 1, dest);
  // write(dest, virtual_disk, sizeof(virtual_disk));
  fclose(dest);
}

void read_disk(const char *file_name)
{
  FILE *dest = fopen(file_name, "r");
  // if(fread(virtual_disk, sizeof(virtual_disk), 1, dest) < 0)
  //    fprintf(stderr, "read from virtual disk to disk failed\n");
  //write(dest, virtual_disk, sizeof(virtual_disk));
  fclose(dest);
}

void print_block(int block_index, char type)
{
  if (type == 'd') {
    printf("virtualdisk[%d] = %s\n", block_index, virtual_disk[block_index].data);
  }
  else if (type == 'f') {
    printf("virtualdisk[%d] = ", block_index);
    for(int i = 0; i < FATENTRYCOUNT; i++) {
      printf("%d", virtual_disk[block_index].fat[i]);
    }
    printf("\n");
  }
  else if (type == 'r') {
    printf("virtualdisk[%d] = \n", block_index);
    printf("is_dir: %d\n", virtual_disk[block_index].dir.is_dir);
    printf("next_entry: %d\n", virtual_disk[block_index].dir.next_entry);
  }
  else {
    printf("Invalid Type");
  }
}

void print_fat(int length)
{
  for(int i = 0; i < length; i++){
    printf("%d, ", FAT[i]);
  }
  printf("\n");
}

/*the basic interface to the virtual disk
 *this moves memory around */
void write_block(diskblock_t *block, int block_address, char type, int print)
{
  if (type == 'd') { //block is data
    if (print == 1)
      printf("write block> %d = %s\n", block_address, block->data);
    memmove(virtual_disk[block_address].data, block->data, BLOCKSIZE);
  }
  else if (type == 'f') { // block is fat
    if (print == 1) {
      printf("write block> %d = ", block_address);
      for(int i = 0; i < FATENTRYCOUNT; i++) printf("%d", block->fat[i]);
      printf("\n");
    }
    memmove(virtual_disk[block_address].fat, block->fat, BLOCKSIZE);
  }
  // else if (type == 'r') { //block if dir
  //   memmove(virtual_disk[block_address].dir, block->dir, BLOCKSIZE);
  // }
  else {
    printf("Invalid Type");
  }
}

void read_block(diskblock_t *block, int block_address, char type, int print)
{
  if (type == 'd') { //block is data
    if (print == 1)
      printf("read block> %d = %s\n", block_address, virtual_disk[block_address].data);
    memmove(block->data, virtual_disk[block_address].data, BLOCKSIZE);
  }
  else if (type == 'f') { // block is fat
    if (print == 1) {
      printf("read block> %d = ", block_address);
      for(int i = 0; i < FATENTRYCOUNT; i++) printf("%d", virtual_disk[block_address].fat[i]);
      printf("\n");
    }
    memmove(block->fat, virtual_disk[block_address].fat, BLOCKSIZE);
  }
  else {
    printf("Invalid Type");
  }
}

void copy_fat(fatentry_t *FAT)
{
  diskblock_t block;
  int index = 0;
  for(int x = 1; x <= (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))); x++) {
    for(int y = 0; y < (BLOCKSIZE / sizeof(fatentry_t)); y++){
      block.fat[y] = FAT[index++];
    }
    write_block(&block, x, 'f', FALSE);
  }
}

void format(char *volume_name)
{
  diskblock_t block;
  // int pos = 0;
  // int fatentry = 0;
  int required_fat_space = (MAXBLOCKS / FATENTRYCOUNT);

  for (int i = 0; i < BLOCKSIZE; i++) {
    block.data[i] = '\0';
  }

  memcpy(block.data, volume_name, strlen(volume_name));
  write_block(&block, 0, 'd', FALSE);

  FAT[0] = ENDOFCHAIN;
  FAT[1] = 2;
  FAT[2] = ENDOFCHAIN;
  FAT[3] = ENDOFCHAIN;
  for(int i = 4; i < MAXBLOCKS; i++){
    FAT[i] = UNUSED;
  }
  copy_fat(FAT);

  diskblock_t root_block;
  int root_block_index = required_fat_space + 1;
  root_block.dir.is_dir = TRUE;
  root_block.dir.next_entry = 0;
  write_block(&root_block, root_block_index, 'd', FALSE);
  root_dir_index = root_block_index;
  current_dir_index = root_dir_index;
}

void init_block(diskblock_t *block)
{
  for (int i = 0; i < BLOCKSIZE; i++) {
    block->data[i] = '\0';
  }
}

int next_unallocated_block()
{
  for(int i = 0; i < MAXBLOCKS; i++){
    if (FAT[i] == UNUSED){
      FAT[i] = 0;
      copy_fat(FAT);
      return i;
    }
  }
  return -1; //disk is full
}

int last_block_in_file(my_file_t *file)
{
  int next_block = file->blockno;
  while (FAT[next_block] != 0) {
    next_block = FAT[next_block];
  }
  return next_block;
}

int file_index(char *filename){
  for(int i = 0; i < MAXBLOCKS; i++){
    if (memcmp(virtual_disk[i].data, filename, strlen(filename) + 1) == 0){
      return i;
    }
  }
  return -1;
}

void move_pos_to_end(my_file_t *file){
  //last block
  while(1) {
    file->blockno = FAT[file->blockno];
    if(FAT[file->blockno] == 0) break;
  }

  // last character
  file->buffer = virtual_disk[file->blockno];
  while(1) {
    if(file->buffer.data[file->pos+++1] == '\0') break;
  }
}

my_file_t *myfopen(char *filename, char *mode)
{
  int location_on_disk = file_index(filename);
  diskblock_t first_block = virtual_disk[location_on_disk]; //should this be malloc?
  if(location_on_disk == -1){
    printf("File did not exist. Creating new file: %s\n", filename);
    location_on_disk = next_unallocated_block();
    init_block(&first_block);
    memcpy(first_block.data, filename, strlen(filename));
    write_block(&first_block, location_on_disk, 'd', FALSE);
  }
  my_file_t *file = malloc(sizeof(my_file_t));
  file->pos = 0;
  file->writing = 0;
  memcpy(file->mode, mode, strlen(mode));
  file->blockno = location_on_disk;
  file->buffer = first_block;

  // add a second block
  FAT[location_on_disk] = next_unallocated_block();
  diskblock_t second_block = virtual_disk[FAT[location_on_disk]];
  init_block(&second_block);
  write_block(&second_block, FAT[location_on_disk], 'd', FALSE);

  if(strncmp(file->mode, "a", 1) == 0){
    move_pos_to_end(file);
  }
  file->pos--; //need to set this for when no default content is assigned

  return file;
}

char myfgetc(my_file_t *file)
{
  int position = file->pos;
  file->pos++;
  if ((file->buffer.data[position] == '\0') && (FAT[file->blockno] != 0)){
    printf(" - NEXT BLOCK - ");
    file->pos = 1;
    file->blockno = FAT[file->blockno];
    file->buffer = virtual_disk[file->blockno];
    return file->buffer.data[file->pos - 1];
  }
  else {
    return file->buffer.data[position];
  }
}

int myfputc(char character, my_file_t *file)
{
  if(strncmp(file->mode, "r", 1) == 0){
    printf("Write of '%c' Failed: File is 'Read Only'\n", character);
    return EOF;
  } else {
    if (file->pos >= BLOCKSIZE){
      if(FAT[file->blockno] != 0){
        file->pos = 0;
        file->blockno = FAT[file->blockno];
        file->buffer = virtual_disk[file->blockno];
      } else {
        FAT[file->blockno] = next_unallocated_block();
        diskblock_t new_block = virtual_disk[FAT[file->blockno]];
        init_block(&new_block);
        write_block(&new_block, FAT[file->blockno], 'd', FALSE);
        FAT[FAT[file->blockno]] = 0;
        copy_fat(FAT);
      }
    }

    file->buffer.data[file->pos] = character;
    file->pos++;
    write_block(&file->buffer, file->blockno, 'd', FALSE);

    return 0;
  }
}

int myfclose(my_file_t *file)
{
  free(file);
  return 0; //unless there's an error?
}

// void save_file()
// {
//   // read
//   my_file_t *read_file = myfopen("read.txt", "r");
//   char c;
//   do {
//     c = myfgetc(read_file);
//     if( c == '\0' )
//     {
//       break;
//     }
//     printf("%c", c);
//   } while(1);
//   printf("\n");

//   //write
//   my_file_t *write_file = myfopen("write.txt", "w");
//   myfputc('1', write_file);
//   myfputc('2', write_file);
//   printf("\n");

//   //append
//   my_file_t *append_file = myfopen("append.txt", "a");
//   myfputc('1', append_file);
//   myfputc('2', append_file);
//   printf("\n");

//   // close file
//   myfclose(read_file);
//   myfclose(write_file);
//   myfclose(append_file);

//   printf("\n");
// }
