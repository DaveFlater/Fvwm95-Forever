#include <FVWMconfig.h>
#include <string.h>

#if HAVE_UNAME
#include <sys/utsname.h>

/* return a string indicating the OS type (i.e. "Linux", "SINIX-D", ... ) */
int mygetostype(char *buf, int max)
{
  struct utsname sysname;
  int ret;

  *buf = '\0';
  if ((ret = uname(&sysname)) != -1)
    strncat (buf, sysname.sysname, max-1);
  return ret;
}
#else
int mygetostype(char *buf, int max)
{
  *buf = '\0';
  return -1;
}
#endif
