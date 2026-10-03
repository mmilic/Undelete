/*

    File: phmain.c

    Copyright (C) 1998-2009 Christophe GRENIER <grenier@cgsecurity.org>

    This software is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write the Free Software Foundation, Inc., 51
    Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "phmain.h"

//#define HAVE_NCURSES
//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* unlink, ftruncate */
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <ctype.h>      /* tolower */
#ifdef HAVE_LOCALE_H
#include <locale.h>	/* setlocale */
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
//#include <errno.h>
//#include "types.h"
//#include "common.h"
#include "intrf.h"
#include "fnctdsk.h"
#ifdef HAVE_NCURSES
#include "intrfn.h"
#endif



extern file_enable_t list_file_enable[];
/* //TODO brisi, ovo mi se cini da ne treba
#ifdef HAVE_SIGACTION
static struct sigaction action;
static void sighup_hdlr(int sig);

static void sighup_hdlr(int sig)
{
  if(sig == SIGINT)
    log_critical("SIGINT detected! PhotoRec has been killed.\n");
  else
    log_critical("SIGHUP detected! PhotoRec has been killed.\n");
  log_flush();
  action.sa_handler=SIG_DFL;
  sigaction(sig,&action,NULL);
  kill(0, sig);
}
#endif
*/
int photorecz( char disk, const char *path )
{
  int use_sudo=0;
  int help=0, version=0;
  int create_log=TD_LOG_NONE;
  int testdisk_mode=TESTDISK_O_RDONLY|TESTDISK_O_READAHEAD_32K;
  list_disk_t *list_disk=NULL;
  list_disk_t *element_disk;
  const char *logfile="photorec.log";
  FILE *log_handle=NULL;
  struct ph_options options={
    .paranoid=1,
    .keep_corrupted_file=0,
    .mode_ext2=0,
    .expert=0,
    .lowmem=0,
    .verbose=0,
    .list_file_format=list_file_enable
  };
  struct ph_param params;
  params.recup_dir=NULL;
  params.cmd_device=NULL;
  params.cmd_run=NULL;
  // random (weak is ok) is need for GPT 
  srand(time(NULL));

    //set destination dir path
    params.recup_dir = (char *) MALLOC(strlen(path) + 1 + strlen(DEFAULT_RECUP_DIR) + 1);
    strcpy(params.recup_dir, path);
    //strcat(params.recup_dir, "/");
    //strcat(params.recup_dir, DEFAULT_RECUP_DIR);
                
    //use only one disk, passed as function parameter.  
    char device_cdrom[] = "\\\\.\\C:";
    device_cdrom[strlen(device_cdrom) - 2] = disk;
 
    disk_t *disk_car = file_test_availability_win32(device_cdrom, options.verbose, testdisk_mode);
    if (disk_car == NULL) {
        printf("\nUnable to open file or device %s\n", disk);
        help = 1;
    } else
        list_disk = insert_new_disk(list_disk, disk_car);
   
  /*
  if(version!=0)//TODO ovo ne treba
  {
    printf("\n");
    printf("Version: %s\n", VERSION);
    printf("Compiler: %s\n", get_compiler());
    printf("Compilation date: %s\n", get_compilation_date());
    printf("ext2fs lib: %s, ntfs lib: %s, ewf lib: %s, libjpeg: %s\n",
	td_ext2fs_version(), td_ntfs_version(), td_ewf_version(), td_jpeg_version());
    printf("OS: %s\n" , get_os());
    free(params.recup_dir);
    return 0;
  }
  if(help!=0)//TODO ovo ne treba
  {
    printf("\nUsage: photorec [/log] [/debug] [/d recup_dir] [file.dd|file.e01|device]\n"\
	"       photorec /version\n" \
        "\n" \
        "/log          : create a photorec.log file\n" \
        "/debug        : add debug information\n" \
        "\n" \
        "PhotoRec searches various file formats (JPEG, Office...), it stores them\n" \
        "in recup_dir directory.\n" \
        "\n" \
        "If you have problems with PhotoRec or bug reports, please contact me.\n");
    free(params.recup_dir);
    return 0;
  }
#ifdef ENABLE_DFXML
  xml_set_command_line(argc, argv);
#endif
  if(create_log!=TD_LOG_NONE)
    log_handle=log_open(logfile, create_log);
#ifdef HAVE_SETLOCALE
  if(run_setlocale>0)
  {
    const char *locale;
    locale = setlocale (LC_ALL, "");
    if (locale==NULL) {
      locale = setlocale (LC_ALL, NULL);
      log_error("Failed to set locale, using default '%s'.\n", locale);
    } else {
      log_info("Using locale '%s'.\n", locale);
    }
  }
#endif
  if(create_log!=TD_LOG_NONE && log_handle==NULL)
    log_handle=log_open_default(logfile, create_log);
#ifdef HAVE_NCURSES
  // ncurses need locale for correct unicode support  
  if(start_ncurses("PhotoRec", argv[0]))
  {
    free(params.recup_dir);
    return 1;
  }
  {
    const char*filename=logfile;
    while(create_log!=TD_LOG_NONE && log_handle==NULL)
    {
      filename=ask_log_location(filename);
      if(filename!=NULL)
	log_handle=log_open(filename, create_log);
      else
	create_log=TD_LOG_NONE;
    }
  }
  aff_copy(stdscr);
  wmove(stdscr,5,0);
  wprintw(stdscr, "Disk identification, please wait...\n");
  wrefresh(stdscr);
#endif
  if(log_handle!=NULL)
  {
    time_t my_time;
#ifdef HAVE_DUP2
    dup2(fileno(log_handle),2);
#endif
    my_time=time(NULL);
    log_info("\n\n%s",ctime(&my_time));
    log_info("Command line: PhotoRec");
    for(i=1;i<argc;i++)
      log_info(" %s", argv[i]);
    log_info("\n\n");
    log_flush();
  }
  log_info("PhotoRec %s, Data Recovery Utility, %s\nChristophe GRENIER <grenier@cgsecurity.org>\nhttp://www.cgsecurity.org\n", VERSION, TESTDISKDATE);
  log_info("OS: %s\n" , get_os());
  log_info("Compiler: %s\n", get_compiler());
  log_info("Compilation date: %s\n", get_compilation_date());
  log_info("ext2fs lib: %s, ntfs lib: %s, ewf lib: %s, libjpeg: %s\n",
      td_ext2fs_version(), td_ntfs_version(), td_ewf_version(), td_jpeg_version());
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(DJGPP)
#else
#ifdef HAVE_GETEUID
  if(geteuid()!=0)
  {
    log_warning("User is not root!\n");
  }
#endif
#endif
  screen_buffer_reset();
 */ // Scan for available device only if no device or image has been supplied in parameter  
  if(list_disk==NULL)
    list_disk=hd_parse(list_disk, options.verbose, testdisk_mode);
  hd_update_all_geometry(list_disk, options.verbose);//HINT ne znam cemu sluzi?!
  //Activate the cache, even if photorec has its own  
  for(element_disk=list_disk;element_disk!=NULL;element_disk=element_disk->next)
  {
    element_disk->disk=new_diskcache(element_disk->disk, testdisk_mode);
  }
  // save disk parameters to rapport  
  log_info("Hard disk list\n");
  for(element_disk=list_disk;element_disk!=NULL;element_disk=element_disk->next)
  {
    disk_t *disk=element_disk->disk;
    log_info("%s, sector size=%u", disk->description(disk), disk->sector_size);
    if(disk->model!=NULL)
      log_info(" - %s", disk->model);
    if(disk->serial_no!=NULL)
      log_info(", S/N:%s", disk->serial_no);
    if(disk->fw_rev!=NULL)
      log_info(", FW:%s", disk->fw_rev);
    log_info("\n");
  }
  log_info("\n");
  reset_list_file_enable(options.list_file_format);//HINT ukljucuje ekstenzije koje ce traziti
  file_options_load(options.list_file_format);
  use_sudo=do_curses_photorec(&params, &options, list_disk); //Zoran sledeci korak
#ifdef HAVE_NCURSES
  end_ncurses();
#endif
  delete_list_disk(list_disk);
  log_info("PhotoRec exited normally.\n");
  if(log_close()!=0)
  {
    printf("PhotoRec: Log file corrupted!\n");
  }
  else if(params.cmd_run!=NULL && params.cmd_run[0]!='\0')
  {
    printf("PhotoRec syntax error: %s\n", params.cmd_run);
  }
  else
  {
    printf("PhotoRec exited normally.\n");
  }
#ifdef SUDO_BIN
  if(use_sudo>0)
    run_sudo(argc, argv);
#endif
  free(params.recup_dir);
  return 0;
}
