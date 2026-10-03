/*

    File: pdisksel.c

    Copyright (C) 1998-2008 Christophe GRENIER <grenier@cgsecurity.org>

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
//#define HAVE_NCURSES
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* geteuid */
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include "types.h"
#include "common.h"
#include "intrf.h"
#ifdef HAVE_NCURSES
#include "intrfn.h"
#endif
#include "dir.h"
#include "list.h"
#include "filegen.h"
#include "photorec.h"
#include "sessionp.h"
#include "partauto.h"
#include "log.h"
#include "pdisksel.h"
#include "ppartsel.h"
#include "hidden.h"
#include "hiddenn.h"
#include "nodisk.h"
#include "chgtypen.h"

#ifdef HAVE_NCURSES
#define NBR_DISK_MAX 		(LINES-6-8)
#define INTER_DISK_X		0
#define INTER_DISK_Y		(8+NBR_DISK_MAX)
#define INTER_NOTE_Y		(LINES-4)
#endif

extern const arch_fnct_t arch_none;

static int photorec_disk_selection_cli(struct ph_param *params, struct ph_options *options, const list_disk_t *list_disk, alloc_data_t *list_search_space)
{
  const list_disk_t *element_disk;
  disk_t *disk=NULL;
  for(element_disk=list_disk;element_disk!=NULL;element_disk=element_disk->next)
  {
    if(strcmp(element_disk->disk->device, params->cmd_device)==0)
      disk=element_disk->disk;
  }
  if(disk==NULL)
  {
    log_critical("No disk found\n");
#ifdef HAVE_NCURSES
    return intrf_no_disk_ncurses("PhotoRec");
#else
    return 0;
#endif
  }
  {
    /* disk sector size is now known, fix the sector ranges */
    struct td_list_head *search_walker = NULL;
    td_list_for_each(search_walker, &list_search_space->list)
    {
      alloc_data_t *current_search_space;
      current_search_space=td_list_entry(search_walker, alloc_data_t, list);
      current_search_space->start=current_search_space->start*disk->sector_size;
      current_search_space->end=current_search_space->end*disk->sector_size+disk->sector_size-1;
    }
  }
  autodetect_arch(disk, &arch_none);
  params->disk=disk;
  if(interface_partition_type(disk, options->verbose, &params->cmd_run)==0)
    menu_photorec(params, options, list_search_space);
  return 0;
}

static int photorec_disk_selection_ncurses(struct ph_param *params, struct ph_options *options, const list_disk_t *list_disk, alloc_data_t *list_search_space)
{
  int command;
  int real_key;
  unsigned int menu=0;
  int offset=0;
  int pos_num=0;
  int use_sudo=0;
  const list_disk_t *element_disk;
  const list_disk_t *current_disk=list_disk;

  if(list_disk==NULL)
  {
    log_critical("No disk found\n");
    return 0;
  }
  /* ncurses interface */
     
	  disk_t *disk=current_disk->disk;
	  const int hpa_dco=is_hpa_or_dco(disk);
	  autodetect_arch(disk, &arch_none);
	  params->disk=disk;
	  if((hpa_dco==0 /*|| interface_check_hidden_ncurses(disk, hpa_dco)==0*/) &&
	      (options->expert == 0 ||
	       interface_partition_type(disk, options->verbose, &params->cmd_run)==0))
	    menu_photorec(params, options, list_search_space);
	
}

int do_curses_photorec(struct ph_param *params, struct ph_options *options, const list_disk_t *list_disk)
{
  static alloc_data_t list_search_space={
    .list = TD_LIST_HEAD_INIT(list_search_space.list)
  };
  /*
//ovde je izbaceno ncurse nastavljanje sesije
  */
  //if(params->cmd_device!=NULL && params->cmd_run!=NULL)
    //return photorec_disk_selection_cli(params, options, list_disk, &list_search_space);//HINT ovde je mozda moguce ubaciti direktno opcije
//#ifdef HAVE_NCURSES
  return photorec_disk_selection_ncurses(params, options, list_disk, &list_search_space);
//#else
  //return 0;
//#endif
}
