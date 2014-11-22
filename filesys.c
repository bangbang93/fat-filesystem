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
    printf("virtualdisk[%d] = ", block_index);
    for (int i = 0; i < BLOCKSIZE; i++)
    {
      printf("%c", virtual_disk[block_index].data[i]);
    }
    printf("\n");
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
    // printf("entry_list: %d\n", virtual_disk[block_index].dir.entrylist);
    for(int i = 0; i < DIRENTRYCOUNT; i++){
      //this should be 'unused' rather than 'name'
      if(strlen(virtual_disk[block_index].dir.entrylist[i].name) == 0){
        printf("Empty direntry_t\n");
      }
      else {
        printf("%d: ", i);
        printf("entrylength: %d, ", virtual_disk[block_index].dir.entrylist[i].entrylength);
        printf("id_dir: %d, ", virtual_disk[block_index].dir.entrylist[i].is_dir);
        printf("unused: %d, ", virtual_disk[block_index].dir.entrylist[i].unused);
        printf("modtime: %ld, ", virtual_disk[block_index].dir.entrylist[i].modtime);
        printf("file_length: %d, ", virtual_disk[block_index].dir.entrylist[i].file_length);
        printf("first_block: %d, ", virtual_disk[block_index].dir.entrylist[i].first_block);
        printf("name: '%s'\n", virtual_disk[block_index].dir.entrylist[i].name);
      }
    }
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

// the basic interface to the virtual disk
void write_block(diskblock_t *block, int block_address, char type)
{
  if (type == 'd') { //block is data
    memmove(virtual_disk[block_address].data, block->data, BLOCKSIZE);
  }
  else if (type == 'f') { // block is fat
    memmove(virtual_disk[block_address].fat, block->fat, BLOCKSIZE);
  }
  else if (type == 'r') { //block is dir, CHECK THIS, dir not working
    memmove(virtual_disk[block_address].data, block->data, BLOCKSIZE);
  }
  else {
    printf("Invalid Type");
  }
}

// void read_block(diskblock_t *block, int block_address, char type, int print)
// {
//   if (type == 'd') { //block is data
//     if (print == 1)
//       printf("read block> %d = %s\n", block_address, virtual_disk[block_address].data);
//     memmove(block->data, virtual_disk[block_address].data, BLOCKSIZE);
//   }
//   else if (type == 'f') { // block is fat
//     if (print == 1) {
//       printf("read block> %d = ", block_address);
//       for(int i = 0; i < FATENTRYCOUNT; i++) printf("%d", virtual_disk[block_address].fat[i]);
//       printf("\n");
//     }
//     memmove(block->fat, virtual_disk[block_address].fat, BLOCKSIZE);
//   }
//   else {
//     printf("Invalid Type");
//   }
// }

void copy_fat(fatentry_t *FAT)
{
  diskblock_t block;
  int index = 0;
  for(int x = 1; x <= (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))); x++) {
    for(int y = 0; y < (BLOCKSIZE / sizeof(fatentry_t)); y++){
      block.fat[y] = FAT[index++];
    }
    write_block(&block, x, 'f');
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
  write_block(&block, 0, 'd');

  FAT[0] = ENDOFCHAIN;
  FAT[1] = 2;
  FAT[2] = ENDOFCHAIN;
  FAT[3] = ENDOFCHAIN;
  for(int i = 4; i < MAXBLOCKS; i++){
    FAT[i] = UNUSED;
  }
  copy_fat(FAT);

  //create a block for to store the root directory
  diskblock_t root_block;
  int root_block_index = required_fat_space + 1;
  root_block.dir.is_dir = TRUE;
  root_block.dir.next_entry = 0;

  // assign an empty entry for all spaces in the root directory
  direntry_t *blank_entry = malloc(sizeof(direntry_t));
  blank_entry->unused = TRUE;
  blank_entry->file_length = 0;
  // memcpy(blank_entry->name, "empty", strlen("empty"));
  for(int i = 0; i < DIRENTRYCOUNT; i ++){
    root_block.dir.entrylist[i] = *blank_entry;
  }

  // write this the directory structure to the disk
  write_block(&root_block, root_block_index, 'd');

  // update the location of the root dir
  root_dir_index = root_block_index;
  current_dir_index = root_dir_index;

  // set the current dir to a blank entry
  current_dir = blank_entry;
}

void init_block(diskblock_t *block)
{
  for (int i = 0; i < BLOCKSIZE; i++) {
    block->data[i] = '\0';
  }
}

void init_dir_block(diskblock_t *block){
  block->dir.is_dir = TRUE;
  block->dir.next_entry = 0;

  // assign an empty entry for all spaces in the root directory
  direntry_t *blank_entry = malloc(sizeof(direntry_t));
  blank_entry->unused = TRUE;
  blank_entry->file_length = 0;
  // memcpy(blank_entry->name, "empty", strlen("empty"));
  for(int i = 0; i < DIRENTRYCOUNT; i ++){
    block->dir.entrylist[i] = *blank_entry;
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
    write_block(&first_block, location_on_disk, 'd');

    // add a second block
    FAT[location_on_disk] = next_unallocated_block();
    diskblock_t second_block = virtual_disk[FAT[location_on_disk]];
    init_block(&second_block);
    write_block(&second_block, FAT[location_on_disk], 'd');
  }

  my_file_t *file = malloc(sizeof(my_file_t));
  file->pos = 0;
  file->writing = 0;
  memcpy(file->mode, mode, strlen(mode));
  file->blockno = location_on_disk;
  file->buffer = first_block;

  if(strncmp(file->mode, "a", 1) == 0){
    move_pos_to_end(file);
    file->pos--; //need to set this for when no default content is assigned
  }

  return file;
}

char myfgetc(my_file_t *file)
{
  int position = file->pos;
  file->pos++;
  if ((file->buffer.data[position] == '\0') && (FAT[file->blockno] != 0)){
    file->pos = 1;
    file->blockno = FAT[file->blockno];
    file->buffer = virtual_disk[file->blockno];
    return file->buffer.data[file->pos - 1];
  }
  //  fix these values to MAXBLOCKSIZE and ENDOFCHAIN
  else if ((file->pos > 1024) && FAT[file->blockno] == 0) {
    return EOF;
  }
  else {
    return file->buffer.data[position];
  }
}

int myfputc(char character, my_file_t *file)
{
  if (file->pos == BLOCKSIZE){
    file->pos = 0;
    if(FAT[file->blockno] == ENDOFCHAIN) {
      FAT[file->blockno] = next_unallocated_block();
      file->blockno = FAT[file->blockno];
      FAT[file->blockno] = 0;
      copy_fat(FAT);

      diskblock_t new_block;
      init_block(&new_block);

      write_block(&new_block, file->blockno, 'd');
      file->buffer = virtual_disk[file->blockno];
    }
    else {
      file->blockno = FAT[file->blockno];
      file->buffer = virtual_disk[file->blockno];
    }
  }

  file->buffer.data[file->pos] = character;
  write_block(&file->buffer, file->blockno, 'd');

  file->pos++;

  return 0;
}

int myfclose(my_file_t *file)
{
  free(file);
  return 0; //unless there's an error?
}

// given the next entry this will return the next one, moving to a new dir block if needed
int next_unallocated_dir_entry(){
  int next_entry = virtual_disk[current_dir_index].dir.next_entry;
  if(next_entry > DIRENTRYCOUNT - 1){
    //revert next value
    virtual_disk[current_dir_index].dir.next_entry = 0;

    int new_dir_block_index = next_unallocated_block();
    FAT[current_dir_index] = new_dir_block_index;
    FAT[new_dir_block_index] = ENDOFCHAIN;
    copy_fat(FAT);

    current_dir_index = new_dir_block_index;

    //create a new dirblock to increase the size of the directory
    diskblock_t new_dir_block = virtual_disk[new_dir_block_index];
    new_dir_block.dir.is_dir = TRUE;
    new_dir_block.dir.next_entry = 0;

    // assign an empty entry for all spaces in the root directory
    direntry_t *blank_entry = malloc(sizeof(direntry_t));
    blank_entry->unused = TRUE;
    blank_entry->file_length = 0;
    // memcpy(blank_entry->name, "empty", strlen("empty"));
    for(int i = 0; i < DIRENTRYCOUNT; i ++){
      new_dir_block.dir.entrylist[i] = *blank_entry;
    }

    // write this the directory structure to the disk
    write_block(&new_dir_block, new_dir_block_index, 'd');

    // update the location of the root dir
    // root_dir_index = new_dir_block_index;
    current_dir_index = new_dir_block_index;

    // set the current dir to a blank entry
    current_dir = blank_entry;
  }
  return virtual_disk[current_dir_index].dir.next_entry++;;
}

void print_directory_structure(int current_dir_block, int indent){
  char string[10]; //up to ten levels
  int x;
  for(x = 0; x < indent; x++){
    string[x] = '\t';
  }

  while(1){
    for(int i = 0; i < DIRENTRYCOUNT; i++){
      printf("%s", string);
      printf("%d: ", i);
      if(strlen(virtual_disk[current_dir_block].dir.entrylist[i].name) == 0){
        printf("empty\n");
      }
      else {
        printf("name: '%s', ", virtual_disk[current_dir_block].dir.entrylist[i].name);
        printf("first_block: %d, ", virtual_disk[current_dir_block].dir.entrylist[i].first_block);
        printf("is_dir: %d, ", virtual_disk[current_dir_block].dir.entrylist[i].is_dir);
        printf("unused: %d, ", virtual_disk[current_dir_block].dir.entrylist[i].unused);
        printf("modtime: %ld, ", virtual_disk[current_dir_block].dir.entrylist[i].modtime);
        printf("file_length: %d, ", virtual_disk[current_dir_block].dir.entrylist[i].file_length);
        printf("entrylength: %d\n", virtual_disk[current_dir_block].dir.entrylist[i].entrylength);

        if (virtual_disk[current_dir_block].dir.entrylist[i].is_dir == 1){
          print_directory_structure(virtual_disk[current_dir_block].dir.entrylist[i].first_block, indent + 1);
        }
      }
    }

    if(FAT[current_dir_block] == 0) break;
    current_dir_block = FAT[current_dir_block];
  }
}

void create_file(){
  //allocate a new block
  int block_index = next_unallocated_block();
  FAT[block_index] = 0;
  diskblock_t block = virtual_disk[block_index];

  //clear it
  init_block(&block);
  memcpy(block.data, "content", strlen("content"));
  write_block(&block, block_index, 'd');

  //find a place for it in the directory
  int next_entry = virtual_disk[current_dir_index].dir.next_entry;
  diskblock_t file_dir_block = virtual_disk[current_dir_index];
  direntry_t *file_dir = &file_dir_block.dir.entrylist[next_entry];
  // this needs to be added to a similar function like next_unallocated_block to create new dir blocks as more files added.
  file_dir_block.dir.next_entry++;

  //set the properties of the dir entry
  memcpy(file_dir->name, "file.txt", strlen("file.txt"));
  file_dir->is_dir = FALSE;
  file_dir->unused = FALSE;
  file_dir->first_block = block_index;

  // update the dirblock
  write_block(&file_dir_block, current_dir_index, 'd');
  // write_block(&file_dir_block, current_dir_index, 'd');

  //allocate a new block for the subdir
  int sub_dir_block_index = next_unallocated_block();
  FAT[sub_dir_block_index] = 0;
  diskblock_t sub_dir_block = virtual_disk[sub_dir_block_index];

  //clear it
  init_dir_block(&sub_dir_block);
  write_block(&sub_dir_block, sub_dir_block_index, 'd');

  //find a place for it in the directory
  next_entry = next_unallocated_dir_entry();
  diskblock_t sub_dir_dir_block = virtual_disk[current_dir_index];
  direntry_t *sub_dir_dir = &sub_dir_dir_block.dir.entrylist[next_entry];
  // this needs to be added to a similar function like next_unallocated_block to create new dir blocks as more files added.
  sub_dir_dir_block.dir.next_entry++;

  //set the properties of the dir entry
  memcpy(sub_dir_dir->name, "directory", strlen("a new directory"));
  sub_dir_dir->first_block = sub_dir_block_index;
  sub_dir_dir->is_dir = TRUE;
  sub_dir_dir->unused = FALSE;

  // update the dirblock
  write_block(&sub_dir_dir_block, current_dir_index, 'd');

  print_directory_structure(root_dir_index, 0);
}
