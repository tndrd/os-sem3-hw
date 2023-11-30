#include <linux/limits.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define PATH "tmp/"
#define SUBDIR PATH "subdir"

void* foo(void* args) {
  const char* path = PATH;
  const char* subdir = SUBDIR;
  int fd, wd;

  if ((fd = inotify_init()) < 0) {
    perror("inotify_init()");
    return NULL;
  }

  uint32_t mask = IN_CREATE | IN_MODIFY | IN_DELETE | IN_DELETE_SELF;
  if ((wd = inotify_add_watch(
           fd, path, mask)) < 0) {
    perror("inotify_add_watch()");
    return NULL;
  }
  /*
  if ((wd = inotify_add_watch(fd, subdir, IN_DELETE | IN_DELETE_SELF)) <
      0) {
    perror("inotify_add_watch()");
    return 1;
  }
  */
  char buffer[(sizeof(struct inotify_event) + NAME_MAX + 1)];
  const struct inotify_event* event;

  for (;;) {
    int len = read(fd, buffer, sizeof(buffer));
    printf("Read %d\n", len);

    if (len < 0) {
      perror("read()");
      return NULL;
    }

    for (char* ptr = buffer; ptr < buffer + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event*)ptr;

      if (event->mask & IN_DELETE) printf("%s: Deleted\n", event->name);
      if (event->mask & IN_DELETE_SELF)
        printf("%s: Self-deleted\n", event->name);
    }
  }

  return NULL;
}

int main() {
  pthread_t thread;

  int ret = pthread_create(&thread, NULL, foo, NULL);
  if (ret != 0) {
    errno = ret;
    perror("pthread_create()");
  }

  ret = pthread_join(thread, NULL);

  if (ret != 0) {
    errno = ret;
    perror("pthread_join()");
  }
}