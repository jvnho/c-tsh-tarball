#ifndef TAR_H
#define TAR_H
#include <stdio.h>
#include <string.h>
/* tar Header Block, from POSIX 1003.1-1990.  */

#define BLOCKSIZE 512
#define BLOCKBITS 9

/* POSIX header.  */

/* Note that sizeof(struct posix_header) == BLOCKSIZE */

struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
  char junk[12];                /* 500 */
};                              /* Total: 512 */

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* ... */

#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */
extern void set_checksum(struct posix_header *hd);
extern int check_checksum(struct posix_header *hd);
extern int end_bloc(struct posix_header *header);
extern void put_at_the_first_null(int descriptor);
extern struct posix_header copyHeader(struct posix_header initial, char *name);
extern struct posix_header create_header(char * name, int dir, int size);
extern int writeZero(int tar_descriptor);
extern int dir_exist(int descriptor, char * directory);
#endif