/**
 * File              : ls.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.02.2022
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef k_lib_ls_h__
#define k_lib_ls_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#if defined _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#endif	

typedef enum filetype_t {
	FILETYPE_FILE,
	FILETYPE_LINK,
	FILETYPE_DIR,
	FILETYPE_OTHER
} FILETYPE;

//execute callback function for each file in path
static int ls(const char *path, void *user_data, int (*callback)(const char *filename, FILETYPE, void *user_data));

/**
 * IMPLIMATION
 */

//run callback for every file in dir. to stop execution - return non zero from callback
int ls(const char *dir, void *user_data, int (*callback)(const char *filename, FILETYPE, void *user_data)){
	char error[BUFSIZ];
#if defined _WIN32
	WIN32_FIND_DATAA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	char fullpath[BUFSIZ];
	sprintf(fullpath, "%s\\*", dir);

	hFind = FindFirstFileA(fullpath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	}
	while(FindNextFileA(hFind, &findData) != 0) {
		char * filename = findData.cFileName; 
		FILETYPE filetype = FILETYPE_OTHER;
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			filetype = FILETYPE_DIR;
		else
			filetype = FILETYPE_FILE;
		
		if (callback)
			if (callback(filename, filetype, user_data) != 0)
				return 1;
	}
	dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES) {
		return dwError;
	}
#else	
	struct dirent *entry;
	DIR *dp = opendir(dir);
	if (dp == NULL){
		return -1;
	}	
	while((entry = readdir(dp))){ //reading files
		char * filename = entry->d_name;
		FILETYPE filetype = FILETYPE_OTHER;
		switch (entry->d_type) {
			case  DT_REG: filetype = FILETYPE_FILE; break;
			case  DT_DIR: filetype = FILETYPE_DIR; break;
			case  DT_LNK: filetype = FILETYPE_LINK; break;
			default: filetype = FILETYPE_OTHER; break;
		}
		if (callback)
			if (callback(filename, filetype, user_data) != 0)
				return 1;
	}
	closedir(dp);
#endif
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif //k_lib_ls_h__
