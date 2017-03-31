/*
 * Copyright (C) 2010 - 2014 Simone Campanoni
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

/**
 * \defgroup FILES Files
 * \ingroup PLATFORM_API
 *    \defgroup FILES_BASE Files functions
 *       \ingroup FILES
 *    \defgroup FILES_UTILITIES File info - Utilities
 *      \ingroup FILES
 *    \defgroup FILES_DIR Directories
 *       \ingroup FILES
 *    \defgroup FILES_WORKINGDIR Working Directory
 *       \ingroup FILES
 *    \defgroup FILES_SYMLINKS Symbolic links
 *       \ingroup FILES
 *
 * \defgroup SYSINFO System & Platform Info
 * \ingroup PLATFORM_API
 *
 * \defgroup DYNLINKS Dynamic linking
 * \ingroup PLATFORM_API
 *
 * \defgroup REGEX Regular Expressions
 * \ingroup PLATFORM_API
 *
 * \defgroup THREADS Threads
 * \ingroup PLATFORM_API
 *    \defgroup THREADS_BASE	Threads functions
 *      \ingroup THREADS
 *    \defgroup THREADS_SCHED	Scheduling Management
 *      \ingroup THREADS
 *    \defgroup THREADS_ATTR Thread Attributes
 *      \ingroup THREADS
 *    \defgroup THREADS_MUTEX Mutex
 *       \ingroup THREADS
 *    \defgroup THREADS_CONDVAR  Condition Variables
 *       \ingroup THREADS
 *
 *
 * \defgroup MISC Miscellaneous
 * \ingroup PLATFORM_API
 *
 */

/** \file platform_API.h
 *  @brief This file contains all the functions written to gain multi-platform support to ILDJIT.
 *
 * Currenlty supported platforms are Linux and Windows 7 (or newer)
 *
 */

#ifndef PLATFORM_API_H
#define PLATFORM_API_H

#ifdef __cplusplus
extern "C" {
#endif

// My headers:
#include <jitsystem.h>

#ifdef WIN32
// WIN32 Header Files:
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include <windows.h>
#include <process.h> // requires to include for exit() function


/* Header files supported by MinGW */
#include <getopt.h>
#include <dirent.h>
#include <utime.h>
#include <sys/stat.h>
#include <malloc.h>


/* WinPthread32 header: */
#include <pthread.h>

/* MinGW Regex Header: */
#include <regex.h>



#else
// LINUX Header files:
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <regex.h>
#include <sched.h>
#include <alloca.h>  //supported by minGW, not supported by libplatform
#include <sys/mman.h>
#include <sys/utsname.h>
#include <langinfo.h>
#include <utime.h>
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <pwd.h>
#include <getopt.h>
#include <sys/syscall.h>    // For SYS_xxx definitions used in iljit/src/system_manager.c riga 1058*/

#include <sys/wait.h>
#include <mcheck.h>

#include <sys/mman.h>
#include <sys/user.h>

#include <sys/types.h>

#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/select.h>


#endif



/* USEFUL DEFINES NEEDED BY OTHER LIBS: */
#ifdef WIN32 // WIN32 Header Files:

#define WIN_DEFAULT_PREFIX "C:\\MinGW\\msys\\1.0\\local"


#define F_RDLCK              0       /* Read lock.  */
#define F_WRLCK              1       /* Write lock.	*/
#define F_UNLCK              2       /* Remove lock.	 */



/* errno not supported by minGW, needed by XanLib:  */
/* needed by XanLib:  */
#define      EMSGSIZE                90      /* Message too long */
#define      ECONNREFUSED    111             /* Connection refused */
#define      ENOTCONN                107     /* Transport endpoint is not connected */


/* needed by libiljitu*/
#define clockid_t int        /* taken from time.h */
#define CLOCK_MONOTONIC 1    /* taken from time.h */

/* needed by libiljitiroptimizer, taken by dlfcn.h */
#define RTLD_NOW     0x00002 /* Immediate function call binding.  */ /* This is the MODE argument to `dlopen' */

/* needed bt libmanfred */

/* Protection bits.  */
#define __S_ISUID       04000   /* Set user ID on execution.  */
#define __S_ISGID       02000   /* Set group ID on execution.  */
#define __S_ISVTX       01000   /* Save swapped text after use (sticky).  */
#define __S_IREAD       0400    /* Read by owner.  */
#define __S_IWRITE      0200    /* Write by owner.  */
#define __S_IEXEC       0100    /* Execute by owner.  */


# define S_ISUID        __S_ISUID       /* Set user ID on execution.  */
# define S_ISGID        __S_ISGID       /* Set group ID on execution.  */

# if defined __USE_BSD || defined __USE_MISC || defined __USE_XOPEN
/* Save swapped text after use (sticky bit).  This is pretty well obsolete.  */
#  define S_ISVTX       __S_ISVTX
# endif

//		# define S_IRUSR	__S_IREAD       /* Read by owner.  */
//		# define S_IWUSR	__S_IWRITE      /* Write by owner.  */
//		# define S_IXUSR	__S_IEXEC       /* Execute by owner.  */
//		/* Read, write, and execute by owner.  */
//		# define S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)

# define S_IRGRP        (S_IRUSR >> 3)  /* Read by group.  */
# define S_IWGRP        (S_IWUSR >> 3)  /* Write by group.  */
# define S_IXGRP        (S_IXUSR >> 3)  /* Execute by group.  */
/* Read, write, and execute by group.  */
# define S_IRWXG        (S_IRWXU >> 3)

# define S_IROTH        (S_IRGRP >> 3)  /* Read by others.  */
# define S_IWOTH        (S_IWGRP >> 3)  /* Write by others.  */
# define S_IXOTH        (S_IXGRP >> 3)  /* Execute by others.  */
/* Read, write, and execute by others.  */
# define S_IRWXO        (S_IRWXG >> 3)


// #define S_IRWXG	((__S_IREAD|__S_IWRITE|__S_IEXEC) >> 3)
//  #define S_IRWXG     (( 0400 | 0200 | 0100 ) >> 3)	/* TO DO: Set here the right value */
//  #define S_IRGRP	(0400 >> 3)                             /* Read by group.  */
//  #define S_IROTH	(S_IRGRP >> 3)                                  /* Read by others.  */
//  #define S_IXOTH     (( 0400 | 0200 | 0100 ) >> 3)	/* TO DO: Set here the right value */



/* needed by iljit */
#define _SC_NPROCESSORS_ONLN         0       /* Probably not needed in windows. Constant needed by sysconf in linux to get processors number. */


///* Needed by iljit, taken from fcntl.h */
//   #define O_ACCMODE            0003
//   #define O_RDONLY             00      /*used by iljit*/
//   #define O_WRONLY             01
//   #define O_RDWR               02      /*used by iljit*/
//   #define O_CREAT              0100    /* not fcntl */	/*used by iljit*/
//   #define O_EXCL               0200    /* not fcntl */	/*used by iljit*/
//   #define O_NOCTTY             0400    /* not fcntl */
//   #define O_TRUNC              01000   /* not fcntl */	/*used by iljit*/
//   #define O_APPEND             02000   /*used by iljit*/
//   #define O_NONBLOCK           04000
//   #define O_NDELAY             O_NONBLOCK
//   #define O_SYNC               04010000
//   #define O_FSYNC              O_SYNC
//   #define O_ASYNC              020000


/* Needed by iljit, taken from dlfcn.h */
/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY            0x00001 /* Lazy function call binding.  */
#define RTLD_NOW             0x00002 /* Immediate function call binding.  */
#define RTLD_BINDING_MASK    0x3     /* Mask of binding time value.  */
#define RTLD_NOLOAD          0x00004 /* Do not load the object.  */
#define RTLD_DEEPBIND        0x00008 /* Use deep binding.  */

/* Needed by mmap() and unmmap() functions: theese costants will be used differently in windows.
 * See the PLATFORM_mmap() windows implementation function for more info */
#define PROT_EXEC		0
#define PROT_READ		0
#define PROT_WRITE		0
#define MAP_SHARED		0
#define MAP_ANONYMOUS	0


#define PAGE_SIZE PLATFORM_getSystemPageSize()
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif

#endif



#ifdef WIN32 // WINDOWS32 STRUCTS:

/* file locks info structs (see fcntl.h)  */
struct flock {
    short int l_type;       /* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
    short int l_whence;     /* Where `l_start' is relative to (like `lseek').  */
#ifndef __USE_FILE_OFFSET64
    long int l_start;       /* Offset where the lock begins.  */
    long int l_len;         /* Size of the locked area; zero means until EOF.  */
#else
    long long int l_start;  /* Offset where the lock begins.  */
    long long int l_len;    /* Size of the locked area; zero means until EOF.  */
#endif
    int l_pid;              /* Process holding the lock.  */
};


/* Structure needed by getPlatformInfo (uname) describing the system and machine. Same as Linux's "sys/utsname.h" */
#define _UTSNAME_LENGTH 65
struct utsname {
    /* Name of the implementation of the operating system.  */
    char sysname[_UTSNAME_LENGTH];

    /* Name of this node on the network.  */
    char nodename[_UTSNAME_LENGTH];                                 /* not used in ildjit */

    /* Current release level of this implementation.  */
    char release[_UTSNAME_LENGTH];
    /* Current version level of this release.  */
    char version[_UTSNAME_LENGTH];

    /* Name of the hardware type the system is running on.  */
    char machine[_UTSNAME_LENGTH];

#if _UTSNAME_DOMAIN_LENGTH - 0
    /* Name of the domain of this node on the network.  */
# ifdef __USE_GNU
    char domainname[_UTSNAME_DOMAIN_LENGTH];
# else
    char __domainname[_UTSNAME_DOMAIN_LENGTH];
# endif
#endif
};




/* winPthread32 header */
#include <pthread.h>
/* */



/* this typedef is temporary. This is Linux netdb.h type.
   There is no network implementation under windows,
   this struct permit compilation of libclimanager/src/lib_net.h:145 */
struct ip_mreq {
};



#define cpu_set_t DWORD_PTR // Windows Affinity Mask


#define uid_t unsigned int
#define gid_t unsigned int

#endif

// TO DO: change CHAR in JIT Types declared in jitsystem.h (char = JITINT8)

// Examples of signature
//	JITBOOLEAN PLATFORM_openFile (char *fileName);
//      JITUINT32 PLATFORM_writeToFile (...);
//      JITUINT32 PLATFORM_readFromFile (...);




/**
 *
 * @name Files
 *  All the functions needed to operate with filesystem.
 *
 */
/**@{*/

/**
 * \ingroup FILES_BASE
 * @brief Open a File.
 *
 * @param  fileName 	The name of the file.
 * @param  modes 		Open file mode.
 * @param  permission 	file permission. Used only in Linux.
 *
 * @return returns File Descriptor, -1 if error.
 */
JITINT32 PLATFORM_open (const char *file, int modes, int permission);

/**
 * \ingroup FILES_BASE
 * @brief Fetch size of a File.
 *
 * @param file File to consider
 * @return Bytes of data stored in the specified file
 */
JITUINT64 PLATFORM_sizeOfFile (FILE *file);

/**
 * \ingroup FILES_BASE
 *
 * @brief  Close a File.
 *
 * @param  fileName 	The name of the file.
 *
 * @return returns 0 if ok, -1 if error.
 */
JITNINT PLATFORM_close (JITNINT arg1);

/**
 * \ingroup FILES_BASE
 *
 * @brief Read nbytes from file.
 *
 * @param	fileDescriptor	The file descriptor of the file.
 * @param	buff		pointer to buffer where the nbytes read are stored.
 * @param 	nbytes		Number  of bytes to read.
 *
 * @return returns non-negative integer indicating the number of bytes actually read. Otherwise, the function returns -1.
 */
ssize_t PLATFORM_read (JITNINT arg1, void* arg2, size_t arg3);

/**
 * \ingroup FILES_BASE
 *
 * @brief Writes nbytes to file.
 *
 * @param	fileDescriptor	File descriptor of the file.
 * @param	buff		pointer to buffer where the nbytes read are stored.
 * @param 	nbytes		Number  of bytes to read.
 *
 * @return non-negative integer indicating the number of bytes actually written. Otherwise, the function returns -1.
 */
ssize_t PLATFORM_write (JITNINT arg1, const void* arg2, size_t arg3);

/**
 * \ingroup FILES_BASE
 *
 * @brief Move the read/write file offset.
 *
 * (See lseek())
 *
 * @param	fileDescriptor	File descriptor of the file.
 * @param	offset		offset (in bytes) to set
 * @param 	whence		SEEK_SET (filestart + offset), SEEK_CUR (current location + offset), SEEK_END (filesize + offset)
 *
 * @return Returns the offset in number of bytes from the beginning of the file. Otherwise, the function returns -1.
 */
off_t  PLATFORM_lseek (JITNINT arg1, off_t arg2, JITNINT arg3);

/**
 * \ingroup FILES_BASE
 * @brief Permit reading and writing memory like it would be a file.
 *
 * (See fmemopen())
 *
 * @param	buf		Start memory address
 * @param	size		Maximum size allowed to read/write
 * @param 	mode		Mode of opening the file (e.g., "r")
 * @return File pointer.
 */
FILE * PLATFORM_fmemopen (void *buf, size_t size, JITINT8 *mode);

/**
 * \ingroup FILES_BASE
 *
 * @brief Truncate a file to a specified length.
 *
 * (see ftruncate())
 *
 * @param	fileDescriptor	File descriptor of the file.
 * @param	length		position of the file cut.
 *
 * @return 0 if successful. Otherwise, the function returns -1.
 */
JITNINT PLATFORM_ftruncate (JITNINT fd, off_t length);

/**
 * \ingroup FILES_BASE
 *
 * @brief Synchronize changes to a file.
 *
 *  (see fsync())
 * All data for the open file descriptor named by fildes is to be transferred to the associated storage device.
 *
 * @param	fileDescriptor	File descriptor of the file.
 *
 * @return 0 if successful. Otherwise, the function returns -1.
 */
JITINT32 PLATFORM_fsync (FILE *file);

/**
 * \ingroup FILES_BASE
 *
 * @brief Flush a stream.
 *
 * (see fsync())
 *
 * @param	fileDescriptor	File descriptor of the file.
 *
 * @return 0 if successful. Otherwise, the function returns ERRNO.
 */
JITINT32 PLATFORM_fflush (FILE *file);



//==================== File info / utilities
/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Test if file exists.
 *
 * @param filename	Name of the file.
 *
 * @return JITTRUE if file exists. Otherwise, returns JITFALSE .
 */
JITBOOLEAN PLATFORM_doesFileExist (JITINT8 *fileName);
/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Test for a terminal device.
 *
 * @param fileDescriptor	File Descriptor of the file.
 *
 * @return JITTRUE if fileDescriptor is associated with a terminal device. Otherwise, returns JITFALSE.
 */
JITNINT PLATFORM_isatty (JITNINT arg1);

/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Lock or unlock file in specified byterange.
 *
 *  Lock or unlock file in byterange specified in cntrl_data struct (see fcntl() with F_SETLKW for details).
 *  Set cntl_data->l_start for postion and cntl_data->l_len for the length of the file segment to lock.
 *
 * @param fileHandler	File Descriptor of the file.
 * @param cntl_data		Position and legnth of the segment to lock.
 *
 * @return If success, value different than -1 is returned. Otherwise, returns -1.
 */
JITNINT PLATFORM_setFileLock ( JITNINT fileHandler, struct flock *cntl_data );

/**
 * \ingroup FILES_UTILITIES
 *
 *  @brief Set file access and modification times.
 *
 * @param path		Path of the file.
 * @param times		Access and modification time to set.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_utime (const char *arg1, const struct utimbuf *arg2);

/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Change mode of a file from path.
 *
 * @param path		Path of the file.
 * @param mode		Mode to set.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_chmod (const char *path, mode_t mode);
/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Change mode of a file from file descriptor.
 *
 * @param fd		File Descriptor of the file.
 * @param mode		Mode to set.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITINT32 PLATFORM_fchmod (JITNINT fd, mode_t mode);

/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Get file status from path.
 *
 * @param path		Path of the file.
 * @param buff		pointer to an area where obtained information are stored.
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_stat (const char *arg1, struct stat *arg2);

/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Get file status from file descriptor.
 *
 * @param fileDescriptor	fileDescriptor of the file.
 * @param buff		pointer to an area where obtained information are stored.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_fstat (JITNINT arg1, struct stat *arg2);

/**
 * \ingroup FILES_UTILITIES
 *
 * @brief Get symbolicLink status.
 *
 * @param fileDescriptor	fileDescriptor of the file.
 * @param buff		pointer to an area where obtained information are stored.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_lstat (const char * path, struct stat * stat);


// ================  Directories
/**
 * \ingroup FILES_DIR
 *
 * @brief Make a new directory in current working directory.
 *
 * @param path		the name of the new directory.
 * @param mode		permission mask of the new directory. Not used in Windows.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_mkdir (const char * arg1, mode_t arg2);

/**
 * \ingroup FILES_DIR
 *
 * @brief Set and get permission mask for file or directory mode creation.
 *
 * (see umask()). Needed by PLATFORM_makeDir().
 * NOTE for Windows: Default read/write permission is returned. Not necessary for windows directories.
 *
 * @param newMask		Permission mask to be set.
 *
 * @return previous value of the file mode creation mask is returned
 */
mode_t PLATFORM_umask (mode_t newMask);
/**
 * \ingroup FILES_DIR
 *
 * @brief Remove a single directory.
 *
 * @param path		path of the directory to be removed.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_rmdir (const JITINT8* arg1);
/**
 * \ingroup FILES_DIR
 *
 * @brief  Remove a directory and all its subdirectories.
 *
 * @param dir		path of the directory to be removed.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_rmdir_recursively (const JITINT8 *dir);
/**
 * \ingroup FILES_DIR
 *
 * @brief Open a direcory stream.
 *
 * @param path		path of the directory to be removed.
 *
 * @return DIR* pointer to opended dir is returned. Otherwise, returns NULL.
 */
DIR*    PLATFORM_openDir (const char *arg1);
/**
 * \ingroup FILES_DIR
 *
 * @brief Close a direcory stream.
 *
 * @param dir		directory stream to be closed.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_closeDir (DIR *arg1);

/**
 * \ingroup FILES_DIR
 *
 * @brief Reads the content of a directory.
 *
 * The content is stored in dirent data structure.
 *
 * @param dir		directory stream to be read.
 *
 * @return If successful, returns a pointer to an object of type struct dirent. Otherwise, returns NULL.
 */
struct dirent *PLATFORM_readDir (DIR *arg1);

/**
 * \ingroup FILES_DIR
 *
 * @brief Get the parent directory name of a file pathname.
 *
 * @param path		pathname of the file.
 *
 * @return If successful, returns a pointer to a string that is the parent directory of path. Otherwise, returns NULL.
 */
char  *PLATFORM_dirname (char *path);


// =========================================
/**
 * \ingroup FILES_WORKINGDIR
 *
 * @brief Change working directory.
 *
 * @param path		path of the new working directory.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_chdir (const char* arg1);
/**
 * \ingroup FILES_WORKINGDIR
 *
 * @brief Get current working directory.
 *
 * @return If successful, returns a pointer to a string that is the current working directory. Otherwise, returns NULL.
 */
char *PLATFORM_get_current_dir_name (void);

// =========================================
/**
 * \ingroup FILES_SYMLINKS
 *
 * @brief Create a symbolic link to a file.
 *
 * @param filePath			string contained in the symbolic link
 * @param symlinkPathName	name of the symbolic link created
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_symlink (const char* arg1, const char *arg2);
/**
 * \ingroup FILES_SYMLINKS
 *
 * @brief Remove a symbolic link to a file.
 *
 * This does not affect any file or directory named by the contents of the symbolic link
 *
 * @param symlinkPathName The name of removed symlink.
 *
 * @return 0 is returned if success. Otherwise, returns -1.
 */
JITNINT PLATFORM_unlink (const char* arg1);
/**
 * \ingroup FILES_SYMLINKS
 *
 * @brief Read the contents of a symbolic link.
 *
 * @param linkPath 	Symbolic link path.
 * @param buf		pointer to a string where sybolic link content is stored.
 * @param bufSize	number of bytes to read.
 *
 * @return If success, returns count of bytes placed in the buffer. Otherwise, returns -1.
 */
JITNINT PLATFORM_readlink (const char* linkPath, char* buf, size_t bufSize);
/**
 * \ingroup FILES_SYMLINKS
 *
 * @brief Test if mode is symbolic link.
 *
 * @param mode 	Symbolic link path..
 *
 * @return Returns JITTRUE if mode parameter is symbolic link
 */
bool PLATFORM_S_ISLNK (unsigned int mode);

/**@}*/	// end of files grouping


/**
 * @name System & Platform informations.
 * Functions needed to obtain information about current platform, system, user, etc...
 *
 */
/**@{*/

//========System & Platform info


/**
 * \ingroup SYSINFO
 *
 *  @brief get some info about current platform.
 *
 *  \n NOTE: In windows is not possible to retrieve some informations. \n All infos are stored in utsname structure.
 * Inside utsname you can find current fields:
 * 	- char[] sysname:	Name of this implementation of the operating system.
 *  - char[] nodename:	Name of this node within the communications network to which this node is attached, if any.
 *  - char[] release:	Current release level of this implementation.
 *  - char[] version:	Current version level of this release.
 *  - char[] machine:	Name of the hardware type on which the system is running
 * see uname() linux function for more infos.
 *
 *
 * @param  infos pointer to an area where obtained information are stored.
 *
 * @return 0 is returned if success. Otherwise, returns
 */
JITNINT PLATFORM_getPlatformInfo (struct utsname *infos);

/**
 * \ingroup SYSINFO
 *
 *  @brief Returns the name of the system.
 *
 * @param  buff point to a buffer that will hold the host name
 * @param  size the size of the buffer in bytes.
 *
 * @return non negative JITINT if successful. Returns 0 if there is no information available, -1 if an error occurred.
 */
JITNINT PLATFORM_gethostname (char *arg1, size_t arg2);
/**
 * \ingroup SYSINFO
 *
 *  @brief  Returns current code page (character encoding) name stored in system's locale.
 *
 * @return return a pointer to the corresponding string in system's locale
 */
JITINT8* PLATFORM_getCurrentCodePageName (void);

/**
 * \ingroup SYSINFO
 *
 *  @brief Returns the login name associated with the process.
 *
 * @return a pointer to memory area where the corresponding string is stored. Returns NULL if an error occurred.
 */
JITINT8* PLATFORM_getUserName (void);
/**
 * \ingroup SYSINFO
 *
 *  @brief Returns the directory of the user of the process.
 *
 * @return a pointer to memory area where the corresponding string is stored. Returns NULL if an error occurred.
 */
JITINT8* PLATFORM_getUserDir (void);
/**
 * \ingroup SYSINFO
 *
 *  @brief Returns number (count) of processors in current platform.
 *
 * @return the number of processors.
 */
long PLATFORM_getProcessorsNumber(void);
/**
 * \ingroup SYSINFO
 *
 *  @brief Returns the current time inside timespec struct.
 *
 * NOTE for WINDOWS: This function is implemented only for CLOCK_MONOTONIC clock
 *
 * @param clk_id	type of the clock. (see posix clockid_t type in time.h)
 * @param tp		pointer to an area where obtained information are stored.
 *
 * @return 0 is returned if success. Otherwise, returns -1
 */
int PLATFORM_clock_gettime (clockid_t clk_id, struct timespec *tp);

/**@}*/	// end of sysinfo grouping


/** @name Miscellaneous
 *  All API functions without any other categorization.
 */
/**@{*/
//=============== MISC Miscellaneous

/**
 * \ingroup MISC
 *
 *  @brief Delays the execution of the program for at least the specified time.
 *
 * See nanosleep().\n
 * timespec structure is specified as:
 *   - time_t tv_sec        seconds
 *   - long   tv_nsec       nanoseconds
 * (see posix time.h)
 *
 * @param rqtp	intervals of time with nanosecond precision.
 * @param rmtp	pointer to an area where the remaining time is stored in case of unexpected wakeup. (Not implemented in Windows)
 *
 * @return 0 is returned if success. Otherwise, returns -1
 */
int PLATFORM_nanosleep (const struct timespec *rqtp, struct timespec *rmtp);

/**
 * \ingroup MISC
 *
 * @brief Parses the command line arguments.
 *
 *
 *  Its arguments argc and argv are  the  argument  count  and array  as passed to the main() function on program invocation.
 *  \n An element of argv that starts with `-' (and is not exactly "-" or "--") is an option element.
 *  \n The characters of this element (aside from the initial  `-')  are  option characters.
 *  \n  If getopt() is called repeatedly, it returns successively each of the option characters from  each  of the option elements.
 *  \n
 *   See linux getopt_long() function for more details.
 *
 *
 * @return 0 function returns the option character if the
 *     option was found successfully, `:' if there was a  missing
 *     parameter  for  one  of  the  options,  `?' for an unknown
 *      option character, or EOF for the end of the option list.
 *
 */
JITNINT PLATFORM_getOptArg (int argc, char * const argv[], const char *optstring, const struct option *longopts, int *longindex);
/**
 * \ingroup MISC
 *
 *  @brief Get next token from string.
 *
 * This function gets the next token from string *stringp, where tokens are strings separated by characters from delim.
 *
 * @param stringp Points to a character string from which to extract tokens.
 * @param delim Points to a null-terminated string of delimiter characters.
 *
 * @return A pointer to the next token from stringp, as delimited by delim. When there are no more tokens, strsep() returns NULL.
 */
char* PLATFORM_strsep (char **stringp, const char *delim);

/**
 * \ingroup MISC
 *
 *  @brief Get next token from string.
 *
 * This function gets the next token from string *stringp, where tokens are strings separated by characters from delim.\n
 * (see scandir()).
 *
 * @param dir 		Is the name of the directory that scandir() reads.
 * @param namelist	Points to an array of structure pointers.
 * @param select 	Points to a routine that is called with a pointer to a directory entry. If the directory entry is to be included in the array, select returns a non-zero value.
 * @param cmp 		Points to a routine that is passed to qsort() to sort the completed array. If this pointer is NULL, the array is not sorted
 *
 * @return If successful, returns the number of entries in the array and, through the namelist parameter, returns a pointer to the array. If the directory cannot be opened for reading or if cannot allocate enough memory to hold all the data structures, scandir() returns -1.
 */
int PLATFORM_scandir(const char *dir, struct dirent ***namelist, int (*select)(const struct dirent *), int (*cmp)(const void *, const void *) );

/**
 * \ingroup MISC
 *
 *  @brief Allocates memory.
 *
 * Allocates size bytes and places the address of the allocated memory in *memptr.\n
 * The address of the allocated memory will be a multiple of alignment, which must be a power of two and a multiple of sizeof(void *).\n
 * If size is 0, then posix_memalign() returns either NULL, or a unique pointer value that can later be successfully passed to free(3).).
 *
 * @param memptr
 * @param size
 *
 * @return returns 0 on success, or one of the error values ( returns -1 if The alignment argument was not a power of two, or was not a multiple of  sizeof(void *); returns ENOMEM if there was insufficient memory to fulfill the allocation request).
 */

int PLATFORM_posix_memalign (void **memptr, size_t alignment, size_t size);

/**
 * \ingroup MISC
 *
 *
 *  @brief Get current process ID.
 *
 * PLATFORM_getThisProcessId
 *
 * @return the Process ID of the caller.
 */
long int PLATFORM_getThisThreadId (void);
/**@}*/	// end of Miscellaneous functions grouping


/** @name Dynamic Links
 *  These functions gain access to external binaries with a runtime (dynamic) linking.
 */
/**@{*/

//====== DYNAMIC LINKS for Shared Libraries (or DLLs)
/**
 * \ingroup DYNLINKS
 *
 *  @brief Gain access to an executable object file.
 *
 * This function makes an executable object file specified by file available to the calling program.
 * (see dlopen())
 *
 * @param file Specifies the object file to load. If file is not an absolute path, an implementation-defined algorithm is used to locate the file. If file is NULL, dlopen() provides a handle to the calling program itself.
 * @param mode Specifies the mode in which symbols should be bound. This may be either RTLD_LAZY or RTLD_NOW. These values are interpreted identically; symbols are always bound at load time. Both are recognized for maximum source compatibility, since there is no operational difference.
 *
 * @return If success, returns a pointer to dynamic link oject. Returns NULL in case of error (call PLATFORM_dlerror() for more detailed diagnostic informations)
 */
void  *PLATFORM_dlopen (const char *arg1, JITNINT arg2);

/**
 * \ingroup DYNLINKS
 *
 *  @brief Obtain the object of the symbolic link.
 * This function allows a process to obtain the address of a symbol defined within an object. \n
 * (see dlsym())
 *
 * @param file Specifies the object file to load. If file is not an absolute path, an implementation-defined algorithm is used to locate the file. If file is NULL, dlopen() provides a handle to the calling program itself.
 * @param mode Specifies the mode in which symbols should be bound. This may be either RTLD_LAZY or RTLD_NOW. These values are interpreted identically; symbols are always bound at load time. Both are recognized for maximum source compatibility, since there is no operational difference.
 *
 * @return Returns the address binding of the symbol as it occurs in the shared object identified by handle. It returns a NULL pointer if the symbol cannot be found. More detailed diagnostic information is available through PLATFORM_dlerror()
 */
void  *PLATFORM_dlsym (void *arg1, const char *arg2);
/**
 * \ingroup DYNLINKS
 *
 *  @brief Close shared object.
 *
 * This function is used to inform the system that the object referenced by a shared object handle is no longer needed by the application.\n
 * (see dlclose())
 *
 * @param handle handle Points to a previously loaded shared object.
 *
 *
 * @return 0 is returned if success. Otherwise, returns non-zero value.
 */
JITNINT PLATFORM_dlclose (void *arg1);
/**
 * \ingroup DYNLINKS
 *
 *  @brief Return last dynamic linking error.
 *
 * This function returns a null-terminated character string (with no trailing newline) that describes the last error that occurred during dynamic linking processing.
 * (see dlerror())
 *
 *
 * @return null-terminated character string describing the last error that occurred.
 */
JITINT8  *PLATFORM_dlerror (void);
/**@}*/	// end of dynamic links grouping


/** @name Regular Expressions
 *  These functions are used to define and extract regular expression match.
 */
/**@{*/
//=============REGEX
/**
 * \ingroup REGEX
 *
 *  @brief Compare string to regular expression.
 *
 * function compares the null-terminated string specified by string with the compiled regular expression 'preg' initialized by a previous call to PLATFORM_regcomp().\n
 * (see regexec() for more details).
 *
 * @param preg		Points to the compiled regular expression that a previous call to regcomp() initialized.
 * @param string	Points to a null-terminated string.
 * @param nmatch	Is the number of matches allowed.
 * @param pmatch	When nmatch is non-zero, points to an array with at least nmatch elements.
 * @param eflags	The bitwise inclusive OR of zero or more of the flags defined in the header <regex.h>.
 *
 * @return If successful, the function returns 0 to indicate that string matched pattern. On failure, it returns REG_NOMATCH (defined in <regex.h>) to indicate that there was no match.
 */
JITNINT PLATFORM_regexec (const regex_t *arg1, const char *arg2, size_t arg3, regmatch_t* arg4, JITNINT arg5);
/**
 * \ingroup REGEX
 *
 *  @brief Compile regular expression.
 *
 * function compiles the regular expression contained in the string pointed to by the pattern argument and place the results in the structure pointed to by preg.
 * The cflags argument is the bitwise inclusive OR of zero or more of the following flags, which are defined in the header <regex.h>:\n
 * - REG_EXTENDED: Use extended regular expressions.
 * - REG_ICASE:    Ignore case in match.
 * - REG_NOSUB:	   Report only success or failure in regexec().
 * - REG_NEWLINE:  Change the handling of newline characters, as described in the text.
 *  (see regexec() for more details).
 *
 * @param preg		Points to the structure where the results are placed.
 * @param pattern	Points to the string that the regular expression.
 * @param cflags	The bitwise inclusive OR of zero or more of the flags defined in the header <regex.h>.
 *
 * @return If successful completion, the function returns 0. Otherwise, it returns a non-zero value.
 */
JITNINT PLATFORM_regcomp (regex_t* arg1, const char *arg2, JITNINT arg3);
/**
 * \ingroup REGEX
 *
 *  @brief Free memory used by regular expression.
 *
 *
 * @param preg A pointer to a compiled regular expression.
 *
 * @return none.
 */
void PLATFORM_regfree (regex_t* arg1);

/**
 * \ingroup REGEX
 *
 *  @brief Compiles the regular expression using a pattern.
 *
 * If the function successfully compiles the regular expression, sets the pattern buffer's fields as follows:
 * - buffer:   		to the compiled pattern.
 * - used:			to the number of bytes the compiled pattern in buffer occupies.
 * - syntax:		to the current value of re_syntax_options.
 * - re_nsub:		to the number of subexpressions in regex.
 * - fastmap_accurate:	to zero on the theory that the pattern you're compiling is different than the one previously compiled into buffer; in that case (since you can't make a fastmap without a compiled pattern), fastmap would either contain an incompatible fastmap, or nothing at all.
 * \n
 * (see re_compile_pattern())
 *
 * @param regex 	 		the regular expression's address.
 * @param regex_size 		its length.
 * @param pattern_buffer	the pattern buffer's address.
 *
 * @return If successful completion, the function returns 0. Otherwise, it returns a non-zero value.
 */
const char* PLATFORM_re_compile_pattern (const char *arg1, size_t arg2, struct re_pattern_buffer *arg3);
/**
 * \ingroup REGEX
 *
 *  @brief Specify the precise syntax of regexps for compilation.
 *
 *  This provides for compatibility for various utilities which historically have different, incompatible syntaxes. \n
 *  The argument SYNTAX is a bit mask comprised of the various bits defined in regex.h.  We return the old syntax.
 *
 * (see re_set_syntax())
 *
 * @param syntax 	 		bit mask comprised of the various bits defined in regex.h
 *
 * @return the old syntax.
 */
reg_syntax_t  PLATFORM_re_set_syntax (reg_syntax_t arg1);

/**@}*/	// end of regex grouping


/** @name Threads
 *  Functions used to gain platform-indipendent multiple flows of control.
 */
/**@{*/

//========= THREADS
/**
 * \ingroup THREADS_BASE
 *
 *  @brief Create new thread.
 *
 * function is used to create a new thread, with attributes specified by 'attr', within a process. (see PLATFORM_initThreadAttr)
 * At creation, the thread executes 'thread_function', with arg as its sole argument. The calling function must ensure that arg remains valid for the new thread throughout its lifetime.
 *
 * @param thread Is the location where the ID of the newly created thread should be stored, or NULL if the thread ID is not required.
 * @param attr 	Is the thread attribute object specifying the attributes for the thread that is being created. If attr is NULL, the thread is created with default attributes.
 * @param thread_function Is the main function for the thread; the thread begins executing user code at this address.
 * @param arg   Is the argument passed to start.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_createThread(pthread_t *arg1, const pthread_attr_t *arg2,  void *(*thread_function)(void*), void *arg4);
/**
 * \ingroup THREADS_BASE
 *
 *  @brief Wait for thread termination.
 *
 * This function suspends execution of the calling thread until the target thread terminates, unless the target thread has already terminated.
 *
 * @param thread Is the thread to wait for.
 * @param status  Is the location where the exit status of the joined thread is stored. This can be set to NULL if the exit status is not required.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_joinThread (pthread_t arg1, void **arg2);

/**
 * \ingroup THREADS_BASE
 *
 *  @brief Get the ID of the calling thread.
 *
 * @return returns the ID of the calling thread.
 */
pthread_t PLATFORM_getSelfThreadID (void);
/**
 * \ingroup THREADS_BASE
 *
 *  @brief Dynamic package initialization.
 *
 * The first call to this function by any thread in a process, with a given 'once_control', calls the 'thread_function' with no arguments. \n
 * Subsequent calls with the same 'once_control' do not call the thread_function.\n
 * Default once_control is PTHREAD_ONCE_INIT constant.
 *
 * @param once_control Determines whether the associated initialization routine has been called.
 * @param thread_function Specifies an initialize routine to be run.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_pthread_once(pthread_once_t *arg1, void (*thread_function)(void) );
/**
 * \ingroup THREADS_BASE
 *
 *  @brief Compare thread IDs.
 *
 * @param thread1	First thread handler
 * @param thread2	Second thread handler.
 *
 * @return If the the handlers refers to the same thread, function returns non-zero value (True). Otherwise, 0 (False) is returned.
 */
JITNINT PLATFORM_equalThread (pthread_t arg1, pthread_t arg2);
/**
 * \ingroup THREADS_BASE
 *
 *  @brief Compute hash function from thread handler.
 *
 * Needed for hash tables.
 *
 * @param element	thread handler
 *
 * @return return the unsigned int referred to the element.
 */
unsigned int PLATFORM_threadHashFunc (void *element);

//=== Thread scheduling control
/**
 * \ingroup THREADS_SCHED
 *
 *  @brief Yeld schedule.
 *
 *  The function forces the running thread to relinquish the processor to allow another thread to run.
 *
 * @return always returns 0.
 */
JITNINT PLATFORM_sched_yield (void);
/**
 * \ingroup THREADS_SCHED
 *
 *  @brief Get priority max limit.
 *
 *  The functions returns the appropriate maximum value, for the scheduling policy specified.
 *
 *  @param policy one of the scheduling policy values defined in <sched.h>
 *
 * @return If successful, returns the appropriate maximum value. Otherwise, returns -1.
 */
JITNINT PLATFORM_sched_get_priority_max (JITNINT arg1);
/**
 * \ingroup THREADS_SCHED
 *
 *   @brief Get priority min limit.
 *
 *  The functions returns the appropriate minimum value, for the scheduling policy specified.
 *
 *  @param policy one of the scheduling policy values defined in <sched.h>
 *
 * @return If successful, returns the appropriate minimum value. Otherwise, returns -1.
 */
JITNINT PLATFORM_sched_get_priority_min (JITNINT arg1);

//=== Thread Attributes

/**
 * \ingroup THREADS_ATTR
 *
 *   @brief initialize thread attribute object.
 *
 *  @param attr Is the thread attribute object to be initialized.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_initThreadAttr (pthread_attr_t *arg1);
/**
 * \ingroup THREADS_ATTR
 *
 *   @brief set scheduling scope attribute.
 *
 *  \n NOTE: this function takes effect only under Linux, by the differences of other platforms thread implementation.
 *  \n
 *  Sets the scheduling scope attribute in the specified attribute object.The thread's scheduling scope attribute determines whether thread-scheduling decisions apply to threads in a given process, or system-wide to all threads.
 *  \n valid settings for scope include:
 *  - PTHREAD_SCOPE_PROCESS:	Threads are scheduled with respect to other threads in the current process.
 *  - PTHREAD_SCOPE_SYSTEM :    Threads are scheduled with respect to all threads in the system.
 *  \n The specified scheduling scope is only used if the scheduling parameter inheritance attribute is PTHREAD_EXPLICIT_SCHED.
 *
 *  @param attr Is the thread attribute object to be set.
 *  @param scope Is the thread scheduling scope attribute value.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setThreadAttr_scope (pthread_attr_t *arg1, JITNINT arg2);

/**
 * \ingroup THREADS_ATTR
 *
 *   @brief Set thread stack size attribute.
 *
 *  \n NOTE: this function takes effect only under Linux, by the differences of other platforms thread implementation.
 *
 *  @param attr			Is the thread attribute object to be set .
 *  @param stacksize	Is the address of location used to store the current stack size attribute
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setThreadAttr_stackSize (pthread_attr_t *arg1, size_t arg2);
/**
 * \ingroup THREADS_ATTR
 *
 *   @brief Set scheduling parameter inheritance state attribute.
 *
 *  \n NOTE: this function takes effect only under Linux, by the differences of other platforms thread implementation.\n
 *  The thread's scheduling parameter inheritance state determines whether scheduling parameters are explicitly specified in this attribute object, or if scheduling attributes should be inherited from the creating thread. \n
 *  Valid settings for 'inheritsched' include:
 *  - PTHREAD_EXPLICIT_SCHED: Scheduling parameters for the newly created thread are specified in the thread attribute object.
 *  - PTHREAD_INHERIT_SCHED:  Scheduling parameters for the newly created thread are the same as those of the creating thread.
 *
 *  @param attr Is the thread attribute object to be set.
 *  @param inheritsched Is the thread scheduling parameter inheritance state attribute value, one of the values specified before.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setThreadAttr_inheritsched (pthread_attr_t *arg1, JITNINT arg2);
/**
 * \ingroup THREADS_ATTR
 *
 *   @brief Set scheduling parameter attribute.
 *
 *  \n NOTE: this function takes effect only under Linux, by the differences of other platforms thread implementation.\n
 *  Sets the scheduling parameter attribute in the specified attribute object, determined by the scheduling policy specified in the attribute object.\n
 *  The only required member of the param structure for the SCHED_FIFO, SCHED_OTHER, or SCHED_RR policies is sched_priority.\n
 *  This function can be used to specify the starting priority of a newly created thread.
 *
 *  @param attr		Is the thread attribute object to be set.
 *  @param param 	Is the thread scheduling parameter attribute value.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setThreadAttr_schedparam (pthread_attr_t *arg1, const struct sched_param *arg2);
/**
 * \ingroup THREADS_ATTR
 *
 *   @brief Set scheduling policy attribute
 *
 *  \n NOTE: this function takes effect only under Linux, by the differences of other platforms thread implementation.\n
 *  The thread's scheduling policy attribute determines how time slices are scheduled for the thread.\n
 *   Valid settings for policy include:
 *   - SCHED_FIFO:	Threads are scheduled in a first-in-first-out order within each priority.
 *   - SCHED_OTHER:	Scheduling behavior is determined by the operating system.
 *   - SCHED_RR:	Threads are scheduled in a round-robin fashion within each priority.
 *   The specified scheduling policy will only be used if the scheduling parameter inheritance attribute is PTHREAD_EXPLICIT_SCHED.
 *
 *  @param attr		Is the thread attribute object to be set.
 *  @param policy  	Is the thread scheduling policy attribute value, one of the values specified before.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setThreadAttr_schedpolicy (pthread_attr_t *arg1, JITNINT arg2);

//=== threads - Mutex
/**
 * \ingroup THREADS_MUTEX
 *
 *   @brief Initialize a mutex.
 *
 *  @param mutex	Is the mutex to initialize.
 *  @param attr  	Specifies the attributes to use to initialize the mutex, or NULL if default attributes should be used.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_initMutex (pthread_mutex_t *arg1, const pthread_mutexattr_t *arg2);
/**
 * \ingroup THREADS_MUTEX
 *
 *   @brief Initialize mutex attribute object.
 *
 *  @param attr  	Is the mutex attribute object to be initialized.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_initMutexAttr (pthread_mutexattr_t *arg1);

/**
 * \ingroup THREADS_MUTEX
 *
 *   @brief Set process-shared state attribute of mutex attribute object.
 *
 *  Sets the process-shared state attribute in the specified mutex attribute object.\n
 *  The mutex's process-shared state determines whether the mutex can be used to synchronize threads within the current process or threads within all processes on the system.\n
 *  Valid settings for pshared include:
 *  - PTHREAD_PROCESS_PRIVATE:	Creates a mutex that can only be used to synchronize threads within the current process.
 *  - PTHREAD_PROCESS_SHARED:   Creates a mutex that can be used to synchronize threads within all processes on the system.

 *
 * @param attr  	Is the mutex attribute object.
 * @param pshared	Is the mutex process-shared state attribute value, one of the values specified before.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setMutexAttr_pshared (pthread_mutexattr_t *arg1, JITNINT arg2);

/**
 * \ingroup THREADS_MUTEX
 *
 *   @brief Set mutex type attribute of mutex attribute object.
 *
 *  Valid settings for type include:\n
 *  - PTHREAD_MUTEX_NORMAL		This type of mutex does not detect deadlock. An attempt to relock this mutex without first unlocking it deadlocks. Attempting to unlock a mutex locked by a different thread results in undefined behavior. Attempting to unlock an unlocked mutex results in undefined behavior.
 *  - PTHREAD_MUTEX_ERRORCHECK	This type of mutex provides error checking. An attempt to relock this mutex without first unlocking it returns with an error. An attempt to unlock a mutex that another thread has locked returns with an error. An attempt to unlock an unlocked mutex returns with an error.
 *  - PTHREAD_MUTEX_RECURSIVE	A thread attempting to relock this mutex without first unlocking it succeeds in locking the mutex. The relocking deadlock that can occur with mutexes of type PTHREAD_MUTEX_NORMAL cannot occur with this type of mutex. Multiple locks of this mutex require the same number of unlocks to release the mutex before another thread can acquire the mutex. An attempt to unlock a mutex that another thread has locked returns with an error. An attempt to unlock an unlocked mutex returns with an error.
 *  - PTHREAD_MUTEX_DEFAULT		Attempting to recursively lock a mutex of this type results in undefined behavior. Attempting to unlock a mutex of this type that was not locked by the calling thread results in undefined behavior. Attempting to unlock a mutex of this type that is not locked results in undefined behavior.
 *
 * @param attr  Is the mutex attribute object.
 * @param type 	The mutex type attribute value; one of the values specified above.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_setMutexAttr_type (pthread_mutexattr_t *arg1, JITNINT arg2);

/**
 * \ingroup THREADS_MUTEX
 *
 *  @brief Destroys a previously initialized mutex attribute object.
 *
 * The attribute object may not be reused until it is reinitialized.
 *
 * @param mutexAttr      Is the mutex attribute object to destroy.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_destroyMutexAttr (pthread_mutexattr_t *arg1);

/**
 * \ingroup THREADS_MUTEX
 *
 *  @brief Destroy a mutex.
 *
 * The mutex must not be used after it has been destroyed.
 *
 * @param mutex      Is the mutex to destroy.
 *
 * @return On success, returns 0. On error, non-zero value is returned.
 */
JITNINT PLATFORM_destroyMutex (pthread_mutex_t *arg1);

/**
* \ingroup THREADS_MUTEX
*
*  @brief Lock a mutex.
*
* If the mutex is already locked, the calling thread blocks until the mutex becomes available.\n
* This operation returns with the mutex in the locked state with the calling thread as its owner.
*
* @param mutex          Is the mutex to lock.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_lockMutex (pthread_mutex_t *arg1);
/**
* \ingroup THREADS_MUTEX
*
*  @brief Try to lock a mutex.
*
* If the mutex is already locked, an error is returned. Otherwise, this operation returns with the mutex in the locked state with the calling thread as its owner.
*
* @param mutex          Is the mutex to try to lock.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_trylockMutex (pthread_mutex_t *arg1);

/**
* \ingroup THREADS_MUTEX
*
*  @brief Unlock a mutex.
*
* If there are threads blocked on the mutex object when PLATFORM_unlockMutex() is called, resulting in the mutex becoming available, the scheduling policy is used to determine which thread acquires the mutex.
*
* @param mutex          Is the mutex to unlock.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_unlockMutex (pthread_mutex_t *arg1);


//=== threads - Condition Variables

/**
* \ingroup THREADS_CONDVAR
*
*  @brief Initialize condition variable.
*
* If attr is NULL, the condition variable is initialized with default attributes.
*
* @param condVar       Is the condition variable to initialize.
* @param attr 		Specifies the attributes used for initializing the condition variable, or NULL if default attributes should be used.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_initCondVar (pthread_cond_t *arg1, const pthread_condattr_t *arg2);

/**
* \ingroup THREADS_CONDVAR
*
*  @brief Initialize condition variable attributes.
*
* Initialize condition variable attributes object with the default settings for each attribute.\n
*
* @param condVar  Is the condition variable attribute object to be initialized.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_initCondVarAttr (pthread_condattr_t *arg1);

/**
* \ingroup THREADS_CONDVAR
*
*  @brief Set the process-shared condition variable attributes.
*
* The condition variable's process-shared state determines whether the condition variable can be used to synchronize threads within the current process or threads within all processes on the system.\n
* Valid settings for pshared include:
* - PTHREAD_PROCESS_PRIVATE:	Creates a condition variable that can only be used to synchronize threads within the current process.
* - PTHREAD_PROCESS_SHARED:		Creates a condition variable that can be used to synchronize threads within all processes on the system
*
*
* @param attr   Is the condition variable attribute object to be initialized.
* @param pshared Is the condition variable process-shared state attribute value; one of the values specified before.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_setCondVarAttr_pshared (pthread_condattr_t *arg1, JITNINT arg2);
/**
* \ingroup THREADS_CONDVAR
*
*  @brief Wait on a condition.
*
* The function blocks on the specified condition variable, which atomically releases the specified mutex and causes the calling thread to block on the condition variable The blocked thread may be awakened by a call to PLATFORM_signalCondVar() or PLATFORM_broadcastCondVar().
* \n This function atomically releases the mutex, causing the calling thread to block on the condition variable Upon successful completion, the mutex is locked and owned by the calling thread.
* \n When using condition variables, there should always be a boolean predicate involving shared variables related to each condition wait. This predicate should become true only when the thread should proceed. Because the return from PLATFORM_waitCondVar() does not indicate anything about the value of this predicate, the predicate should be reevaluated on return. Unwanted wakeups from pthread_cond_wait() may occur (since another thread could have obtained the mutex, changed the state and released the mutex, prior to this thread obtaining the mutex); the reevaluation of the predicate ensures consistency.
* \n The function is a cancellation point. If a cancellation request is acted on while in a condition wait when the cancellation type of a thread is set to deferred, the mutex is reacquired before calling the first cancellation cleanup handler. In other words, the thread is unblocked, allowed to execute up to the point of returning from the call pthread_cond_wait(), but instead of returning to the caller, it performs the thread cancellation.
*
* @param condVar   Is the condition variable to wait on.
* @param mutex  Is the mutex associated with the condition variable.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_waitCondVar (pthread_cond_t *arg1, pthread_mutex_t *arg2);

/**
* \ingroup THREADS_CONDVAR
*
*  @brief Signal a condition.
*
* \n This function unblocks a single thread blocked on the specified condition variable.
* \n The function has no effect if no threads are blocked on the condition variable.
* \n This function may be called by a thread whether or not it owns the mutex which threads calling PLATFORM_waitCondVar() have associated with the condition variable during their waits.
* However, if predictable scheduling behavior is required, then that mutex should be locked by the thread calling PLATFORM_signalCondVar().
*
* @param condVar   Is the condition variable to signal.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_signalCondVar (pthread_cond_t *arg1);
/**
* \ingroup THREADS_CONDVAR
*
*  @brief Broadcast a condition.
*
* \n This function unblocks all threads blocked on the specified condition variable.
* \n The function has no effect if no threads are blocked on the condition variable. PLATFORM_broadcastCondVar() may be called by a thread whether or not it owns the mutex which threads calling PLATFORM_waitCondVar() have associated with the condition variable during their waits.
* \n However, if predictable scheduling behavior is required, then that mutex should be locked by the thread calling PLATFORM_broadcastCondVar().
*
* @param condVar   Is the condition variable to broadcast.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_broadcastCondVar (pthread_cond_t *arg1);

/**
* \ingroup THREADS_CONDVAR
*
*  @brief Destroy condition variable.
*
* The condition variable must not be used after it has been destroyed.
*
* @param condVar   Is the condition variable to destroy.
*
* @return On success, returns 0. On error, non-zero value is returned.
*/
JITNINT PLATFORM_destroyCondVar (pthread_cond_t *arg1);

/**@}*/	// end of thread functions grouping





/* UNCATEGORIZED and recently added API: */

void  *PLATFORM_mmap(void * arg1, size_t arg2, int arg3, int arg4, int arg5, off_t arg6);

int  PLATFORM_munmap(void *arg1, size_t arg2);

long int PLATFORM_getSystemPageSize();

int PLATFORM_pthread_setaffinity_np (pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuAffinityMask);

void PLATFORM_CPU_ZERO(cpu_set_t *set);
void PLATFORM_CPU_SET(int cpu, cpu_set_t *set);
int PLATFORM_CPU_ISSET(int cpu, cpu_set_t *set);

int PLATFORM_mkfifo (const char *path, mode_t mode);

/* http://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html */
ssize_t PLATFORM_getdelim(char ** lineptr, size_t *n, int delimiter, FILE *stream);

/* http://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html */
ssize_t PLATFORM_getline(char **lineptr, size_t *n, FILE *stream);

pid_t PLATFORM_fork();
pid_t PLATFORM_waitpid(pid_t pid, int *stat_loc, int options);
/* http://publib.boulder.ibm.com/infocenter/tpfhelp/current/index.jsp?topic=%2Fcom.ibm.ztpf-ztpfdf.doc_put.cur%2Fgtpc2%2Fcpp_wifexited.html */
int PLATFORM_WIFEXITED(int status);
/* http://publib.boulder.ibm.com/infocenter/tpfhelp/current/index.jsp?topic=%2Fcom.ibm.ztpf-ztpfdf.doc_put.cur%2Fgtpc2%2Fcpp_wexitstatus.html */
int PLATFORM_WEXITSTATUS(int status);

int PLATFORM_setenv(const char *envname, const char *envval, int overwrite);
char* PLATFORM_getenv(const char *envVarName);

/*      http://www.kernel.org/doc/man-pages/online/pages/man3/clearenv.3.html*/
int PLATFORM_clearenv(void);

/* http://pubs.opengroup.org/onlinepubs/009604499/functions/setgid.html */
int PLATFORM_setgid(gid_t gid);
int PLATFORM_setuid(uid_t uid);

#ifdef __cplusplus
};
#endif

#endif
