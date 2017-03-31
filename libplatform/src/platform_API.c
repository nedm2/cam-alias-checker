/*
 *  Copyright (C) 2010 - 2014 Simone Campanoni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifdef WIN32

/* Macro defined for minGW compatibility*/
#define _WIN32_WINNT  0x0601
#define WINVER        0x0601

// WIN32 Header Files:
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>

#include <windows.h>
#include <process.h> // required for exit() function

/* Header files supported by MinGW */
#include <getopt.h>
#include <dirent.h>
#include <utime.h>
#include <sys/stat.h>
#include <malloc.h>
#include <unistd.h>

/* WinPthread32 header: */
#include <pthread.h>

/* MinGW Regex Header: */
#include <regex.h>

// probably useless header files (included in windows.h)
#include <direct.h>
#include <io.h>
#include <winsock.h>
#include <winbase.h>
#include <Winnt.h>

// some useful defines
#define INFO_BUFFER_SIZE 256
#define BUFSIZE 256      //needed by PLATFORM_get_current_dir_name

#else // LINUX Header files:
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <regex.h>
#include <sched.h>
#include <alloca.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <langinfo.h>
#include <utime.h>
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <pwd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/user.h>
//#include <mcheck.h> used in ILDJIT only for DEBUG.


#endif

// My headers
#include "platform_API.h"




//#ifdef WIN32
///* DLL Entry Point */
//BOOL APIENTRY DllMain (HINSTANCE hInst /* Library instance handle. */,
//		       DWORD reason /* Reason this function is being called. */,
//               LPVOID reserved /* Not used. */ ){
//	switch (reason) {
//	    case DLL_PROCESS_ATTACH:
//		    break;
//
//	    case DLL_PROCESS_DETACH:
//		    break;
//
//	    case DLL_THREAD_ATTACH:
//		    break;
//
//	    case DLL_THREAD_DETACH:
//		    break;
//	}
//
//	/* Returns TRUE on success, FALSE on failure */
//	return TRUE;
//}
//#endif



#ifdef WIN32    // Procedure executed in case of not supported function for Win32
void winFuncNotSupported (char* msg) {
    printf("PLATFORM_API:  Function '%s' not supported. Program terminated", msg);
    exit(191); // terminate entire program
}
#endif

#ifdef WIN32    // Procedure for debug, before the execution of the function
#ifdef WINDEBUG
void winDebugStart (const char* funcName) {
    printf("PLATFORM_API DEBUG: Function '%s' has been called...\n", funcName);
}
#endif
#endif


JITBOOLEAN PLATFORM_doesFileExist (JITINT8 *fileName) {
    FILE    *file;

    if (fileName == NULL) {
        return JITFALSE;
    }
    file = fopen((char *) fileName, "r");
    if (file == NULL) {
        return JITFALSE;
    }
    fclose(file);
    return JITTRUE;
}

//=======================================  fcntl.h

// Lock or unlock file in byterange specified in cntrl_data struct (see fcntl F_SETLKW for details)
JITNINT PLATFORM_setFileLock ( JITNINT fileHandler, struct flock *cntl_data ) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    BOOL check;
    DWORD start, len;
    start = cntl_data->l_start;
    len = cntl_data->l_len;

    if ( ((DWORD) cntl_data->l_type) == F_WRLCK) {
        check = LockFile( (HANDLE) fileHandler, start, 0,len,0);
        if (!check)	{
            return -1;
        } else {
            return 0;
        }
    }

    if (((DWORD) cntl_data->l_type) == F_UNLCK) {
        check = UnlockFile( (HANDLE) fileHandler, start, 0,len,0);
        if (!check)	{
            return -1;
        } else {
            return 0;
        }
    }
    return 0;

#else           // Linux
    return fcntl( fileHandler, F_SETLKW, &cntl_data);
#endif
}



JITINT32 PLATFORM_open (const char *file, int modes, int permission) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    (void) permission; /* parameter not used in WIN */
    int flag;
    DWORD dwAttrs;
    dwAttrs = GetFileAttributes(file);

    if (0xFFFFFFFF == dwAttrs) {
        flag = 0;
    } else {
        flag = 1;
    }

    switch (modes) {
        case 1281:             //creat, excl,w
            if (flag == 0) {
                return (JITINT32) fopen(file, "w");
            } else {
                return -1;
            }
            break;
        case 1282:             //creat, excl, rw
            if (flag == 0) {
                return (JITINT32) fopen(file, "w+");
            } else {
                return -1;
            }
            break;
        case 769:             // creat, trunc, w
            return (JITINT32) fopen(file, "w");
            break;
        case 770:             //creat, trunc, wr
            return (JITINT32) fopen(file, "w+");
            break;
        case 0:             //regular, r
            return (JITINT32) fopen(file, "r");
            break;
        case 1:             // regular, w
            return (JITINT32) fopen(file, "r+");
            break;
        case 2:             // regular, rw
            return (JITINT32) fopen(file, "r+");
            break;
        case 256:             // creat, r
            if (flag == 0) {
                return (JITINT32) fopen(file, "w");
                break;
            } else {
                return (JITINT32) fopen(file, "r");
                break;
            }
        case 257:             // creat, w
            if (flag == 0) {
                return (JITINT32) fopen(file, "w");
                break;
            } else {
                return (JITINT32) fopen(file, "r+");
                break;
            }
        case 258:             // creat, rw
            if (flag == 0) {
                return (JITINT32) fopen(file, "w+");
                break;
            } else {
                return (JITINT32) fopen(file, "r+");
                break;
            }
        case 513:             // trunc, w
            if (flag == 1) {
                return (JITINT32) fopen(file, "w");
                break;
            } else {
                return -1;
            }
            break;
        case 514:             // trunc, wr
            if (flag == 1) {
                return (JITINT32) fopen(file, "w+");
                break;
            } else {
                return -1;
            }
            break;
        case 9:             // append, w
            return (JITINT32) fopen(file, "a");
            break;
        case 10:             // append, rw
            return (JITINT32) fopen(file, "a+");
            break;
        default:
            return -1;
            break;
    }
    return -1;
#else
    return open(file, modes, permission);
#endif
}

JITUINT64 PLATFORM_sizeOfFile (FILE *file) {
    JITUINT64	current;
    JITUINT64	size;

    /* Assertions.
     */
    assert(file != NULL);

    /* Fetch the current position.
     */
    current		= ftell(file);

    /* Seek till the end of the file.
     */
    fseek(file, 0L, SEEK_END);

    /* Fetch the size.
     */
    size		= ftell(file);

    /* Seek back.
     */
    fseek(file, current, SEEK_SET);

    return size;
}


//=======================================  Sys/stat.h

/* NOTE for Windows platform: this function is supported by minGW, not Windows! */
JITNINT PLATFORM_stat (const char *arg1, struct stat *arg2) {
    return stat(arg1,arg2);
}




/* NOTE for Windows platform: this function is supported by minGW, not Windows! */
JITNINT PLATFORM_fstat (JITNINT arg1, struct stat *arg2) {
    return fstat(arg1,arg2);
}



/* like stat(), but works with SymLinks*/
JITNINT PLATFORM_lstat (const char *linkPath, struct stat *stat) {
#ifdef WIN32    // Windows
#define PATH_LENGTH 256
    char filePath[PATH_LENGTH];

    /* Get symbolic link target */
    if ( PLATFORM_readlink(linkPath, filePath,PATH_LENGTH) != -1) {
        /* return the stat of the target file*/
        return PLATFORM_stat(filePath, stat);
    } else {
        /* returns error: cannot open symbolic link */
        return -1;
    }
#else           // Linux
    return lstat(linkPath,stat);
#endif
}



//mode_t is JITINT32 type
mode_t PLATFORM_umask (mode_t newMask) {
#ifdef WIN32                    // Windows
    /* alternative implementation:
       mode_tToWinFileMode(arg1);
       set&read all the mode
       WinFileModeTomode_t(); */
    (void) newMask;
    mode_t oldMask = 18;    /* default read/write value - ok for current ILDJIT implementation*/
    return oldMask;
#else                           // Linux
    return umask(newMask);
#endif
}



JITNINT PLATFORM_mkdir (const char* pathname, mode_t mode) {
#ifdef WIN32            // Windows
    (void) mode;    /* not used in Windows */
    return _mkdir(pathname);
#else                   // Linux
    return mkdir(pathname,mode);
#endif
}





//=====================================  unistd.h
JITNINT PLATFORM_chdir (const char* arg1) {
#ifdef WIN32    // Windows
    return _chdir(arg1);
#else           // Linux
    return chdir(arg1);
#endif
}



JITNINT PLATFORM_close (JITNINT arg1) {
#ifdef WIN32    // Windows
    return _close(arg1);
#else           // Linux
    return close(arg1);
#endif
}


/* NOTE for Windows platform: this function is supported by minGW, not Windows!
   If ftruncate is not supported, use _chsize(fd,length) for win*/
JITNINT PLATFORM_ftruncate (JITNINT fd, off_t length) {
    return ftruncate(fd,length);
}


/* NOTE for Windows platform: link to libws2_32.a lib needed */
JITNINT PLATFORM_gethostname (char *name, size_t size) {
    return gethostname(name,size);
}



JITNINT PLATFORM_isatty (JITNINT arg1) {
#ifdef WIN32    // Windows
    return _isatty(arg1);
#else           // Linux
    return isatty(arg1);
#endif
}

off_t  PLATFORM_lseek (JITNINT arg1, off_t arg2, JITNINT arg3) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    return _lseek(arg1,arg2,arg3);
#else           // Linux
    return lseek(arg1,arg2,arg3);
#endif
}

FILE * PLATFORM_fmemopen (void *buf, size_t size, JITINT8 *mode) {
    FILE	*f;

    /* Open a file.
     */
    f	= fmemopen(buf, size, (const char *)mode);

    return f;
}

ssize_t PLATFORM_read (JITNINT arg1, void* arg2, size_t arg3) {
#ifdef WIN32    // Windows
    return (ssize_t) _read(arg1,arg2,arg3);
#else           // Linux
    return read(arg1,arg2,arg3);
#endif
}

JITNINT PLATFORM_rmdir_recursively (const JITINT8 *dir) {

    JITNINT err;
    DIR     *plugin_dir;
    JITINT8 *buf;
    JITINT32 bufLength;

    /* Assertions		*/
    assert(dir != NULL);

    /* Open the directory	*/
    plugin_dir = PLATFORM_openDir((char *) dir);
    if (plugin_dir == NULL) {
        return 0;
    }

    /* Allocate the buffer	*/
    bufLength = 256 + strlen((char *) dir);
    buf = malloc(sizeof(char) * bufLength);
    assert(buf != NULL);

    /* Load the plugins	*/
    while (1) {
        struct dirent *info_current_plugin;
        char *current_dirname;
        JITNINT dirent_type = 0;

        /* Read the directory			*/
        info_current_plugin = PLATFORM_readDir(plugin_dir);
        if (info_current_plugin == NULL) {
            if (PLATFORM_closeDir(plugin_dir) != 0) {
                fprintf(stderr, "ERROR= Cannot close the plugin directory\n");
                abort();
            }
            break;
        }

        /* Remove the file			*/
        current_dirname = info_current_plugin->d_name;
        if (    (strcmp(current_dirname, ".") != 0)         &&
                (strcmp(current_dirname, "..") != 0)        ) {
            JITUINT32 current_dirname_length;
            current_dirname_length = strlen(current_dirname);
            if (current_dirname[current_dirname_length - 1] == '/') {
                snprintf((char *) buf, bufLength, "%s%s", dir, info_current_plugin->d_name);
            } else {
                snprintf((char *) buf, bufLength, "%s/%s", dir, info_current_plugin->d_name);
            }

            /* Remove the file			*/
#ifdef WIN32    // Windows
            char *current_filepath = malloc(bufLength);
            sprintf(current_filepath, "%s\\%s", current_dirname, info_current_plugin->d_name);
            dirent_type = GetFileAttributes(current_filepath);
            free(current_filepath);
#define DIRENT_TYPE_DIRECTORY FILE_ATTRIBUTE_DIRECTORY
#else           // Linux
            dirent_type = info_current_plugin->d_type;
#define DIRENT_TYPE_DIRECTORY DT_DIR
#endif

            switch (dirent_type) {
                case DIRENT_TYPE_DIRECTORY:
                    PLATFORM_rmdir_recursively(buf);
                    break;
                default:
                    if (PLATFORM_unlink((char *) buf) != 0) {
                        fprintf(stderr, "ERROR on unlinking %s\n", buf);
                        fprintf(stderr, "	%s\n", strerror(errno));
                        abort();
                    }
            }
        }
    }

    /* Free the memory		*/
    free(buf);

    /* Remove the directory		*/
    err = PLATFORM_rmdir(dir);

    /* Return			*/
    return err;

}

JITNINT PLATFORM_rmdir (const JITINT8* arg1) {
#ifdef WIN32    // Windows
    return _rmdir((char*) arg1);
#else           // Linux
    return rmdir((char*) arg1);
#endif
}

ssize_t PLATFORM_write (JITNINT arg1, const void* arg2, size_t arg3) {
#ifdef WIN32    // Windows
    return _write(arg1,arg2,arg3);
#else           // Linux
    return write(arg1,arg2,arg3);
#endif
}

JITNINT PLATFORM_unlink (const char* arg1) {
#ifdef WIN32    // Windows

#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    JITNINT result;
    DWORD dwAttrs;
    dwAttrs = GetFileAttributes(arg1);
    if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY) {
        result = RemoveDirectory(arg1);
        if (!result) {
            return -1;
        } else {
            return 0;
        }
    } else {
        result = DeleteFile(arg1);
        if (!result) {
            return -1;
        } else {
            return 0;
        }
    }
#else           // Linux
    return unlink(arg1);
#endif
}

JITNINT PLATFORM_symlink (const char* arg1, const char *arg2) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    // unlink, symlink and redlink call "METHOD_END(system, "Platform.FileMethodos.CreateLink(char [],char [])");"

    JITNINT result;
    DWORD dwAttrs;
    dwAttrs = GetFileAttributes(arg1);
    /* if is a directory */
    if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY) {
        result = CreateSymbolicLink(arg2, arg1, 0x1);
        if (!result) {
            return -1;
        } else {
            return 0;
        }
    } else {
        result = CreateSymbolicLink(arg2, arg1, 0x0);
        if (!result) {
            return -1;
        } else {
            return 0;
        }
    }
#else           // Linux
    return symlink(arg1,arg2);
#endif
}





JITNINT PLATFORM_readlink (const char* linkPath, char* buf, size_t bufSize) {
#ifdef WIN32    // Windows

    HANDLE symLinkHandle;
    PFILE_NAME_INFO pfileInfo;

    char targetFilePath[bufSize];

    int returnValue=-1;
    int stringSize=0;

    /* Allocate file information */
    size_t dwFileNameLength = 1024;
    pfileInfo = malloc(dwFileNameLength);

    /* Get symbolic link file handle */
    symLinkHandle = CreateFile ( TEXT(linkPath),
                                 GENERIC_READ,
                                 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_BACKUP_SEMANTICS,
                                 NULL);

    if( symLinkHandle != INVALID_HANDLE_VALUE ) {
        /* Get the target filepath of the sybolic link handler */
        size_t fileInfoSize = sizeof(FILE_NAME_INFO) + sizeof(WCHAR)*bufSize;
        if(GetFileInformationByHandleEx(symLinkHandle, FileNameInfo, pfileInfo, fileInfoSize) != 0) {
            /* convert from wide char to narrow char array */
            stringSize = (pfileInfo->FileNameLength)/2;    //wchar sizes double
            WideCharToMultiByte(CP_ACP,0,pfileInfo->FileName,-1, targetFilePath,stringSize,NULL, NULL);
            targetFilePath[stringSize] = '\0';

            /*NOTE: the output string has no drive letter (ex."c:\"), but it's ok*/
            strcpy(buf, targetFilePath);

            /* returns strings size*/
            returnValue = stringSize;
        } else {
            /* error getting informations from file (you can get errorcode by GetLastError()) */
            returnValue = -1;
        }
    } else {
        /* Error opening symbolic link (you can get errorcode by GetLastError()) */
        returnValue = -1;
    }

    /* free previously allocated memory */
    free(pfileInfo);

    return returnValue;

#else           // Linux
    return readlink(linkPath,buf,bufSize);
#endif
}


//=====================================  pthread.h



/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_createThread (pthread_t *arg1, const pthread_attr_t *arg2,  void *(*thread_function)(void*), void *arg4) {
    return pthread_create(arg1,arg2,thread_function,arg4);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_joinThread (pthread_t arg1, void **arg2) {
    return pthread_join(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
pthread_t PLATFORM_getSelfThreadID (void) {
    return pthread_self();
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_pthread_once (pthread_once_t *arg1, void (*thread_function)(void) ) {
    return pthread_once(arg1,thread_function);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_equalThread (pthread_t arg1, pthread_t arg2) {
    return pthread_equal(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_initThreadAttr (pthread_attr_t *arg1) {
    return pthread_attr_init(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setThreadAttr_scope (pthread_attr_t *arg1, JITNINT arg2) {
    return pthread_attr_setscope(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setThreadAttr_stackSize (pthread_attr_t *arg1, size_t arg2) {
    return pthread_attr_setstacksize(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setThreadAttr_inheritsched (pthread_attr_t *arg1, JITNINT arg2) {
    return pthread_attr_setinheritsched(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setThreadAttr_schedparam (pthread_attr_t *arg1, const struct sched_param *arg2) {
    return pthread_attr_setschedparam(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setThreadAttr_schedpolicy (pthread_attr_t *arg1, JITNINT arg2) {
    return pthread_attr_setschedpolicy(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_initMutex (pthread_mutex_t *arg1, const pthread_mutexattr_t *arg2) {
    return pthread_mutex_init(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_initMutexAttr (pthread_mutexattr_t *arg1) {
    return pthread_mutexattr_init(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setMutexAttr_pshared (pthread_mutexattr_t *arg1, JITNINT arg2) {
    return pthread_mutexattr_setpshared(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setMutexAttr_type (pthread_mutexattr_t *arg1, JITNINT arg2) {
    return pthread_mutexattr_settype(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_destroyMutex (pthread_mutex_t *arg1) {
    return pthread_mutex_destroy(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_destroyMutexAttr (pthread_mutexattr_t *arg1) {
    return pthread_mutexattr_destroy(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_lockMutex (pthread_mutex_t *arg1) {
    return pthread_mutex_lock(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_trylockMutex (pthread_mutex_t *arg1) {
    return pthread_mutex_trylock(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_unlockMutex (pthread_mutex_t *arg1) {
    return pthread_mutex_unlock(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_initCondVar (pthread_cond_t *arg1, const pthread_condattr_t *arg2) {
    return pthread_cond_init(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_initCondVarAttr (pthread_condattr_t *arg1) {
    return pthread_condattr_init(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_setCondVarAttr_pshared (pthread_condattr_t *arg1, JITNINT arg2) {
    return pthread_condattr_setpshared(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_waitCondVar (pthread_cond_t *arg1, pthread_mutex_t *arg2) {
    return pthread_cond_wait(arg1,arg2);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_signalCondVar (pthread_cond_t *arg1) {
    return pthread_cond_signal(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_broadcastCondVar (pthread_cond_t *arg1) {
    return pthread_cond_broadcast(arg1);
}

/* NOTE: function implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_destroyCondVar (pthread_cond_t *arg1) {
    return pthread_cond_destroy(arg1);
}


/* Hash function needed by XanHashTables if used with pthreads*/
unsigned int PLATFORM_threadHashFunc (void *element) {
#ifdef WIN32    // Windows
    /* WinPthreads32 pthread_t is a struct: custom hash function needed. */
    pthread_t* threadElem = (pthread_t  *) element;
    return (unsigned int) threadElem->p;
#else           // Linux
    /* POSIX pthread_t is a int: this is the default XanLib Hash Table */
    pthread_t* threadElem = (pthread_t  *) element;
    return (unsigned int ) (*threadElem);
#endif
}


//=====================================  REGEX

/* NOTE: this function is implemented in Windows by MinGW-REGEX libraries (mingw-libgnurx) */
void PLATFORM_regfree (regex_t* arg1) {
    return regfree(arg1);
}

/* NOTE: this function is implemented in Windows by MinGW-REGEX libraries (mingw-libgnurx) */
JITNINT PLATFORM_regexec (const regex_t *arg1, const char *arg2, size_t arg3, regmatch_t* arg4, JITNINT arg5) {
    return regexec(arg1, arg2, arg3, arg4, arg5);
}

/* NOTE: this function is implemented in Windows by MinGW-REGEX libraries (mingw-libgnurx) */
JITNINT PLATFORM_regcomp (regex_t* arg1, const char *arg2, JITNINT arg3) {
    return regcomp(arg1, arg2, arg3);
}

/* NOTE: this function is implemented in Windows by MinGW-REGEX libraries (mingw-libgnurx) */
const char* PLATFORM_re_compile_pattern (const char *arg1, size_t arg2, struct re_pattern_buffer *arg3) {
    return re_compile_pattern(arg1, arg2, arg3);
}

/* NOTE: this function is implemented in Windows by MinGW-REGEX libraries (mingw-libgnurx) */
reg_syntax_t  PLATFORM_re_set_syntax (reg_syntax_t arg1) {
    return re_set_syntax(arg1);
}


//=====================================  sched.h

/* NOTE: this function is implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_sched_yield (void) {
    return sched_yield();
}

/* NOTE: this function is implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_sched_get_priority_max (JITNINT arg1) {
    return sched_get_priority_max(arg1);
}

/* NOTE: this function is implemented in Windows by WinPthread32 libraries */
JITNINT PLATFORM_sched_get_priority_min (JITNINT arg1) {
    return sched_get_priority_min(arg1);
}

//=====================================  sys/utsname.h


JITNINT PLATFORM_getPlatformInfo (struct utsname *infos) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    /* This used in ildjit only as informational function by prints.
     * This is a secondary function.
     * No data taken from windows system, for now.
     * Detailed windows system data will be implemented in future versions.
     */

    strcpy(infos->sysname, "Not Available");
    strcpy(infos->nodename,"Not Available");
    strcpy(infos->release,"Not Available");
    strcpy(infos->version,"0.0.0 - Not Available");
    strcpy(infos->machine,"Not Available");

    /* if this function returns 0 means that no data is available */
    return 0;

#else           // Linux
    return uname(infos);
#endif
}


//=====================================  langinfo.h

//ex nl_langinfo

JITINT8* PLATFORM_getCurrentCodePageName (void) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    /* Not perfect but simple implementation. Best implementation requires moving codesets translation here,
     * making a JITINT32 getCurrentCodePage() function that returns directly the codePage.
     * See libiljitu/src/locale.c to see how this function is used.
     */
    char* codePageName;
    codePageName = malloc(30);
    strcpy(codePageName,"windows-");

    char codePage[10];

    itoa(GetACP(), codePage, 10);
    strcat(codePageName, codePage);

    return (JITINT8*) codePageName;

#else           // Linux
    return (JITINT8*) nl_langinfo(CODESET);
#endif
}


//=====================================  utime.h

/* NOTE for Windows platform: this function is supported by minGW, not Windows! */
JITNINT PLATFORM_utime (const char *path, const struct utimbuf *times) {
#ifdef WIN32    // Windows
    /* note: this cast is safe just in this function. */
    return utime(path, (struct utimbuf*) times);
#else           // Linux
    return utime(path,times);
#endif

}




//=====================================  dirent.h

/* NOTE for Windows platform: this function is supported by minGW. */
DIR* PLATFORM_openDir (const char *arg1) {
    return opendir(arg1);
}

/* NOTE for Windows platform: this function is supported by minGW.*/
JITNINT PLATFORM_closeDir (DIR *arg1) {
    return closedir(arg1);
}

/* NOTE for Windows platform: this function is supported by minGW.*/
struct dirent *PLATFORM_readDir (DIR *arg1) {
    return readdir(arg1);
}


//int PLATFORM_scandir (const char *arg1, struct dirent ***arg2, int (*arg3)(const struct dirent *), int (*arg4)(const struct dirent **, const struct dirent **)){
//#ifdef WIN32    // Windows
//#ifdef WINDEBUG
//	winDebugStart(__func__);
//#endif
//      int var;
//	winFuncNotSupported("scandir");
//	return 0;
//#else           // Linux
//	return scandir(arg1,arg2,arg3,arg4);
//#endif
//}


/* function taken from libGW32 and modified. */
//int SCANDIR (dir, namelist, select, cmp)
//     const char *dir;
//     DIRENT_TYPE ***namelist;
//     int (*select) (const DIRENT_TYPE *);
//     int (*cmp) (const void *, const void *);
//{
int PLATFORM_scandir (const char *dir, struct dirent ***namelist, int (*select)(const struct dirent *), int (*cmp)(const void *, const void *) ) {

#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    DIR *dp = opendir(dir);
    struct dirent **v = NULL;
    size_t vsize = 0, i;
    struct dirent *d;
    int save;
#define DIRENT_TYPE struct dirent
# define _D_EXACT_NAMLEN(d) ((d)->d_namlen)
# define _D_ALLOC_NAMLEN(d) (_D_EXACT_NAMLEN(d) + 1)

    if (dp == NULL) {
        return -1;
    }

    save = errno;
    //__set_errno (0);

    i = 0;
    while ((d = readdir(dp)) != NULL) {
        if (select == NULL || (*select)(d)) {
            struct dirent *vnew;
            size_t dsize;

            /* Ignore errors from select or readdir */
            //__set_errno (0);

            if (__builtin_expect(i == vsize, 0)) {
                struct dirent **new;
                if (vsize == 0) {
                    vsize = 10;
                } else {
                    vsize *= 2;
                }
                new = (struct dirent **) realloc(v, vsize * sizeof(*v));
                if (new == NULL) {
                    break;
                }
                v = new;
            }

            dsize = &d->d_name[_D_ALLOC_NAMLEN(d)] - (char *) d;
            vnew = (struct dirent *) malloc(dsize);
            if (vnew == NULL) {
                break;
            }

            v[i++] = (struct dirent *) memcpy(vnew, d, dsize);
        }
    }

    if (__builtin_expect(errno, 0) != 0) {
        save = errno;

        while (i > 0) {
            free(v[--i]);
        }
        free(v);

        i = -1;
    } else   {
        /* Sort the list if we have a comparison function to sort with.  */
        if (cmp != NULL) {
            qsort(v, i, sizeof(*v), cmp);
        }

        *namelist = v;
    }

    (void) closedir(dp);
    //__set_errno (save);

    return i;


#else           // Linux
    return scandir(dir, namelist, select, (int (*)(const struct dirent **, const struct dirent **))cmp);
#endif

}




//=====================================  dlfcn.h

#ifdef WIN32
/* The following code are dlfcn.h functions ported on Windows
 * by Ramiro Polla and released under the LGPL
 */

/* POSIX says these are implementation-defined.
 * To simplify use with Windows API, we treat them the same way.
 */
//#define RTLD_LAZY   0           /*EDIT: this is already been defined by minGW*/
//#define RTLD_NOW    0           /*EDIT: this is already been defined by minGW*/
#define RTLD_GLOBAL (1 << 1)
#define RTLD_LOCAL  (1 << 2)
/* These two were added in The Open Group Base Specifications Issue 6.
 * Note: All other RTLD_* flags in any dlfcn.h are not standard compliant.
 */
#define RTLD_DEFAULT    0
#define RTLD_NEXT       0
/* Note:
 * MSDN says these functions are not thread-safe. We make no efforts to have
 * any kind of thread safety.
 */
typedef struct global_object {
    HMODULE hModule;
    struct global_object *previous;
    struct global_object *next;
} global_object;

static global_object first_object;

/* These functions implement a double linked list for the global objects. */
static global_object *global_search ( HMODULE hModule ) {
    global_object *pobject;

    if ( hModule == NULL ) {
        return NULL;
    }

    for ( pobject = &first_object; pobject; pobject = pobject->next ) {
        if ( pobject->hModule == hModule ) {
            return pobject;
        }
    }

    return NULL;
}

static void global_add ( HMODULE hModule ) {
    global_object *pobject;
    global_object *nobject;

    if ( hModule == NULL ) {
        return;
    }

    pobject = global_search( hModule );

    /* Do not add object again if it's already on the list */
    if ( pobject ) {
        return;
    }

    for ( pobject = &first_object; pobject->next; pobject = pobject->next ) {
        ;
    }

    nobject = malloc( sizeof(global_object) );

    /* Should this be enough to fail global_add, and therefore also fail
     * dlopen?
     */
    if ( !nobject ) {
        return;
    }

    pobject->next = nobject;
    nobject->next = NULL;
    nobject->previous = pobject;
    nobject->hModule = hModule;
}

static void global_rem ( HMODULE hModule ) {
    global_object *pobject;

    if ( hModule == NULL ) {
        return;
    }

    pobject = global_search( hModule );

    if ( !pobject ) {
        return;
    }

    if ( pobject->next ) {
        pobject->next->previous = pobject->previous;
    }
    if ( pobject->previous ) {
        pobject->previous->next = pobject->next;
    }

    free( pobject );
}

/* POSIX says dlerror( ) doesn't have to be thread-safe, so we use one
 * static buffer.
 * MSDN says the buffer cannot be larger than 64K bytes, so we set it to
 * the limit.
 */
static char error_buffer[65535];
static char *current_error;

static int copy_string ( char *dest, int dest_size, const char *src ) {
    int i = 0;

    /* gcc should optimize this out */
    if ( !src && !dest ) {
        return 0;
    }

    for ( i = 0; i < dest_size-1; i++ ) {
        if ( !src[i] ) {
            break;
        } else {
            dest[i] = src[i];
        }
    }
    dest[i] = '\0';

    return i;
}

static void save_err_str ( const char *str ) {
    DWORD dwMessageId;
    DWORD pos;

    dwMessageId = GetLastError( );

    if ( dwMessageId == 0 ) {
        return;
    }

    /* Format error message to:
     * "<argument to function that failed>": <Windows localized error message>
     */
    pos = copy_string( error_buffer,     sizeof(error_buffer),     "\"" );
    pos += copy_string( error_buffer+pos, sizeof(error_buffer)-pos, str );
    pos += copy_string( error_buffer+pos, sizeof(error_buffer)-pos, "\": " );
    pos += FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId,
                          MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                          error_buffer+pos, sizeof(error_buffer)-pos, NULL );

    if ( pos > 1 ) {
        /* POSIX says the string must not have trailing <newline> */
        if ( error_buffer[pos-2] == '\r' && error_buffer[pos-1] == '\n' ) {
            error_buffer[pos-2] = '\0';
        }
    }

    current_error = error_buffer;
}

static void save_err_ptr_str ( const void *ptr ) {
    char ptr_buf[19]; /* 0x<pointer> up to 64 bits. */

    sprintf( ptr_buf, "0x%p", ptr );

    save_err_str( ptr_buf );
}

void *WIN_dlopen ( const char *file, int mode ) {
    HMODULE hModule;
    UINT uMode;

    current_error = NULL;

    /* Do not let Windows display the critical-error-handler message box */
    uMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    if ( file == 0 ) {
        /* POSIX says that if the value of file is 0, a handle on a global
         * symbol object must be provided. That object must be able to access
         * all symbols from the original program file, and any objects loaded
         * with the RTLD_GLOBAL flag.
         * The return value from GetModuleHandle( ) allows us to retrieve
         * symbols only from the original program file. For objects loaded with
         * the RTLD_GLOBAL flag, we create our own list later on.
         */
        hModule = GetModuleHandle( NULL );

        if ( !hModule ) {
            save_err_ptr_str( file );
        }
    } else {
        char lpFileName[MAX_PATH];
        JITNUINT i;

        /* MSDN says backslashes *must* be used instead of forward slashes. */
        for ( i = 0; i < (JITNUINT) sizeof(lpFileName)-1; i++ ) {
            if ( !file[i] ) {
                break;
            } else if ( file[i] == '/' ) {
                lpFileName[i] = '\\';
            } else {
                lpFileName[i] = file[i];
            }
        }
        lpFileName[i] = '\0';

        /* POSIX says the search path is implementation-defined.
         * LOAD_WITH_ALTERED_SEARCH_PATH is used to make it behave more closely
         * to UNIX's search paths (start with system folders instead of current
         * folder).
         */
        hModule = LoadLibraryEx( (LPSTR) lpFileName, NULL,
                                 LOAD_WITH_ALTERED_SEARCH_PATH );

        /* If the object was loaded with RTLD_GLOBAL, add it to list of global
         * objects, so that its symbols may be retrieved even if the handle for
         * the original program file is passed. POSIX says that if the same
         * file is specified in multiple invocations, and any of them are
         * RTLD_GLOBAL, even if any further invocations use RTLD_LOCAL, the
         * symbols will remain global.
         */
        if ( !hModule ) {
            save_err_str( lpFileName );
        } else if ( (mode & RTLD_GLOBAL) ) {
            global_add( hModule );
        }
    }

    /* Return to previous state of the error-mode bit flags. */
    SetErrorMode( uMode );

    return (void *) hModule;
}

int WIN_dlclose ( void *handle ) {
    HMODULE hModule = (HMODULE) handle;
    BOOL ret;

    current_error = NULL;

    ret = FreeLibrary( hModule );

    /* If the object was loaded with RTLD_GLOBAL, remove it from list of global
     * objects.
     */
    if ( ret ) {
        global_rem( hModule );
    } else {
        save_err_ptr_str( handle );
    }

    /* dlclose's return value in inverted in relation to FreeLibrary's. */
    ret = !ret;

    return (int) ret;
}

void *WIN_dlsym ( void *handle, const char *name ) {
    FARPROC symbol;

    current_error = NULL;

    symbol = GetProcAddress( handle, name );

    if ( symbol == NULL ) {
        HMODULE hModule;

        /* If the handle for the original program file is passed, also search
         * in all globally loaded objects.
         */

        hModule = GetModuleHandle( NULL );

        if ( hModule == handle ) {
            global_object *pobject;

            for ( pobject = &first_object; pobject; pobject = pobject->next ) {
                if ( pobject->hModule ) {
                    symbol = GetProcAddress( pobject->hModule, name );
                    if ( symbol != NULL ) {
                        break;
                    }
                }
            }
        }

        CloseHandle( hModule );
    }

    if ( symbol == NULL ) {
        save_err_str( name );
    }

    return (void*) symbol;
}

char *WIN_dlerror ( void ) {
    char *error_pointer = current_error;

    /* POSIX says that invoking dlerror( ) a second time, immediately following
     * a prior invocation, shall result in NULL being returned.
     */
    current_error = NULL;

    return error_pointer;
}

/* End of dlfcn.h functions ported on Windows by Ramiro Polla */

#endif



void  *PLATFORM_dlopen (const char *arg1, JITNINT arg2) {
#ifdef WIN32    // Windows
    return WIN_dlopen(arg1,arg2);
#else           // Linux
    return dlopen(arg1,arg2);
#endif
}

JITNINT PLATFORM_dlclose (void *arg1) {
#ifdef WIN32    // Windows
    return WIN_dlclose(arg1);
#else           // Linux
    return dlclose(arg1);
#endif
}

void  *PLATFORM_dlsym (void *arg1, const char *arg2) {
#ifdef WIN32    // Windows
    return WIN_dlsym(arg1,arg2);
#else           // Linux
    return dlsym(arg1,arg2);
#endif
}

JITINT8  *PLATFORM_dlerror (void) {
#ifdef WIN32    // Windows
    return (JITINT8*) WIN_dlerror();
#else           // Linux
    return (JITINT8*) dlerror();
#endif
}


//=====================================  libgen.h


char  *PLATFORM_dirname (char *path) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    char drive [256];
    char dir [252];

    char* outputPath;
    char buff[256];

    /* copy static input path var in writable buff var */
    strcpy(buff,path);

    /* if ther is a slash at the end of the path, remove it */
    if ( path[(strlen(path)-1)]== '\\' ) {
        buff[(strlen(buff)-1)] = '\0';
    }

    /* Get directory name */
    _splitpath(buff, drive, dir, NULL, NULL);
    outputPath = strcat(drive, dir);

    /* remove last '\' char only if you are not in the root drive (ex. 'C:\') */
    if (strcmp(dir,"\\")) {
        outputPath[(strlen(outputPath)-1)] = '\0';
    }

    return outputPath;

#else           // Linux
    return dirname(path);
#endif
}




//================================================

JITINT8* PLATFORM_getUserName (void) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    TCHAR* infoBuf = malloc(INFO_BUFFER_SIZE);
    DWORD bufCharCount = INFO_BUFFER_SIZE;
    bufCharCount = INFO_BUFFER_SIZE;
    GetUserName( infoBuf, &bufCharCount );
    // returns char* or wchar_t depending on the text coding (ANSI VS UNICODE)
    return (JITINT8*) infoBuf;

#else           // Linux
    return (JITINT8*) cuserid(NULL);
#endif
}


/* NOTE for Windows platform: this function is supported by minGW.*/
JITNINT PLATFORM_getOptArg (int argc, char * const argv[], const char *optstring, const struct option *longopts, int *longindex) {
    return getopt_long(argc, argv, optstring, longopts, longindex);
}




JITINT8 * PLATFORM_getUserDir (void) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    //The following function is supported by ShFolder.dll
    //SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, outputString
    winFuncNotSupported("getUserDir");
    return NULL;
#else           // Linux
    return (JITINT8 *)getenv("HOME");
#endif
}




int PLATFORM_posix_memalign (void **memptr, size_t alignment, size_t size) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    (*memptr) = __mingw_aligned_malloc( size, alignment  );
    if ((*memptr)!=NULL) {
        return 0;       /* ok */
    } else                                  {
        return -1;      /* error */
    }
#else                           // Linux
    return posix_memalign(memptr, alignment, size);
#endif
}




long int PLATFORM_getThisThreadId (void) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    pthread_t thisThread;
    thisThread = PLATFORM_getSelfThreadID();
    return (long int) thisThread.p;
#else           // Linux
    return syscall(SYS_gettid);
#endif
}



long PLATFORM_getProcessorsNumber () {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    return (long) siSysInfo.dwNumberOfProcessors;

#else           // Linux
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}


char* PLATFORM_strsep (char **stringp, const char *delim) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    // richiede forse niente
    // includes: stdio.h

    char *s;
    const char *spanp;
    int c, sc;
    char *tok;

    if ((s = *stringp) == NULL) {
        return NULL;
    }
    for (tok = s;; ) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0) {
                    s = NULL;
                } else {
                    s[-1] = 0;
                }
                *stringp = s;
                return tok;
            }
        } while (sc != 0);
    }

#else           // Linux
    return strsep(stringp, delim);
#endif
}



bool isSymbolicLink (char* filePath) {
#ifdef WIN32    // Windows
    if ( GetFileAttributes(filePath) == FILE_ATTRIBUTE_REPARSE_POINT) {
        return true;
    } else {
        return false;
    }
#else           // Linux
    struct stat st;
    if (PLATFORM_lstat((char *) filePath, &st) >= 0) {
        if (S_ISLNK(st.st_mode)) {
            return true;
        }
    }
    return false;

#endif
}


/* Returns ture if the file si a Symbolic Link */
/* questa funzione pu� essere chiamata isSymbolicLink(path)
   linux: dal path prendi la mode con stat() e ritorni la S_ISLNK(mode)
   win: fai un test sulla getFileAttributes(path)
   FUNZIONE DA RISCRIVERE: iljit/internal_calls_io:1843
   Seve una funzione che riestituisca il tipo di file dato il path.
   per tipi di files che si ottenfono con la GetFileAttributes: http://msdn.microsoft.com/en-us/library/gg258117%28v=vs.85%29.aspx

    isSymbolicLink
    if S_ISLNK(info.mode) then         FILE_ATTRIBUTE_REPARSE_POINT
      Writeln ('File is a link');

      isRegularFile
    if S_ISREG(info.mode) then         S_ISREG(m)
      Writeln ('File is a regular file'); FILE_ATTRIBUTE_NORMAL

      isDirectory
    if S_ISDIR(info.mode) then            S_ISDIR(m)
      Writeln ('File is a directory');   FILE_ATTRIBUTE_DIRECTORY

      isCharDeviceFile
    if S_ISCHR(info.mode) then           S_ISCHR(m)
      Writeln ('File is a character device file');               S_IFCHR

      isBlockDeviceFile
    if S_ISBLK(info.mode) then      S_ISBLK(m)
      Writeln ('File is a block device file');                   S_IFBLK

      isFifoFile
    if S_ISFIFO(info.mode) then
      Writeln ('File is a named pipe (FIFO)');    S_ISFIFO(m)

      isSocketFile
    if S_ISSOCK(info.mode) then
      Writeln ('File is a socket');


   WIN: controlla mode.st_mode con header di mingw

 */
bool PLATFORM_S_ISLNK (unsigned int mode) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    // richiede kernel32.dll
    // includes: winbase.h

    JITBOOLEAN result;
    DWORD dwAttrs;
    dwAttrs = GetFileAttributes((LPCSTR) mode);
    if (dwAttrs & FILE_ATTRIBUTE_REPARSE_POINT) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
#else           // Linux
    return S_ISLNK(mode);
#endif
}


char *PLATFORM_get_current_dir_name (void) {
#ifdef WIN32    // Windows
    /* NOTE: This function requires kernel32.dll and winbase.h include.*/
#define BUFSIZE 256

    TCHAR buffer[BUFSIZE];
    int stringSize;
    char* out;

    /* get current dir (NULL terminated string) */
    stringSize = GetCurrentDirectory(BUFSIZE, buffer);

    /* Add the terminator */
    buffer[stringSize] = '\0';

    /* allocate the string into heap */
    out = malloc(BUFSIZE);
    strcpy( out, buffer);

    if ( stringSize == 0 ) {
        return NULL;
    } else {
        return out;
    }
#else           // Linux
    return get_current_dir_name();
#endif
}

/* rmtp is the remaining time in case of unexpected wakeup, not used in WIN. Probably not used in final API version */
int PLATFORM_nanosleep (const struct timespec *rqtp, struct timespec *rmtp) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif
    (void) rmtp;        /* to be removed here and in the arg */

    long wtime = (rqtp->tv_sec*1000) + (rqtp->tv_nsec);

    Sleep(wtime);
    return 0;

#else           // Linux
    return nanosleep(rqtp, rmtp);
#endif
}


int PLATFORM_clock_gettime (clockid_t clk_id, struct timespec *tp) {
#ifdef WIN32    // Windows
#ifdef WINDEBUG
    winDebugStart(__func__);
#endif

    if (clk_id == CLOCK_MONOTONIC) {
        long years, months, days, hours, minutes, seconds, total;
        SYSTEMTIME st;
        GetSystemTime(&st);
        years = ((st.wYear)-1970)*31556926;
        months = (st.wMonth)*2629744;
        days = (st.wDay)*86400;
        hours = (st.wHour)*3600;
        minutes = (st.wMinute)*60;
        seconds = st.wSecond;
        total = years + months + days + hours + minutes + seconds;

        tp->tv_sec = total;
        tp->tv_nsec = 0;
        return 0;
    } else {
        /*   The clk_id specified is not supported on this system. */
        return EINVAL;
    }

#else           // Linux
    return clock_gettime(clk_id, tp);
#endif
}


JITINT32 PLATFORM_fflush (FILE *file) {
    return fflush(file);
}

JITINT32 PLATFORM_fsync (FILE *file) {
#ifdef WIN32 // Windows
    /* note: to flush all the data out of file buffer and disk buffer,
       you also need to CreateFile() file with FILE_FLAG_WRITE_THROUGH flag. */
    return (JITINT32) FlushFileBuffers(file);
#else           // Linux
    JITINT32 id;
    id = fileno(file);
    return fsync(id);
#endif
}


JITNINT PLATFORM_chmod (const char *path, mode_t mode) {

    return chmod(path,mode);

}


JITINT32 PLATFORM_fchmod (JITNINT fd, mode_t mode) {
#ifdef WIN32  // Windows

    char* buf;

    /* get the filename from file descriptor */
    GetFileInformationByHandleEx (fd, 2,buf,256);

    return chmod(buf,mode);

#else           // Linux
    return fchmod(fd,mode);
#endif
}


void  *PLATFORM_mmap(void * ptr, size_t size, int prot, int type, int handle, off_t arg) {
#ifdef WIN32  // Windows

    /*
    //NOTE: This could be a better implemetation, but does not work on Windows 64bit in Debug mode by Windows bug
    HANDLE mmaph=CreateFileMapping(fd, 0, PAGE_WRITECOPY, 0, filesize, 0);
    void *page=MapViewOfFile(mmaph, FILE_MAP_COPY, 0, position, size);
    // for unmap:
    UnmapViewOfFile();
    CloseHandle();
    */
    static long g_pagesize;
    static long g_regionsize;

    /* First time initialization */
    if (! g_pagesize) {
        g_pagesize = PLATFORM_getSystemPageSize ();
    }
    /* Allocate this */
    ptr = VirtualAlloc (ptr, size,
                        MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
    if (! ptr) {
//        /* if error, return a handle to the existing object */
//        ptr = (void*) handle;
        ptr = (void*) MAP_FAILED;
        goto mmap_exit;
    }
mmap_exit:
    return ptr;


#else           // Linux
    return mmap(ptr, size, prot, type, handle, arg);
#endif
}


int  PLATFORM_munmap(void *ptr, size_t size) {
#ifdef WIN32  // Windows

    static long g_pagesize;
    static long g_regionsize;
    int rc = -1;

    /* First time initialization */
    if (! g_pagesize) {
        g_pagesize = PLATFORM_getSystemPageSize();
    }

    /* Free this */
    if (! VirtualFree (ptr, 0, MEM_RELEASE)) {
        goto munmap_exit;
    }
    rc = 0;
munmap_exit:
    return rc;

#else           // Linux
    return munmap(ptr, size);
#endif
}


long int PLATFORM_getSystemPageSize()	{
#ifdef WIN32                    // Windows
    static long pageSize = 0;

    if (! pageSize) {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        pageSize = systemInfo.dwPageSize;
    }
    return pageSize;

#else                           // Linux
    return getpagesize();
    // OR: return sysconf(_SC_PAGESIZE);
#endif
}



/* other implementation: http://stackoverflow.com/questions/6601862/query-thread-not-process-processor-affinity
*/
int PLATFORM_pthread_getaffinity_np (pthread_t thread, size_t cpusetsize,  cpu_set_t* cpuAffinityMask)	{

#ifdef WIN32                    // Windows
    DWORD_PTR thrAffinity;
    HANDLE winThread = GetCurrentThread(); // <== DELETE THIS
//HANDLE winThread = pthread_getw32threadhandle_np(thread);  // <-- USE THIS

    thrAffinity = SetThreadAffinityMask(winThread, *(cpuAffinityMask));
    SetThreadAffinityMask(winThread, thrAffinity);

    *(cpuAffinityMask) = thrAffinity;

#ifdef WINDWBUG
    printf("getThreadAffinityMask returned: %b", thrAffinity);
#endif

    return thrAffinity;

#else                           // Linux
    return pthread_getaffinity_np( thread, cpusetsize, cpuAffinityMask);
#endif

}



int PLATFORM_pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t* cpuAffinityMask)	{

#ifdef WIN32                    // Windows
    (void) cpusetsize;
    HANDLE winThread = GetCurrentThread(); // <== DELETE THIS
//HANDLE winThread = pthread_getw32threadhandle_np(thread);  // <-- USE THIS

    if( SetThreadAffinityMask( winThread, (*cpuAffinityMask))  != 0 ) {
        return  0;    // ok
    } else {
        return -1;    // error
    }
#else                           // Linux
    if (cpuAffinityMask != 0) {
        return pthread_setaffinity_np( thread, cpusetsize, cpuAffinityMask);
    } else {
        return -1;
    }
#endif

}


void PLATFORM_CPU_ZERO(cpu_set_t* cpuMask)	{
#ifdef WIN32                    // Windows
    *(cpuMask) = 0;
#else                           // Linux
    CPU_ZERO(cpuMask);
#endif
}

/* se cpu=0 significa il primo CPU o nessun CPU????? CONTROLLA. Per ora, fai primo CPU*/
void PLATFORM_CPU_SET(int cpu, cpu_set_t* cpuMask)	{
#ifdef WIN32                    // Windows

    /* pow() function error in mingw: calculate mask manually */
    int cpuPow2=2;
    if(cpu==0) {
        cpuPow2 = 1;
    } else if (cpu==1) {
        cpuPow2 = 2;
    } else {
        while(cpu>1) {
            cpuPow2=2*cpuPow2;
            cpu--;
        }
    }
    (*cpuMask) = (*cpuMask) | cpuPow2;   /* bitwise OR */
#else                           // Linux
    CPU_SET(cpu, cpuMask);
#endif
}



/* 0: is not set
 * != 0: is set
 */
int PLATFORM_CPU_ISSET(int cpu, cpu_set_t* cpuMask)	{
#ifdef WIN32                    // Windows
    /* pow() function error in mingw: calculate mask manually */
    int cpuPow2=2;
    if(cpu==0) {
        cpuPow2 = 1;
    } else if (cpu==1) {
        cpuPow2 = 2;
    } else {
        while(cpu>1) {
            cpuPow2=2*cpuPow2;
            cpu--;
        }
    }
    /* bitwise AND: if result=0 is not set, if !=0 is set */
    return (*cpuMask) & cpuPow2;

#else                           // Linux
    return CPU_ISSET(cpu, cpuMask);
#endif

}


/* to be fully tested */
int PLATFORM_CPU_CLR(int cpu, cpu_set_t* cpuMask)	{
#ifdef WIN32                    // Windows
    /* pow() function error in mingw: calculate mask manually */
    int cpuPow2=2;
    if(cpu==0) {
        cpuPow2 = 1;
    } else if (cpu==1) {
        cpuPow2 = 2;
    } else {
        while(cpu>1) {
            cpuPow2=2*cpuPow2;
            cpu--;
        }
    }
    /* bitwise NOT(cpuToRemove) AND cpuMask: */
    return  ~cpuPow2 & (*cpuMask);
#else                           // Linux
    return CPU_ISSET(cpu, cpuMask);
#endif

}





int PLATFORM_mkfifo (const char *path, mode_t mode)	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return mkfifo (path, mode);
#endif
}


/* http://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html */
ssize_t PLATFORM_getdelim(char **lineptr, size_t *n, int delimiter, FILE *stream)	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return getdelim(lineptr, n, delimiter, stream);
#endif
}

/* http://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html */
ssize_t PLATFORM_getline(char **lineptr, size_t *n, FILE *stream)	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return getline(lineptr, n, stream);
#endif
}



pid_t PLATFORM_fork()	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return fork();
#endif
}

pid_t PLATFORM_waitpid(pid_t pid, int *stat_loc, int options)	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return waitpid(pid, stat_loc, options);
#endif
}

/* Used only for print status
returns not zero if the child exited normally
* http://publib.boulder.ibm.com/infocenter/tpfhelp/current/index.jsp?topic=%2Fcom.ibm.ztpf-ztpfdf.doc_put.cur%2Fgtpc2%2Fcpp_wifexited.html */
int PLATFORM_WIFEXITED(int status)	{
#ifdef WIN32                    // Windows
    if (status == EXIT_SUCCESS) {
        return 1;
    } else {
        return 0;
    }

#else                           // Linux
    return WIFEXITED(status);
#endif
}

/* Used only for print status
 * http://publib.boulder.ibm.com/infocenter/tpfhelp/current/index.jsp?topic=%2Fcom.ibm.ztpf-ztpfdf.doc_put.cur%2Fgtpc2%2Fcpp_wexitstatus.html */
/* WIFEXITED is true of status, this macro returns the low-order 8 bits of the exit status value from the child process */
int PLATFORM_WEXITSTATUS(int status)	{
#ifdef WIN32                    // Windows
    return status;
#else                           // Linux
    return WEXITSTATUS(status);
#endif
}


/*
// Other mingW-indipendent implementation:
 http://msdn.microsoft.com/en-us/library/windows/desktop/ms682653%28v=vs.85%29.aspx
*/
int PLATFORM_setenv(const char *envVarName, const char *envVarValue, int overwrite)	{
#ifdef WIN32                    // Windows
    char buffer[2048];

    if(!getenv(envVarName) || (overwrite!=0)) {
        strcpy(buffer,envVarName);
        strcat(buffer,"=");
        strcat(buffer,envVarValue);

        return putenv(&buffer);
    }

    return -1;

#else                           // Linux
    return setenv(envVarName, envVarValue, overwrite);
#endif
}



char* PLATFORM_getenv(const char *envVarName)	{
    return getenv(envVarName);
}

/*
       http://www.kernel.org/doc/man-pages/online/pages/man3/clearenv.3.html
*/


int PLATFORM_clearenv(void)	{
#ifdef WIN32                    // Windows

    WinFuncNotSupported("clearenv");

#else                           // Linux
    return clearenv();
#endif
}








/* http://pubs.opengroup.org/onlinepubs/009604499/functions/setgid.html */

int PLATFORM_setgid(gid_t gid)	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return setgid(gid);
#endif
}


int PLATFORM_setuid(uid_t uid)	{
#ifdef WIN32                    // Windows



#else                           // Linux
    return setuid(uid);
#endif
}


