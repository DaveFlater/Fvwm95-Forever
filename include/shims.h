// 2017-04-02 DWF
// Shims for chronically unchecked lib calls and deprecated functions.

#ifndef SHIMH
#define SHIMH

#include <stdio.h>
#include <X11/XKBlib.h>

// Check return values, print warnings, but return anyway.
void chkwrite (int fd, const void *buf, size_t count);
void chkread (int fd, void *buf, size_t count);
void chksystem (const char *command);
void chkfgets (char *s, int size, FILE *stream);
void chkfscanf (FILE *stream, const char *format, void *u_mad);
void chkfread (void *ptr, size_t size, size_t nmemb, FILE *stream);

// Just make the stupid warnings go away.
void ignfread (void *ptr, size_t size, size_t nmemb, FILE *stream);

// Deal with deprecated function.
KeySym fixKeycodeToKeysym (Display *display, KeyCode keycode, int index);

#endif
