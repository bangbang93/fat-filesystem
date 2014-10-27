/*filesys.c
   provides interface to virtual disk
*/

#include "filesys.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

diskblock_t virtualDisk[MAXBLOCKS];  // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t FAT[MAXBLOCKS];            // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t rootDirIndex = 0;          // rootDir will be set by format
direntry_t *currentDir = NULL;
fatentry_t currentDirIndex = 0;

/*writedisk : writes virtual disk out to physical disk
 *in: file name of stored virtual disk
*/

// use this for testing
void printBlock(int blockIndex, char type)
{
  if (type == 'd') {
    printf("virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data);
  }
  else if (type == 'f') {
    printf("virtualdisk[%d] = ", blockIndex);
    for(int i = 0; i < FATENTRYCOUNT; i++) {
      printf("%d", virtualDisk[blockIndex].fat[i]);
    }
    printf("\n");
  }
  else {
    printf("Invalid Type");
  }
}

void writedisk(const char *filename)
{
  printf("writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data);
  FILE *dest = fopen(filename, "w");
  // if(fwrite(virtualDisk, sizeof(virtualDisk), 1, dest) < 0)
  //    fprintf(stderr, "write virtual disk to disk failed\n");
  //write(dest, virtualDisk, sizeof(virtualDisk));
  fclose(dest);
}

void readdisk(const char *filename)
{
  FILE *dest = fopen(filename, "r");
  // if(fread(virtualDisk, sizeof(virtualDisk), 1, dest) < 0)
  //    fprintf(stderr, "read from virtual disk to disk failed\n");
  //write(dest, virtualDisk, sizeof(virtualDisk));
  fclose(dest);
}

/*the basic interface to the virtual disk
 *this moves memory around */

void writeBlock(diskblock_t *block, int block_address, char type, int print)
{
  if (type == 'd') { //block is data
    if (print == 1)
      printf("write block> %d = %s\n", block_address, block->data);
    memmove(virtualDisk[block_address].data, block->data, BLOCKSIZE);
  }
  else if (type == 'f') { // block is fat
    if (print == 1) {
      printf("write block> %d = ", block_address);
      for(int i = 0; i < FATENTRYCOUNT; i++) printf("%d", block->fat[i]);
    }
    memmove(virtualDisk[block_address].fat, block->fat, BLOCKSIZE);
  }
  else {
    printf("Invalid Type");
  }
}

void copyFat(fatentry_t *FAT)
{
  diskblock_t block;
  int index = 0;
  for(int x = 1; x <= (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))); x++) {
    for(int y = 0; y < (BLOCKSIZE / sizeof(fatentry_t)); y++){
      block.fat[y] = FAT[index++];
    }
    writeBlock(&block, x, 'f', FALSE);
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
void format(char *volumeName)
{
  diskblock_t block;
  direntry_t  rootDir;
  int pos = 0;
  int fatentry = 0;
  int fatblocksneeded = (MAXBLOCKS / FATENTRYCOUNT);

  for (int i = 0; i < BLOCKSIZE; i++) {
    block.data[i] = '\0';
  }

  memcpy(block.data, volumeName, strlen(volumeName));
  writeBlock(&block, 0, 'd', FALSE);

  FAT[0] = ENDOFCHAIN;
  FAT[1] = 2;
  FAT[2] = ENDOFCHAIN;
  FAT[3] = ENDOFCHAIN;
  for(int i = 4; i < MAXBLOCKS; i++){
    FAT[i] = UNUSED;
  }
  copyFat(FAT);

  printBlock(0, 'd');
  printBlock(1, 'f');
  printBlock(2, 'f');
  printBlock(3, 'd');

  /*prepare root directory
  *write root directory block to virtual disk
  */
}
