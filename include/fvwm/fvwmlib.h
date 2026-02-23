#ifndef FVWMLIB_H
#define FVWMLIB_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif
#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif
#ifndef HAVE_STRERROR
char *strerror(int num);
#endif
char *CatString3(char *a, char *b, char *c);
int mygethostname(char *client, int namelen);
int mygetostype(char *buf, int max);
void SendText(int *fd,char *message,unsigned long window);
void SendInfo(int *fd,char *message,unsigned long window);
char *safemalloc(int);
char *findIconFile(char *icon, char *pathlist, int type);
int ReadFvwmPacket(int fd, unsigned long *header, unsigned long **body);
void CopyString(char **dest, char *source);
void sleep_a_little(int n);
int GetFdWidth(void);
void GetConfigLine(int *fd, char **tline);
void SetMessageMask(int *fd, unsigned long mask);
int  envExpand(char *s, int maxstrlen);
char *envDupExpand(const char *s, int extra);

// 2017-03-14 DWF
// This function replaces totally broken logic that appeared twice in
// FvwmWharf and twice in FvwmTaskBar.
// In:
//   buf - a null-terminated string.
//   b   - the index at which to begin searching for an arg.
// Out:
//   b   - the index of the first character of the arg
//   e   - the index of the last character of the arg
// If an arg is not quoted, any whitespace will terminate it.
// If an arg is quoted, the quotes will be excluded from the result,
//   unless it is a zero-length string "".
// If the arg is a zero-length string, b and e will be the index of the
//   close quote or terminating null.
// If the string ends before an arg is found, b and e will be the index of
//   the terminating null.
int DWF_parse_arg (char *buf, int *b, int *e);

typedef struct PictureThing
{
  struct PictureThing *next;
  char *name;
  Pixmap picture;
  Pixmap mask;
  unsigned int depth;
  unsigned int width;
  unsigned int height;
  unsigned int count;
} Picture;

void InitPictureCMap(Display *, Window);
Picture *GetPicture(Display *, Window, char *iconpath, char *pixmappath,char*);
Picture *CachePicture(Display*,Window,char *iconpath,char *pixmappath,char*);
void DestroyPicture(Display *, Picture *p);

XFontStruct *GetFontOrFixed(Display *disp, char *fontname);

#endif
