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
direntry_t *current_dir = NULL; //use this to track the location of the first block in a dir block chain
fatentry_t current_dir_index = 0;

void write_disk(const char *file_name)
{
  // printf("write_disk> %s\n", virtual_disk[0].data);
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

// This isn't used, seems to move data from the dist into the block
// void read_block(diskblock_t *block, int block_address, char type, int print)
// {
//   memmove(block->data, virtual_disk[block_address].data, BLOCKSIZE);
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
  blank_entry->first_block = root_dir_index;
  current_dir = blank_entry;
}

// clean out any junk memory to give a fresh new data block
void init_block(diskblock_t *block)
{
  for (int i = 0; i < BLOCKSIZE; i++) {
    block->data[i] = '\0';
  }
}

// clean a block and give it a dir structure
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

// updates the fat and returns the next place a block can be placed.
int next_unallocated_block()
{
  for(int i = 0; i < MAXBLOCKS; i++){
    if (FAT[i] == UNUSED){
      FAT[i] = ENDOFCHAIN;
      copy_fat(FAT);
      return i;
    }
  }
  return -1; //disk is full
}

// this will return the first free slot in the dir, moving to a new dir block if needed
int next_unallocated_dir_entry(){
  // NOT WORKING!!!
  // look for empty slots
  // int original_dir_index = current_dir_index;
  // int current_dir_index = current_dir->first_block;
  // while(1){
  //   for(int i = 0; i < DIRENTRYCOUNT; i++){
  //     if (strlen(virtual_disk[current_dir_index].dir.entrylist[i].name) == 0){
  //       printf("%d\n", virtual_disk[current_dir_index].dir.entrylist[i].unused);
  //       return i;
  //     }
  //   }
  //   if(FAT[current_dir_index] == ENDOFCHAIN) break;
  //   current_dir_index = FAT[current_dir_index];
  // }
  // current_dir_index = original_dir_index;

  int next_entry = virtual_disk[current_dir_index].dir.next_entry;
  if(next_entry > DIRENTRYCOUNT - 1){
    //revert 'next_entry' value
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
    // current_dir = blank_entry;
  }
  return virtual_disk[current_dir_index].dir.next_entry++;;
}

// returns where a file is in the current dir
int file_entry_index(char *filename){
  current_dir_index = current_dir->first_block;
  while(1){
    for(int i = 0; i < DIRENTRYCOUNT; i++){
      if (memcmp(virtual_disk[current_dir_index].dir.entrylist[i].name, filename, strlen(filename) + 1) == 0)
        return i;
    }

    if(FAT[current_dir_index] == ENDOFCHAIN) break;
    current_dir_index = FAT[current_dir_index];
  }
  return -1;
}

// used when opening in append mode to go to the end of the file
void move_pos_to_end(my_file_t *file){
  //last block
  while(1) {
    if(FAT[file->blockno] == ENDOFCHAIN) break;
    file->blockno = FAT[file->blockno];
  }

  // last character
  file->buffer = virtual_disk[file->blockno];
  while(1) {
    if(file->buffer.data[file->pos+++1] == '\0') break;
  }

  // if there is no content it needs to be set back to zero
  if (file->buffer.data[0] == '\0') file->pos = 0;
}

// this adds a block at an index to the hierarchy, data or dir
int add_block_to_directory(int index, char *name, int is_dir) {
  //find a place for it in the directory
  int entry_index = next_unallocated_dir_entry();

  // find the block of the current directory
  diskblock_t file_dir_block = virtual_disk[current_dir_index];

  // get the direntry for where the file is going to be stored
  direntry_t *file_dir = &file_dir_block.dir.entrylist[entry_index];

  //set the properties of the dir entry
  memcpy(file_dir->name, name, strlen(name));
  file_dir->is_dir = is_dir;
  file_dir->unused = FALSE;
  file_dir->first_block = index;

  // save the dirblock to the disk
  write_block(&file_dir_block, current_dir_index, 'd');

  return entry_index;
}

diskblock_t create_block(int index, int type) {
  // load the space to be used from the disk
  diskblock_t block = virtual_disk[index];

  //clear it
  if(type == DIR) init_dir_block(&block);
  if(type == DATA) init_block(&block);
  // // give it some data
  // memcpy(block.data, "data", strlen("data"));

  //save the block to the disk
  write_block(&block, index, 'd'); //writing as data seems to work even for dir

  return block;
}

my_file_t *myfopen(char *path, char *mode)
{
  // only cd if we have a path with many levels
  if(number_of_entries_in_path(path_to_array(path)) > 1){
    mychdir(path);
  }

  char filename[MAXNAME];
  strcpy(filename, last_entry_in_path(path_to_array(path)));
  strcat(filename, "\0");

  int dir_entry_index = file_entry_index(filename);

  // if it doesn't exist then create it
  if(dir_entry_index == -1 && strncmp(mode, "r", 1) != 0){
    printf("File did not exist. Creating new file: %s\n", path);

    // create a block for it on the disk
    int block_index = next_unallocated_block();
    create_block(block_index, DATA);

    // create a dir entry for it in the current directory
    dir_entry_index = add_block_to_directory(block_index, filename, FALSE);
  }

  // load up it's dir entry
  direntry_t dir_entry = virtual_disk[current_dir_index].dir.entrylist[dir_entry_index];

  // create a file object to return
  my_file_t *file = malloc(sizeof(my_file_t));
  file->pos = 0;
  file->writing = 0;
  memcpy(file->mode, mode, strlen(mode));
  file->blockno = dir_entry.first_block;
  file->buffer = virtual_disk[dir_entry.first_block];

  // move to the end if in append mode
  if(strncmp(file->mode, "a", 1) == 0){
    move_pos_to_end(file);
  }

  return file;
}

char myfgetc(my_file_t *file)
{
  int position = file->pos;
  file->pos++;
  if ((file->buffer.data[position] == '\0') && (FAT[file->blockno] != ENDOFCHAIN)){
    file->pos = 1;
    file->blockno = FAT[file->blockno];
    file->buffer = virtual_disk[file->blockno];
    return file->buffer.data[file->pos - 1];
  }
  else if ((file->pos > BLOCKSIZE) && FAT[file->blockno] == ENDOFCHAIN) {
    return EOF;
  }
  else {
    return file->buffer.data[position];
  }
}

int myfputc(char character, my_file_t *file)
{
  if (strncmp(file->mode, "r", 1) == 0) return 1;
  if (file->pos == BLOCKSIZE){
    file->pos = 0;
    if(FAT[file->blockno] == ENDOFCHAIN) {
      int block_index = next_unallocated_block();
      FAT[file->blockno] = block_index;
      copy_fat(FAT);

      file->blockno = block_index;
      file->buffer = create_block(block_index, DATA);
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

void print_directory_structure(int current_dir_block, int indent){
  char string[] = "\0\0\0\0\0\0"; //up to five levels
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

    if(FAT[current_dir_block] == ENDOFCHAIN) break;
    current_dir_block = FAT[current_dir_block];
  }
}

// keeping this in mind for when functions are needed to create dirs too
void manually_create_file_and_directory(){
  // create a new block for the file
  int block_index = next_unallocated_block();
  create_block(block_index, DATA);
  add_block_to_directory(block_index, "file.txt", FALSE);

  //allocate a new block for the subdir
  int sub_dir_block_index = next_unallocated_block();
  create_block(sub_dir_block_index, DIR);
  add_block_to_directory(sub_dir_block_index, "directory", TRUE);

  // 'cd' to the new sub dir
  current_dir_index = sub_dir_block_index;
  current_dir->first_block = sub_dir_block_index;

  print_directory_structure(root_dir_index, 0);
}

void mymkdir(char *path) {
  int initial_current_dir_index = current_dir_index;
  int initial_current_dir_first_block = current_dir->first_block;

  // if the path is an absolute path switch to root before creating
  // CONFIRM IF THIS IS REQUIRED
  if (path[0] == '/') {
    current_dir_index = root_dir_index;
    current_dir->first_block = root_dir_index;
  }

  // not sure why I can't just use path here, life is too short
  char str[strlen(path)];
  strcpy(str, path);

  char *dir_name = strtok(str, "/");

  while (dir_name) {
      //allocate a new block for the subdir
      int sub_dir_block_index = next_unallocated_block();
      create_block(sub_dir_block_index, DIR);
      add_block_to_directory(sub_dir_block_index, dir_name, TRUE);

      // 'cd' to the new sub dir
      current_dir_index = sub_dir_block_index;
      current_dir->first_block = sub_dir_block_index;

      //set the name for the next level
      dir_name = strtok(NULL, "/");
  }

  // move back to the original dir index
  current_dir_index = initial_current_dir_index;
  current_dir->first_block = initial_current_dir_first_block;
}

void print_dir_list(char **list){
  for (int i = 0; i< 10; i++){
    if(strcmp(list[i],"ENDOFDIR") == 0) break;
    printf("%s\n", list[i]);
  }
}

char **mylistdir(char *path) {
  int initial_current_dir_index = current_dir_index;
  int initial_current_dir_first_block = current_dir->first_block;

  if (strcmp(path, "root") == 0 || strcmp(path, "") == 0){
    printf("Whoa, listing root!\n");
    current_dir_index = root_dir_index;
  } else {
    int location = dir_index_for_path(path);
    if (location == -1) {
      // create an array in a mental way just to get out of here
      char **file_list = malloc(1 * MAXNAME * sizeof(char));
      for (int i = 0; i < 2; i++)
          file_list[i] = malloc(sizeof(**file_list) * 30);
      strcpy(file_list[0], "Directory does not exist.");
      strcpy(file_list[1], "ENDOFDIR");
      return file_list;
    }
    current_dir_index = location;
  }

  // maximum of ten entries printed
  int max_entries = 10;
  char **file_list = malloc(max_entries * MAXNAME * sizeof(char));
  for (int i = 0; i < max_entries; i++)
      file_list[i] = malloc(sizeof(**file_list) * 30);

  int print_count = 0;
  while(1){
    for(int i = 0; i < DIRENTRYCOUNT; i++){
      if(strlen(virtual_disk[current_dir_index].dir.entrylist[i].name) != 0){
        // printf("priint: %s\n", virtual_disk[current_dir_index].dir.entrylist[i].name);
        strcpy(file_list[print_count], virtual_disk[current_dir_index].dir.entrylist[i].name);
        print_count++;

        if (print_count >= max_entries) break;
      }
    }
    if (print_count >= max_entries) break;

    if(FAT[current_dir_index] == ENDOFCHAIN) break;
    current_dir_index = FAT[current_dir_index];
  }

  // reset the current dir to its original state
  current_dir_index = initial_current_dir_index;
  current_dir->first_block = initial_current_dir_first_block;

  char **file_list_final = malloc((print_count + 1) * MAXNAME * sizeof(char));
  for (int i = 0; i < print_count; i++){
    file_list_final[i] = file_list[i];
  }
  file_list_final[print_count] = "ENDOFDIR";

  return file_list_final;
}

int dir_index_for_path(char *path){
  int initial_current_dir_index = current_dir_index;
  int initial_current_dir_first_block = current_dir->first_block;

  if (path[0] == '/') {
    current_dir_index = root_dir_index;
    current_dir->first_block = root_dir_index;
  }

  char str[strlen(path)];
  strcpy(str, path);

  int location;
  int next_dir = current_dir_index;
  char *dir_name = strtok(str, "/");
  while (dir_name) {
      location = file_entry_index(dir_name);
      if (location == - 1) return -1;

      // unless it's a file...
      if (virtual_disk[current_dir_index].dir.entrylist[location].is_dir == 0) break;
      next_dir = virtual_disk[current_dir_index].dir.entrylist[location].first_block;

      current_dir_index = next_dir;
      current_dir->first_block = next_dir;

      dir_name = strtok(NULL, "/");
      if (dir_name == NULL) break;
  }

  // move back to the original dir index
  current_dir_index = initial_current_dir_index;
  current_dir->first_block = initial_current_dir_first_block;

  return next_dir;
}

char **path_to_array(char *path) {
  int max_entries = 10;
  char **file_list = malloc(max_entries * 256 * sizeof(char));
  for (int i = 0; i < max_entries; i++)
      file_list[i] = malloc(sizeof(**file_list) * 30);

  char str[strlen(path)];
  strcpy(str, path);

  char *dir_name = strtok(str, "/");
  file_list[0] = dir_name;
  int count = 1;
  while (dir_name) {
      dir_name = strtok(NULL, "/");
      if (dir_name == NULL) break;
      file_list[count] = dir_name;
      count++;
  }

  return file_list;
}

char *last_entry_in_path(char **path){
  int count = 0;
  for (int i = 0; i < 10; i++){
    if(strlen(path[i]) == 0) break;
    count++;
  }
  return path[count-1];
}

int number_of_entries_in_path(char **path){
  int count = 0;
  for (int i = 0; i < 10; i++){
    if(strlen(path[i]) == 0) break;
    count++;
  }
  return count;
}

void mychdir(char *path){
  if (strcmp(path, "root") == 0 || strcmp(path, "") == 0){
    printf("Returning to root...\n");
    current_dir_index = root_dir_index;
    current_dir->first_block = root_dir_index;
    return;
  }

  if (path[0] == '/') {
    current_dir_index = root_dir_index;
    current_dir->first_block = root_dir_index;
  }

  int new_dir = dir_index_for_path(path);
  if (new_dir == -1) return;
  current_dir_index = new_dir;
  current_dir->first_block = new_dir;
}

// removes a file given it's path
void myremove(char *path){
  int initial_current_dir_index = current_dir_index;
  int initial_current_dir_first_block = current_dir->first_block;

  mychdir(path);

  // set the file name
  char filename[MAXNAME];
  strcpy(filename, last_entry_in_path(path_to_array(path)));
  strcat(filename, "\0");

  int location = file_entry_index(filename);

  if (location == -1) {
    printf("File not found.\n");
    return;
  }

  // clear the fat entries
  int block_index = virtual_disk[current_dir_index].dir.entrylist[location].first_block;
  int next_block_index;
  while(1){
    next_block_index = FAT[block_index];
    FAT[block_index] = UNUSED;
    if (next_block_index == ENDOFCHAIN) break;
    block_index = next_block_index;
  }
  copy_fat(FAT);

  // clear the dir_entry
  direntry_t *dir_entry = &virtual_disk[current_dir_index].dir.entrylist[location];
  dir_entry->first_block = 0;
  dir_entry->unused = 1;
  int length = strlen(dir_entry->name);
  for (int i = 0; i < length; i++){
    dir_entry->name[i] = '\0';
  }

  current_dir_index = initial_current_dir_index;
  current_dir->first_block = initial_current_dir_first_block;
}

void current(){
  printf("******\n");
  printf("current_dir_index: %d\n", current_dir_index);
  printf("first: %d\n", current_dir->first_block);
  printf("******\n");
}
