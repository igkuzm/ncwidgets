/**
 * File              : fm.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 04.09.2021
 * Last Modified Date: 07.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/*
 * FileManager
 * Functions to manage files and directories
 */

#ifndef k_lib_fm_h__
#define k_lib_fm_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

/* fexists
 * true if file exists and writable
 * %path - file path
 */
static bool fexists(const char *path);

/* isdir
 * true if directory at path exists
 * and is accesable
 * %path - directory path
 */
static bool isdir(const char *path);

/* fsize
 * return file size
 * %path - file path
 */
static off_t fsize(const char *path);

/* homedir
 * return allocated string with path to home directory
 */
static char * homedir(void);

/* fext
 * return file extension or NULL on error 
 * %filename - name or path of file
 */
static const char * fext(const char *filename);

/* fname
 * return allocated string with file name without 
 * extension and path
 * %path - name or path of file
 */
static char * fname(char *path);

/* lastpath
 * return last path component 
 * %path - name or path of file
 */
static int lastpath(const char *path);

/* dname
 * return allocated string with name of 
 * directory path (like POSIX dirname())
 * %path - path of file
 */
static char * dname(const char *path);

/* fcopy 
 * copy and overwrite file 
 * return 0 on success
 * %from - filepath source file
 * %to   - filepath dastination file 
 */ 
static int fcopy(const char *from, const char *to);

/* dcopy 
 * copy directory recursive
 * return 0 on success
 * %from - filepath source file
 * %to   - filepath dastination file 
 * %overwrite - overwrite destination file if true
 * %error - pointer to allocated error message
*/ 
static int dcopy(const char *from, const char *to,
		             bool overwrite, char **error);

/* newdir
 * create new directory
 * link to mkdir function with universal
 * for unix/windows args
 * %path - directory path with name
 * %mode - access mode (not used in windows)
 */
static int newdir(const char *path, int mode);

/*
 * POSIX functions for Windows
 * 
 * basename
 * returns the last path component or NULL on error
 * %path - file path
 * const char *basename(const char *path);
 *
 * dirname
 * returns allocated string with the path without last 
 * component or NULL on error
 * %path - file path
 * char *dirname(const char *path);
 *
 * scandir
 * scans the directory dirp, calling filter()
 * on each directory entry.
 * Entries for which filter() returns
 * nonzero are stored in strings allocated 
 * via malloc, sorted using qsort with the 
 * comparison function compar(), and
 * collected in array namelist which is 
 * allocated via malloc. If filter is NULL, all 
 * entries are selected.
 * The alphasort() and versionsort() functions can be 
 * used as the comparison function compar(). The 
 * former sorts directory entries using strcoll, the 
 * latter using strverscmp on the strings
 * (*a)->d_name and (*b)->d_name.
 * %dirp - directory path
 * %namelist - allocated namelist array of dirents
 * %filter - filter funcion
 * %compar - sort function
 * int scandir(
 *		 const char *restrict dirp,
 *		 struct dirent ***restrict namelist,
 *		 int (*filter)(const struct dirent *),
 *		 int (*compar)(
 *					const struct dirent **, 
 *					const struct dirent **));
 */

/********************************************/
/*IMPLIMATION *******************************/	
/********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define F_OK 0
#define access _access
	static const char *basename(const char *path);
#else
#include <unistd.h>
#include <sys/stat.h> // mkdir, stat
#include <dirent.h>
#include <libgen.h>   // basename, basedir
#endif

bool fexists(const char *path) {
  if (access(path, F_OK) == 0)
    return true;
  return false;
}

off_t fsize(const char *path) {
#if defined _WIN32
  WIN32_FIND_DATAA findData;
  HANDLE hFind = INVALID_HANDLE_VALUE;

  hFind = FindFirstFileA(path, &findData);
  if (hFind != INVALID_HANDLE_VALUE) {
    return (findData.nFileSizeHigh * (MAXDWORD + 1)) + findData.nFileSizeLow;
  }
#else
  struct stat st;
  if (stat(path, &st) == 0) {
    return st.st_size;
  }
#endif
  return 0;
}

char *homedir(void)
{
	char *p = (char *)malloc(BUFSIZ);
	if (!p){
		perror("malloc");
		return NULL;
	}
#ifdef _WIN32
	snprintf(p, BUFSIZ, "%s%s", 
			getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
	snprintf(p, BUFSIZ, "%s", 
			getenv("HOME"));
#endif
	return p;																								        
}

bool isdir(const char *path) {
#if defined _WIN32
  WIN32_FIND_DATAA findData;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  char fullpath[BUFSIZ];
  sprintf(fullpath, "%s\\*", path);

  hFind = FindFirstFileA(fullpath, &findData);
  if (hFind != INVALID_HANDLE_VALUE)
    return true;
#else
  struct dirent *entry;
  DIR *dp = opendir(path);
  if (dp != NULL) {
    closedir(dp);
    return true;
  }
#endif
  return false;
}

const char * fext(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

char *
fname(char *filename)
{
	char *b = basename(filename);
	char *p = strdup(b);
	char *s = strrchr(p, '.');
	if (s)
		*s = 0;
	return p;
}

char * dname(const char *path) {
	char *d = strdup(path);
#if defined _WIN32
	char *s = strrchr(d, '\\');
#else
	char *s = strrchr(d, '/');
#endif
	if (s)
		*s = 0;
	else 
		strcpy(d, ".");
	return d;
}

static int lastpath(const char *path) {
		const char *slash;
#ifdef _WIN32
	 	slash = strrchr(path, '\\');
#else
	 	slash = strrchr(path, '/');
#endif
		if (!slash || slash == path)
			return 0;
		return slash - path;
}

/* return codes of fcopy functions */
enum {
	FCP_NOERR = 0,
	FCP_FROM,
	FCP_TO,
	FCP_ERRNO
};

int fcopy (const char *from, const char *to) {
	int ret = FCP_NOERR;
	FILE *src, *dst;
	char buf[BUFSIZ];

	// open source file
	if ((src=fopen(from, "rb"))){
		// open destination file
		if ((dst = fopen(to, "wb"))){
			// do copy
			while (feof(src) == 0){
				fread (buf, sizeof(buf), 1, src);
				fwrite(buf, sizeof(buf), 1, dst);
			}
			// check errors - to handle error call errno()
			if (ferror(src) || ferror(dst))
				ret = FCP_ERRNO;
			fclose(dst);
		} else {
			ret = FCP_TO;
		}
		fclose(src);
	} else {
		ret = FCP_FROM;
	}

  return ret;
}

#define DCOPY_ERR(...)\
	({\
		char *e = (char *)malloc(BUFSIZ);\
		if (e){\
			sprintf(e, __VA_ARGS__);\
		} else\
			perror("malloc");\
	  e;\
	})

static void _dcopy_herror(
		int res, const char *src, const char *dst, 
		char **error)
{
	if (error){
		switch (res) {
			case FCP_FROM:
				*error = 
					DCOPY_ERR(
							"can't read file: %s", src); 
			case FCP_TO:
				*error = 
					DCOPY_ERR(
							"can't read file: %s", dst); 
			case FCP_ERRNO:
				*error = 
					DCOPY_ERR(
							"write file error: %s, %s: %d", src, dst, errno); 
		}
	}
}

int dcopy(const char *from, const char *to,
		bool overwrite, char **error)
{
#if defined _WIN32
	WIN32_FIND_DATAA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	char fullpath[BUFSIZ];
	sprintf(fullpath, "%s\\*", from);

	hFind = FindFirstFileA(fullpath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (error)
			*error = DCOPY_ERR("can't open directory: %s", from); 
		return -1;
	}

	// create new directory
	mkdir(to);

	// read files	
	while(FindNextFileA(hFind, &findData) != 0) {
		char src[BUFSIZ], dst[BUFSIZ];
		sprintf(src, "%s/%s", from, findData.cFileName);
		sprintf(dst, "%s/%s", to,   findData.cFileName);
		
		// check if directory
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			dcopy(src, dst, overwrite, error);
		}
		else {
			// check if not exists or overwrite 
			if (!fexists(dst) || overwrite){
				int res = fcopy(src, dst);
				if (res)
					_dcopy_herror(res, src, dst, error);
			}
		}
	}
	dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES) {
		return dwError;
	}
#else	
	struct dirent *entry;
	DIR *dp = opendir(from);
	if (dp == NULL){
		if (error)
			*error = DCOPY_ERR("can't open directory: %s", from); 
		return -1;
	}	
	
	// create new directory
	struct stat dstat;
	if (stat(from, &dstat) < 0){
		if (error)
			*error = DCOPY_ERR("can't read directory: %s", from); 
		closedir(dp);
		return -1;
	};
	mkdir(to, dstat.st_mode);
	
	// read files
	while((entry = readdir(dp))){
		// skip system dir names
		if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0)
			continue;
		
		char src[BUFSIZ], dst[BUFSIZ];
		sprintf(src, "%s/%s", from, entry->d_name);
		sprintf(dst, "%s/%s", to,   entry->d_name);

		// check if directory
		if (entry->d_type == DT_DIR){
			dcopy(src, dst, overwrite, error);
		} else {
			// check if not exists or overwrite 
			if (!fexists(dst) || overwrite){
				int res = fcopy(src, dst);
				if (res)
					_dcopy_herror(res, src, dst, error);
			}
		}
	}
	closedir(dp);
#endif
	return 0;
}

int newdir(const char *path, int mode)
{
#ifdef _WIN32
	return mkdir(path);
#else
	return mkdir(path, mode);
}
#endif

/* POSIX FUNCTIONS FOR WINDOWS */
#ifdef _WIN32

enum
  {
    DT_UNKNOWN = 0,
# define DT_UNKNOWN	DT_UNKNOWN
    DT_FIFO = 1,
# define DT_FIFO	DT_FIFO
    DT_CHR = 2,
# define DT_CHR		DT_CHR
    DT_DIR = 4,
# define DT_DIR		DT_DIR
    DT_BLK = 6,
# define DT_BLK		DT_BLK
    DT_REG = 8,
# define DT_REG		DT_REG
    DT_LNK = 10,
# define DT_LNK		DT_LNK
    DT_SOCK = 12,
# define DT_SOCK	DT_SOCK
    DT_WHT = 14
# define DT_WHT		DT_WHT
  };

typedef unsigned long ino_t;

struct dirent {
	ino_t          d_ino;       /* inode number */
	off_t          d_off;       /* offset to the next dirent */
	unsigned short d_reclen;    /* length of this record */
	unsigned char  d_type;      /* type of file; not supported
														     by all file system types */
	char           d_name[256]; /* filename */
};

/* returns pointer to path string without 
 * last path component*/
const char *
basename(const char *path)
{
	const char *dash = strrchr(path, '\\');
	if (!dash || dash == path)
		return path;
	return dash + 1;
}

/* returns allocated string with directory path 
 * or NULL on error */
char *
dirname(path)
        const char *path;
{
		static char *bname = NULL;
		const char *endp;

		if (bname == NULL) {
						bname = (char *)malloc(MAX_PATH);
						if (bname == NULL)
										return(NULL);
		}

		/* Empty or NULL string gets treated as "." */
		if (path == NULL || *path == '\0') {
						(void)strcpy(bname, ".");
						return(bname);
		}

		/* Strip trailing slashes */
		endp = path + strlen(path) - 1;
		while (endp > path && *endp == '\\')
						endp--;

		/* Find the start of the dir */
		while (endp > path && *endp != '\\')
						endp--;

		/* Either the dir is "/" or there are no slashes */
		if (endp == path) {
						(void)strcpy(bname, *endp == '\\' ? "\\" : ".");
						return(bname);
		} else {
						do {
										endp--;
						} while (endp > path && *endp == '\\');
		}

		if (endp - path + 2 > MAX_PATH) {
						return(NULL);
		}
		(void)strncpy(bname, path, endp - path + 1);
		bname[endp - path + 1] = '\0';
		return(bname);
}

#define ISDIGIT(a) isdigit(a)

/* states: S_N: normal, S_I: comparing integral part, S_F: comparing
           fractional parts, S_Z: idem but with leading Zeroes only */
#define  S_N    0x0
#define  S_I    0x4
#define  S_F    0x8
#define  S_Z    0xC

/* result_type: CMP: return diff; LEN: compare using len_diff/diff */
#define  CMP    2
#define  LEN    3


/* Compare S1 and S2 as strings holding indices/version numbers,
   returning less than, equal to or greater than zero if S1 is less than,
   equal to or greater than S2 (for more info, see the Glibc texinfo doc).  */

int
strverscmp (const char *s1, const char *s2)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;
  int state;
  int diff;

  /* Symbol(s)    0       [1-9]   others  (padding)
     Transition   (10) 0  (01) d  (00) x  (11) -   */
  static const unsigned int next_state[] =
    {
      /* state    x    d    0    - */
      /* S_N */  S_N, S_I, S_Z, S_N,
      /* S_I */  S_N, S_I, S_I, S_I,
      /* S_F */  S_N, S_F, S_F, S_F,
      /* S_Z */  S_N, S_F, S_Z, S_Z
    };

  static const int result_type[] =
    {
      /* state   x/x  x/d  x/0  x/-  d/x  d/d  d/0  d/-
                 0/x  0/d  0/0  0/-  -/x  -/d  -/0  -/- */

      /* S_N */  CMP, CMP, CMP, CMP, CMP, LEN, CMP, CMP,
                 CMP, CMP, CMP, CMP, CMP, CMP, CMP, CMP,
      /* S_I */  CMP, -1,  -1,  CMP, +1,  LEN, LEN, CMP,
                 +1,  LEN, LEN, CMP, CMP, CMP, CMP, CMP,
      /* S_F */  CMP, CMP, CMP, CMP, CMP, LEN, CMP, CMP,
                 CMP, CMP, CMP, CMP, CMP, CMP, CMP, CMP,
      /* S_Z */  CMP, +1,  +1,  CMP, -1,  CMP, CMP, CMP,
                 -1,  CMP, CMP, CMP
    };

  if (p1 == p2)
    return 0;

  c1 = *p1++;
  c2 = *p2++;
  /* Hint: '0' is a digit too.  */
  state = S_N | ((c1 == '0') + (ISDIGIT (c1) != 0));

  while ((diff = c1 - c2) == 0 && c1 != '\0')
    {
      state = next_state[state];
      c1 = *p1++;
      c2 = *p2++;
      state |= (c1 == '0') + (ISDIGIT (c1) != 0);
    }

  state = result_type[state << 2 | (((c2 == '0') + (ISDIGIT (c2) != 0)))];

  switch (state)
    {
    case CMP:
      return diff;

    case LEN:
      while (ISDIGIT (*p1++))
        if (!ISDIGIT (*p2++))
          return 1;

      return ISDIGIT (*p2) ? -1 : diff;

    default:
      return state;
    }
}

static int alphasort(
		const struct dirent **a, const struct dirent **b)
{
	return strcoll((*a)->d_name, (*b)->d_name);
}

static int versionsort(
	const struct dirent **a, const struct dirent **b)
{
	return strverscmp((*a)->d_name, (*b)->d_name);
}

static int 
scandir(
		 const char *restrict dirp,
		 struct dirent ***restrict namelist,
		 int (*filter)(const struct dirent *),
		 int (*compar)(
					const void *, 
					const void *))
{
	WIN32_FIND_DATAA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	char fullpath[BUFSIZ];
	sprintf(fullpath, "%s\\*", dirp);

	hFind = FindFirstFileA(fullpath, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
		return -1;

	struct dirent **array = NULL;
	int len = 0;

	while(FindNextFileA(hFind, &findData) != 0) {
		struct dirent *entry = 
			(struct dirent *)malloc(sizeof(struct dirent));
		if (!entry)
			return -1;

		strncpy(entry->d_name, findData.cFileName, 
				sizeof(entry->d_name) - 1);
		entry->d_name[sizeof(entry->d_name)-1] = 0;
		
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			entry->d_type = DT_DIR;
		else 
			entry->d_type = DT_REG;
		
		if (filter && filter(entry) == 0){
			free(entry);
		} else {
			// add to array
			array = 
				(struct dirent **)realloc(array, 
						sizeof(array) * len + sizeof(array));
			if (!array)
				return -1;

			array[len++] = entry; 
		}
	}

	// sort array
	if (compar)
		qsort(array, len, sizeof(struct dirent *), compar);

	if (namelist)
		*namelist = array;
	else{
		for (i = 0; i < len; ++i)
			free(array[i])
		free(array);
	}

	return len;
}

#endif

#ifdef __cplusplus
}
#endif

#endif // k_lib_fm_h__
