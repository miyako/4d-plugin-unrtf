#ifndef _PATH_H_INCLUDED_
#define _PATH_H_INCLUDED_

#ifndef DEFAULT_UNRTF_SEARCH_PATH
#define DEFAULT_UNRTF_SEARCH_PATH       CONFIG_DIR
#endif

//#include "4DPluginAPI.h"
//#include "4DPlugin.h"

//extern char *search_path;
//extern int   path_checked;

struct path_dir
{
	char *dir_name;
	struct path_dir *next;
};

extern int   check_dirs();
extern void  show_dirs();
extern char *search_in_path(const char *name, char *suffix);

#endif /* _PATH_H_INCLUDED_ */
