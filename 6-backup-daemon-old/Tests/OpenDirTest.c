#include <dirent.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char* argv[]) {
  assert(argc == 2);
  const char* path = argv[1];

  DIR* dir =opendir(path);
  assert(dir);
  getchar();
  closedir(dir);
}