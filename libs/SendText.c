#include <FVWMconfig.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <shims.h>

/************************************************************************
 *
 * Sends arbitrary text to fvwm
 *
 ***********************************************************************/
void SendText(int *fd,char *message,unsigned long window)
{
  int w;

  if(message != NULL)
    {
      chkwrite(fd[0],&window, sizeof(unsigned long));

      w=strlen(message);
      chkwrite(fd[0],&w,sizeof(int));
      chkwrite(fd[0],message,w);

      /* keep going */
      w = 1;
      chkwrite(fd[0],&w,sizeof(int));
    }
}
