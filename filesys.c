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

/*write_disk : writes virtual disk out to physical disk
 *in: file name of stored virtual disk
*/
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

// use this for testing
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

/*read and write FAT
 please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
  our blocksize for the virtual disk is 1024, therefore
  we can store 512 FAT entries in one block

  how many disk blocks do we need to store the complete FAT:
  - our virtual disk has MAXBLOCKS blocks, which is currently 1024
    each block is 1024 bytes long
  - our FAT has MAXBLOCKS entries, which is currently 1024
    each FAT entry is a fatentry_t, which is currently 2 bytes
  - we need(MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
    FAT
  - each block can hold(BLOCKSIZE / sizeof(fatentry_t)) fat entries
*/

// implement format()
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
