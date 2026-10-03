/*

    File: ppartsel.c

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
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <ctype.h>      /* isdigit */
#include "types.h"
#include "common.h"
#include "intrf.h"
#include "fnctdsk.h"
#include "dir.h"
#include "list.h"
#include "lang.h"
#include "filegen.h"
#include "photorec.h"
#include "phrecn.h"
#include "log.h"
#include "log_part.h"
#include "hdaccess.h"
#include "ext2grp.h"
#include "pfree_whole.h"
#include "ppartsel.h"
#include "askloc.h"
#include "geometry.h"
#include "addpart.h"
#include "intrfn.h"

extern const arch_fnct_t arch_none;

enum { INIT_SPACE_WHOLE, INIT_SPACE_PREINIT, INIT_SPACE_EXT2_GROUP, INIT_SPACE_EXT2_INODE };

static int spacerange_cmp(const struct td_list_head *a, const struct td_list_head *b)
{
  const alloc_data_t *space_a=td_list_entry_const(a, const alloc_data_t, list);
  const alloc_data_t *space_b=td_list_entry_const(b, const alloc_data_t, list);
  if(space_a->start < space_b->start)
    return -1;
  if(space_a->start > space_b->start)
    return 1;
  return space_a->end - space_b->end;
}

#ifdef HAVE_NCURSES
#define INTER_SELECT_X	0
#define INTER_SELECT_Y	(LINES-2)
#define INTER_SELECT	(LINES-2-7-1)
#endif

void menu_photorec(struct ph_param *params, struct ph_options *options, alloc_data_t*list_search_space)
{
  list_part_t *list_part;
  list_part_t *current_element;
  unsigned int current_element_num;
  unsigned int carve_free_space_only=0;
  unsigned int user_blocksize=0;
  int done=0;
  int mode_init_space=(td_list_empty(&list_search_space->list)?INIT_SPACE_WHOLE:INIT_SPACE_PREINIT);
#ifdef HAVE_NCURSES
  int command;
  unsigned int offset=0;
  unsigned int menu=0;
  static const struct MenuItem menuMain[]=
  {
	{'S',"Search","Start file recovery"},
	{'O',"Options","Modify options"},
	{'F',"File Opt","Modify file options"},
	{'G',"Geometry", "Change disk geometry" },
	{'Q',"Quit","Return to disk selection"},
	{0,NULL,NULL}
  };
#endif
  params->blocksize=0;
  list_part=params->disk->arch->read_part(params->disk,options->verbose,0);
  {
    int insert_error=0;
    partition_t *partition_wd;
    partition_wd=new_whole_disk(params->disk);
    list_part=insert_new_partition(list_part, partition_wd, 0, &insert_error);
    if(insert_error>0)
    {
      free(partition_wd);
    }
  }
  if(list_part==NULL)
    return;
  {
    list_part_t *element;
    for(element=list_part;element!=NULL;element=element->next)
    {
      log_partition(params->disk,element->part);
    }
  }
  if(list_part->next!=NULL)
  {
    current_element_num=1;
    current_element=list_part->next;
  }
  else
  {
    current_element_num=0;
    current_element=list_part;
  }
  while(done==0)
  {
    if(params->cmd_run!=NULL)
    {
      while(params->cmd_run[0]==',')
	params->cmd_run++;
      if(params->cmd_run[0]=='\0')
      {
	part_free_list(list_part);
	return;
      }
      if(strncmp(params->cmd_run,"search",6)==0)
      {
	params->cmd_run+=6;
	if(params->recup_dir==NULL)
	{
	  char *res;
#ifdef HAVE_NCURSES
	  res=ask_location("Please select a destination to save the recovered files.\nDo not choose to write the files to the same partition they were stored on.", "", NULL);
#else
	  res=get_default_location();
#endif
	  if(res!=NULL)
	  {
	    params->recup_dir=(char *)MALLOC(strlen(res)+1+strlen(DEFAULT_RECUP_DIR)+1);
	    strcpy(params->recup_dir,res);
	    strcat(params->recup_dir,"/");
	    strcat(params->recup_dir,DEFAULT_RECUP_DIR);
	    free(res);
	  }
	}
	if(params->recup_dir!=NULL)
	{
	  params->partition=current_element->part;
	  if(mode_init_space==INIT_SPACE_EXT2_GROUP)
	  {
	    params->blocksize=ext2_fix_group(list_search_space, params->disk, params->partition);
	    if(params->blocksize==0)
	      display_message("Not a valid ext2/ext3/ext4 filesystem");
	  }
	  else if(mode_init_space==INIT_SPACE_EXT2_INODE)
	  {
	    params->blocksize=ext2_fix_inode(list_search_space, params->disk, params->partition);
	    if(params->blocksize==0)
	      display_message("Not a valid ext2/ext3/ext4 filesystem");
	  }
	  if(td_list_empty(&list_search_space->list))
	  {
	    init_search_space(list_search_space, params->disk, params->partition);
	  }
	  if(carve_free_space_only>0)
	  {
	    params->blocksize=remove_used_space(params->disk, params->partition, list_search_space);
	  }
	  if(user_blocksize > 0)
	    params->blocksize=user_blocksize;
	  photorec(params, options, list_search_space, carve_free_space_only);
	}
      }
      else if(strncmp(params->cmd_run,"options",7)==0)
      {
	params->cmd_run+=7;
	interface_options_photorec(options, &params->cmd_run);
      }
      else if(strncmp(params->cmd_run,"fileopt",7)==0)
      {
	params->cmd_run+=7;
	interface_file_select(options->list_file_format, &params->cmd_run);
      }
      else if(strncmp(params->cmd_run,"blocksize,",10)==0)
      {
	params->cmd_run+=10;
	user_blocksize=atoi(params->cmd_run);
	while(params->cmd_run[0]!=',' && params->cmd_run[0]!='\0')
	  params->cmd_run++;
      }
      else if(strncmp(params->cmd_run,"geometry,",9)==0)
      {
	params->cmd_run+=9;
	change_geometry(params->disk, &params->cmd_run);
      }
      else if(strncmp(params->cmd_run,"inter",5)==0)
      {	/* Start interactive mode */
	params->cmd_run=NULL;
      }
      else if(strncmp(params->cmd_run,"wholespace",10)==0)
      {
	params->cmd_run+=10;
	carve_free_space_only=0;
      }
      else if(strncmp(params->cmd_run,"freespace",9)==0)
      {
	params->cmd_run+=9;
	carve_free_space_only=1;
      }
      else if(strncmp(params->cmd_run,"ext2_group,",11)==0)
      {
	unsigned int groupnr;
	params->cmd_run+=11;
	options->mode_ext2=1;
	groupnr=atoi(params->cmd_run);
	while(params->cmd_run[0]!=',' && params->cmd_run[0]!='\0')
	  params->cmd_run++;
	if(mode_init_space==INIT_SPACE_WHOLE)
	  mode_init_space=INIT_SPACE_EXT2_GROUP;
	if(mode_init_space==INIT_SPACE_EXT2_GROUP)
	{
          alloc_data_t *new_free_space;
          new_free_space=(alloc_data_t*)MALLOC(sizeof(*new_free_space));
          /* Temporary storage, values need to be multiplied by group size and aligned */
          new_free_space->start=groupnr;
          new_free_space->end=groupnr;
          new_free_space->file_stat=NULL;
          if(td_list_add_sorted_uniq(&new_free_space->list, &list_search_space->list, spacerange_cmp))
	    free(new_free_space);
        }
      }
      else if(strncmp(params->cmd_run,"ext2_inode,",11)==0)
      {
	unsigned int inodenr;
	params->cmd_run+=11;
	options->mode_ext2=1;
	inodenr=atoi(params->cmd_run);
	while(params->cmd_run[0]!=',' && params->cmd_run[0]!='\0')
	  params->cmd_run++;
	if(mode_init_space==INIT_SPACE_WHOLE)
	  mode_init_space=INIT_SPACE_EXT2_INODE;
	if(mode_init_space==INIT_SPACE_EXT2_INODE)
	{
          alloc_data_t *new_free_space;
          new_free_space=(alloc_data_t*)MALLOC(sizeof(*new_free_space));
          /* Temporary storage, values need to be multiplied by group size and aligned */
          new_free_space->start=inodenr;
          new_free_space->end=inodenr;
          new_free_space->file_stat=NULL;
          if(td_list_add_sorted_uniq(&new_free_space->list, &list_search_space->list, spacerange_cmp))
	    free(new_free_space);
        }
      }
      else if(isdigit(params->cmd_run[0]))
      {
	list_part_t *element;
	unsigned int order;
	order= atoi(params->cmd_run);
	while(params->cmd_run[0]!=',' && params->cmd_run[0]!='\0')
	  params->cmd_run++;
	for(element=list_part;element!=NULL && element->part->order!=order;element=element->next);
	if(element!=NULL)
	  current_element=element;
      }
      else
      {
	log_critical("Syntax error in command line: %s\n", params->cmd_run);
	part_free_list(list_part);
	return;
      }
    }
    // ovde pocinju ncurse meniji
   // char resZ[] = "D:\\work";
    //params->recup_dir=(char *)MALLOC(strlen(resZ)+1+strlen(DEFAULT_RECUP_DIR)+1);
//		strcpy(params->recup_dir,resZ);/
		//strcat(params->recup_dir,"/");
	//	strcat(params->recup_dir,DEFAULT_RECUP_DIR);
                
	  if(current_element!=NULL)
	  {
	    params->partition=current_element->part;
	    ask_mode_ext2(params->disk, params->partition, &options->mode_ext2, &carve_free_space_only); //ovde pita da li je ext2 i odredjuje carve_free_space_only
	 
	    if(params->recup_dir==NULL)
	    {
	      char *res;
	    //  res=ask_location("Please select a destination to save the recovered files.\nDo not choose to write the files to the same partition they were stored on.", "", NULL);
	      if(res!=NULL)
	      {
		params->recup_dir=(char *)MALLOC(strlen(res)+1+strlen(DEFAULT_RECUP_DIR)+1);
		strcpy(params->recup_dir,res);
		strcat(params->recup_dir,"/");
		strcat(params->recup_dir,DEFAULT_RECUP_DIR);
		free(res);
	      }
	    }
	    if(params->recup_dir!=NULL)
	    {
	      if(td_list_empty(&list_search_space->list))
	      {
		init_search_space(list_search_space, params->disk, params->partition);
	      }
	      if(carve_free_space_only>0)
	      {
		params->blocksize=remove_used_space(params->disk, params->partition, list_search_space);
		/* Only free space is carved, list_search_space is modified.
		 * To carve the whole space, need to quit and reselect the params->partition */
		done = 1;
	      }
	      photorec(params, options, list_search_space, carve_free_space_only);
	    }
	  }
    
  }
  log_info("\n");
  part_free_list(list_part);
}
