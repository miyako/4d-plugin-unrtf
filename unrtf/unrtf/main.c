/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000, 2001, 2004 by Zachary Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

   The maintainer is reachable by electronic mail at daved@physiol.usyd.edu.au
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    main.c
 * Author name:    Zachary Smith
 * Create date:    01 Sep 00
 * Purpose:        main() routine with file open/close.
 *----------------------------------------------------------------------
 * Changes:
 * 14 Oct 00, tuorfa@yahoo.com: added -nopict option
 * 15 Oct 00, tuorfa@yahoo.com: added verify_file_type()
 * 08 Apr 01, tuorfa@yahoo.com: more GNU-like switches implemented
 * 24 Jul 01, tuorfa@yahoo.com: removed verify_file_type()
 * 03 Aug 01, tuorfa@yahoo.com: added --inline switch
 * 08 Sep 01, tuorfa@yahoo.com: added use of UnRTF
 * 19 Sep 01, tuorfa@yahoo.com: addition of output personalities
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks
 * 23 Sep 01, tuorfa@yahoo.com: added wpml switch
 * 08 Oct 03, daved@physiol.usyd.edu.au: added stdlib.h for linux
 * 07 Jan 04, tuorfa@yahoo.com: removed broken PS support
 * 25 Sep 04, st001906@hrz1.hrz.tu-darmstadt.de: added stdlib.h for djgpp
 * 29 Mar 05, daved@physiol.usyd.edu.au: changes requested by ZT Smith
 * 06 Jan 06, marcossamaral@terra.com.br: includes verbose mode
 * 16 Dec 07, daved@physiol.usyd.edu.au: updated to GPL v3
 * 17 Dec 07, daved@physiol.usyd.edu.au: support for --noremap from
 *		David Santinoli
 * 09 Nov 08, arkadiusz.firus@gmail.com: support for -t <tag_file>
 		and read stdin if no input file provided
 * 13 Dec 08, daved@physiol.usyd.edu.au: search path code
 * 17 Jan 10, daved@physiol.usyd.edu.au: change search path to directory
 *		containing output conf and font charmap files
 * 14 Dec 11, jf@dockes.org: cleaned up get_config
 * 14 Dec 11, daved@physiol.usyd.edu.au: cleaned up get_config added --quiet
 *--------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "defs.h"
#include "error.h"
#include "word.h"
#include "convert.h"
#include "parse.h"
#include "hash.h"
#include "malloc.h"
#include "path.h"

#include "output.h"
#include "user.h"
#include "main.h"
#include "util.h"
