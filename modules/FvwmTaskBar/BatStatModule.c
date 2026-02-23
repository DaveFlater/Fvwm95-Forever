#include <FVWMconfig.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <shims.h>

#include <X11/X.h>
#include <X11/xpm.h>

#include <fvwm/fvwmlib.h>

#include "GoodyLoadable.h"

#define PROCAPMFILE             "/proc/apm"
#define BATCHARGEFULLFILE       "/sys/class/power_supply/BAT0/charge_full"
#define BATCHARGENOWFILE        "/sys/class/power_supply/BAT0/charge_now"
#define BATSTATUSFILE           "/sys/class/power_supply/BAT0/status"
#define ACONLINEFILE            "/sys/class/power_supply/AC0/online"

#define FULL_ICON               "apm-full.xpm"
#define HALF_ICON               "apm-half.xpm"
#define EMPTY_ICON              "apm-empty.xpm"
#define ALERT_ICON              "apm-alert.xpm"
#define UNKNOWN_ICON            "apm-unknown.xpm"
#define LOADING_ICON            "apm-loading.xpm"
#define ONLINE_ICON             "apm-online.xpm"

typedef struct IconDef {
	char *        Name;
	Pixmap        Pix;
	Pixmap        Mask;
	XpmAttributes Attr;
} IconDef_t;

typedef struct BatStatInfo BatStatInfo_t;
struct BatStatInfo {
	char *id;
	/* other stuff */

        // Legacy system:
        //   fd is open file descriptor for APM battery information
        //   BatteryChargeFull is 0
        // New system:
        //   fd is -1
        //   BatteryChargeFull is nonzero (used to calculate the percent)
        // Both:
        //   ACStatus is 0 or 1
        //   BatteryPercent is 0..100
        //   BatteryStatus is
        //     0=Full, 1=Half, 2=Low, other=Empty if ACStatus==0
        //     3=Charging, other=Online if ACStatus==1
        //   BatteryStatusText is the status description

	int fd;
	char ACStatus;
	char BatteryStatus;
	char BatteryPercent;
        unsigned long BatteryChargeFull;
        #define BatteryStatusTextLen 15
        char BatteryStatusText[BatteryStatusTextLen];

        IconDef_t FullIcon;
        IconDef_t HalfIcon;
        IconDef_t EmptyIcon;
        IconDef_t AlertIcon;
        IconDef_t LoadingIcon;
        IconDef_t OnlineIcon;
        IconDef_t UnknownIcon;
        char * IconLocation;

	int offset;
	int visible;
	int show; /* whether to show the icon */
	time_t lastchecked;
	char *lock;
};

static void * BatStatModule_Init(char *id);
static int    BatStatModule_ParseResource(
		     BatStatInfo_t *mif,
		     char *tline,
		     char *Module,
		     int Clength);
static void  BatStatModule_Load(
		     BatStatInfo_t *mif,
		     Display *dpy,
		     Drawable win);
static void  BatStatModule_Draw(
		     BatStatInfo_t *mif,
		     Display *dpy,
		     Window win);
static int  BatStatModule_SeeMouse(
		     BatStatInfo_t *mif,
		     int x,
		     int y);
static void  BatStatModule_CreateIconTipWindow_(
		     BatStatInfo_t *mif);
static void  BatStatModule_IconClick(
		     BatStatInfo_t *mif,
		     XEvent event);

extern int win_width, stwin_width;
extern int RowHeight;
extern GC statusgc;

/*
*
* Purely local functions
*
*/

/* Might better be part of the FvwmTaskbar environment */

// 2017-04-01 DWF
// Fixed to use findIconFile instead of only working with a single directory.

static int LoadIcon (Display *dpy, Drawable win, char *iconPath,
		     char *iconFile, IconDef_t *Icon) {
  int rv = 0;
  char *path;

  path = findIconFile (iconFile, iconPath, R_OK);
  if (path) {
    if (XpmReadFileToPixmap (dpy, win, path, &(Icon->Pix), &(Icon->Mask),
			     &(Icon->Attr)) == XpmSuccess) {
      rv = 1;
      Icon->Name = path;
    } else {
      fprintf (stderr, "BatStat LoadIcon: failure reading %s\n", path);
      free (path);
    }
  } else
    fprintf (stderr, "BatStat LoadIcon: failed to find %s in path\n",
	     iconFile);
  return rv;
}

/* Figure out what Icon to use with respect to the battery status */

static IconDef_t * GetActualIcon( BatStatInfo_t * mif ) {

        IconDef_t * TheIcon;

	if( mif->ACStatus == 0x01 ) {
	  /* online */
	  if( mif->BatteryStatus == 0x03 ) {
	    /* Charging */
	    TheIcon = &(mif->LoadingIcon);
	  } else {
	    /* Simply online */
	    TheIcon = &(mif->OnlineIcon);
	  }
	} else {
	  /* offline */
	  switch( mif->BatteryStatus ) {
	    case 0 : /* High */
	      TheIcon = &(mif->FullIcon);
	      break;
	    case 1 : /* Low */
	      TheIcon = &(mif->HalfIcon);
	      break;
	    case 2 : /* Critical */
	      TheIcon = &(mif->AlertIcon);
	      break;
	    default : /* All others */
	      TheIcon = &(mif->EmptyIcon);
	      break;
	  }
	}
	return TheIcon;
}

/*
*
* Read information from OS about battery status
* Current implementation (Linux) Directly reads /proc/apm
* Hence support for the apm is required
*
* DWF 2014-01-25:  Added support for /sys/class/power_supply.
*
* returns 1 if successful, 0 if not
*
*/

static int  ReadBatteryInfo(BatStatInfo_t *mif) {

        char Buffer[100];
        FILE *fp;

	if (mif == NULL) return 0;

        if (mif->BatteryChargeFull==0 && mif->fd==-1) {
          // First run.  Attempt to use the new system.
          fp = fopen(BATCHARGEFULLFILE,"r");
          if (fp != NULL) {
            chkfscanf(fp, "%lu", &mif->BatteryChargeFull);
            fclose(fp);
          }
        }

        if (mif->BatteryChargeFull==0 && mif->fd==-1) {
          // That didn't work.  Try the legacy system.
	  mif->fd = open(PROCAPMFILE, O_RDONLY);
  	  if (mif->fd == -1)
	    return 0;   // Neither works.
	}

        if (mif->BatteryChargeFull) {
          // Update status using the new system.
          unsigned long charge_now = 0;
          fp = fopen(BATCHARGENOWFILE,"r");
          if (fp != NULL) {
            chkfscanf(fp, "%lu", &charge_now);
            fclose(fp);
          }
          mif->BatteryPercent = 100 * charge_now / mif->BatteryChargeFull;
          mif->BatteryStatusText[0] = '\0';
          fp = fopen(BATSTATUSFILE,"r");
          if (fp != NULL) {
            int i;
            chkfgets(mif->BatteryStatusText, BatteryStatusTextLen, fp);
            fclose(fp);
            for (i=0; i<BatteryStatusTextLen; ++i) {
              if (mif->BatteryStatusText[i] == '\n') {
                mif->BatteryStatusText[i] = '\0';
                break;
              }
            }
          }
          fp = fopen(ACONLINEFILE,"r");
          if (fp != NULL) {
            unsigned temp;
            chkfscanf(fp, "%u", &temp);
            fclose(fp);
            mif->ACStatus = temp;
          }
          // Fake up BatteryStatus.
          if (mif->ACStatus) {
            if (mif->BatteryPercent >= 100)
              mif->BatteryStatus = 0;
            else
              mif->BatteryStatus = 3;
          } else {
            if (mif->BatteryPercent > 50)
              mif->BatteryStatus = 0;
            else if (mif->BatteryPercent > 7)
              mif->BatteryStatus = 1;
            else if (mif->BatteryPercent > 2)
              mif->BatteryStatus = 2;
            else
              mif->BatteryStatus = 3;
          }
        } else {
          // Update status using the legacy system.
          int ACStatus;
          int BatStatus;
          long BatPercent;

	  lseek( mif->fd, 0, SEEK_SET );

          /* Information about the layout and meaning of bits
             can be found in the apm_bios.c source file in the kernel
          */

	  chkread( mif->fd, Buffer, 100 );

	  { int arg;

	    ACStatus = BatStatus = BatPercent = 0;
	    /* skip driver version
	       skip apm-bios version
	       skip apm-bios flags
	    */
	    arg = sscanf( Buffer, "%*s %*s %*s %x %x %*s %ld",
	          &ACStatus, &BatStatus, &BatPercent );
	    if( arg < 3 )
	      fprintf( stderr, "Bad conversion %d\n", arg );
          }

	  mif->ACStatus = ACStatus;
	  mif->BatteryStatus = BatStatus;
	  mif->BatteryPercent = BatPercent;

          // Fake up BatteryStatusText (moved from old CreateIconTipWindow)
          if (ACStatus) {
            if (BatStatus == 3)
              strcpy (mif->BatteryStatusText, "Charging");
            else
              strcpy (mif->BatteryStatusText, "Online");
          } else {
            strcpy (mif->BatteryStatusText, "Discharging");
          }
	}

	return 1;
}

/*
*
*       Goody Module interface functions
*
*/

static void * BatStatModule_Init(char *id) {

	BatStatInfo_t *mif;

#ifdef __DEBUG__
	fprintf(stderr, "FvwmTaskBar.BatStatModule.Init(\"%s\")\n", id);
	fflush( stderr);
#endif

	mif = NULL;
	mif = (BatStatInfo_t *) calloc(1, sizeof(BatStatInfo_t));
	if(mif == NULL) {
	  perror("FvwmTaskBar.BatStatModule.Init()");
	  return NULL;
	}

	mif->id = id;
	/* Initialize to Unknown status */
	mif->fd = -1;
	mif->ACStatus = -1;
	mif->BatteryStatus = -1;
	mif->BatteryPercent = -1;
	mif->BatteryStatusText[0] = '\0';
        mif->BatteryChargeFull = 0;
	mif->show = 1; /* show up by default */
	mif->lastchecked = 0;
	mif->lock = NULL;
	mif->IconLocation = FVWM_ICONDIR; // 2017-03-30 DWF: was NULL

	/* Call to test if apm is available */
	if(!ReadBatteryInfo(mif)) {
	  /* No apm */
	  free( mif );
	  mif = NULL;
	}
	return mif;
}

static int  BatStatModule_ParseResource(BatStatInfo_t *mif, char *tline,
                                 char *Module, int Clength) {
#ifdef __DEBUG__
	fprintf(stderr,
	        "FvwmTaskBar.BatStatModule.ParseResource(\"%s\",\"%s\",*)\n",
	       mif->id, tline);
	fflush(stderr);
#endif

	if (mif == NULL) return 0;

	if(strncasecmp(tline,CatString3(Module,
		      "ModuleIconPath",mif->id),
		      Clength+14+strlen(mif->id))==0) {
          char * Runner = &tline[Clength+15+strlen(mif->id)];

          while( isspace( *Runner ) && *Runner )
            Runner ++;

	  if( *Runner ) {
	    char * EOLPtr = Runner;

	    while( ! isspace( *EOLPtr ) && *EOLPtr )
	      EOLPtr ++;

            *EOLPtr = '\0'; /* Could already be '\0' */

	    /* Location of icons */
	    mif->IconLocation = strdup( Runner );
#ifdef __DEBUG__
	    fprintf( stderr, "Icon Location %s\n", mif->IconLocation );
	    fflush(stderr);
#endif
	  }
	  return(1);
	} else
	  return 0;
}

static void  BatStatModule_Load(
                BatStatInfo_t *mif, Display *dpy, Drawable win) {

#ifdef __DEBUG__
	fprintf(stderr, "FvwmTaskBar.BatStatModule.Load()\n");
	fflush( stderr );
#endif

        /* Load icons */

        if( ! LoadIcon( dpy, win, mif->IconLocation, FULL_ICON,
                      &(mif->FullIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, mif->IconLocation, HALF_ICON,
                      &(mif->HalfIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, mif->IconLocation, EMPTY_ICON,
                      &(mif->EmptyIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, mif->IconLocation, ALERT_ICON,
                      &(mif->AlertIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, mif->IconLocation, LOADING_ICON,
                      &(mif->LoadingIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, mif->IconLocation, ONLINE_ICON,
                      &(mif->OnlineIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, mif->IconLocation, UNKNOWN_ICON,
                      &(mif->UnknownIcon) ) ) {
	  mif->visible = False;
	  return;
        }

	/* Icons loaded sucessfully */
	mif->visible = True;
	if ((mif->LoadingIcon.Attr.valuemask & XpmSize) == 0) {
	  mif->LoadingIcon.Attr.width = 16;
	  mif->LoadingIcon.Attr.height = 16;
	}

	mif->offset = icons_offset;
	icons_offset += mif->LoadingIcon.Attr.width+2;
}

static void  BatStatModule_Draw(BatStatInfo_t *mif, Display *dpy, Window win) {

	XGCValues gcv;
	time_t now;
	IconDef_t * TheIcon;
	unsigned long  gcm =
	        GCClipMask | GCClipXOrigin | GCClipYOrigin;

	if (mif == NULL) return;

	now = time(NULL);

	if (mif->visible && mif->show) {

	  if (now-mif->lastchecked > 2) {
	    mif->lastchecked = now;
	    ReadBatteryInfo( mif );
	  }

	  TheIcon = GetActualIcon( mif );

	  /* Whipe icon */
	  XClearArea( dpy, win,
	              win_width - stwin_width+icons_offset+3, 1,
		      TheIcon->Attr.width,
		      RowHeight-2, False);
	  gcv.clip_mask = TheIcon->Mask;
	  gcv.clip_x_origin = (win_width-stwin_width) + icons_offset+3;
	  gcv.clip_y_origin = ((RowHeight - TheIcon->Attr.height) >> 1);

	  XChangeGC(dpy, statusgc, gcm, &gcv);
	  XCopyArea(dpy, TheIcon->Pix, win, statusgc, 0, 0,
		    TheIcon->Attr.width,
		    TheIcon->Attr.height,
		    gcv.clip_x_origin,
		    gcv.clip_y_origin);

	  mif->offset = icons_offset;
	  icons_offset += TheIcon->Attr.width+2;
	}
}

static int  BatStatModule_SeeMouse(BatStatInfo_t *mif, int x, int y) {

	int xl, xr;
	IconDef_t * TheIcon;
  // 2017-04-19 DWF
  // Calibrate x against the offset that was used in the previous function
  // for whatever reason.
  const int xcal = 3;

	if (mif == NULL) return 0;
	if (mif->show == 0) return 0;

	TheIcon = GetActualIcon( mif );

	/* Mouse in icon area ? */
	xl = xcal + win_width - stwin_width + mif->offset;
	xr = xcal + win_width - stwin_width + mif->offset + TheIcon->Attr.width;

	return (x>=xl && x<xr && y>1 && y<RowHeight-2);
}


static void  BatStatModule_CreateIconTipWindow_(BatStatInfo_t *mif) {

        char Buffer[100];

	if (mif == NULL) return;

        sprintf(Buffer, "%s: %d%%", mif->BatteryStatusText,
              mif->BatteryPercent);
	PopupTipWindow(
	      win_width - stwin_width + mif->offset,
	      0,
	      Buffer);
}

static void  BatStatModule_IconClick(BatStatInfo_t *mif, XEvent event) {
        return;
}

struct GoodyLoadable BatStatModuleSymbol = {
  (LoadableInit_f)            &BatStatModule_Init,
  (LoadableParseResource_f)   &BatStatModule_ParseResource,
  (LoadableLoad_f)            &BatStatModule_Load,
  (LoadableDraw_f)            &BatStatModule_Draw,
  (LoadableSeeMouse_f)        &BatStatModule_SeeMouse,
  (LoadableCreateTipWindow_f) &BatStatModule_CreateIconTipWindow_,
  (HandleIconClick_f)         &BatStatModule_IconClick
};
