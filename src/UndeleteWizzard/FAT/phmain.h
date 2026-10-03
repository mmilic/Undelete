/* 
 * File:   phmain.h
 * Author: milic
 *
 * Created on August 8, 2014, 10:41 PM
 */

#ifndef PHMAIN_H
#define	PHMAIN_H

#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include "types.h"
#include <stddef.h>
#include "common.h"
#include "log.h"
#include "dir.h"
#include "filegen.h"
#include "photorec.h"

#include "hdcache.h"
#include "ewf.h"
#include "log.h"
#include "hdaccess.h"
#include "sudo.h"
#include "phcfg.h"
#include "misc.h"
#include "ext2_dir.h"
#include "file_jpg.h"
#include "ntfs_dir.h"
#include "pdisksel.h"
#include "dfxml.h"
//#define HAVE_SYS_STAT_H
#ifdef	__cplusplus
extern "C" {
#endif
int photorecz(char ,const char *);



#ifdef	__cplusplus
}
#endif

#endif	/* PHMAIN_H */

