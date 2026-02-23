#include <FVWMconfig.h>
// 2017-04-02 DWF
// Shim for unchecked write calls.

#include <shims.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <X11/XKBlib.h>

void chkwrite (int fd, const void *buf, size_t count) {
  ssize_t ret = write(fd, buf, count);
  if (ret < 0)
    perror ("write");
  else if (ret < count)
    fprintf (stderr, "write: short write (ignored)\n");
}

void chkread (int fd, void *buf, size_t count) {
  ssize_t ret = read(fd, buf, count);
  if (ret < 0)
    perror ("read");
}

void chksystem (const char *command) {
  if (command != NULL) {
    int ret = system(command);
    if (ret == -1)
      fprintf (stderr, "system: child process could not be created (ignored):\n %s\n", command);
    else {
      ret = WEXITSTATUS(ret);
      if (ret != 0)
        fprintf (stderr, "system: child exit status = %d:\n %s\n", ret, command);
    }
  }
}

void chkfgets (char *s, int size, FILE *stream) {
  char *ret = fgets(s, size, stream);
  if (ret == NULL)
    fprintf (stderr, "fgets: error (ignored).\n");
}

void chkfscanf (FILE *stream, const char *format, void *u_mad) {
  int ret = fscanf (stream, format, u_mad);
  if (ret != 1)
    fprintf (stderr, "fscanf: failed (ignored)\n");
}

void chkfread (void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t ret = fread (ptr, size, nmemb, stream);
  if (ret < nmemb)
    fprintf (stderr, "fread: short read (ignored)\n");
}

// Voiding the return isn't enough to silence the GCC warning.
void ignfread (void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t ret __attribute__((unused));
  ret = fread (ptr, size, nmemb, stream);
}

KeySym fixKeycodeToKeysym (Display *display, KeyCode keycode, int index) {
  return XkbKeycodeToKeysym (display, keycode, 0, index);
}
