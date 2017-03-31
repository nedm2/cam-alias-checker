/*
 * Copyright (C) 2007 - 2010 Campanoni Simone, Luca Rocchini, Michele Tartara
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <platform_API.h>

/* My headers	*/
#include <xanlib.h>
#include <xan-system.h>
#include <config.h>


/**
 * Unsynchronised functions.
 **/

XanVar * xanVar_new (void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr)) {
    XanVar  *var;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        var		= malloc(sizeof(XanVar));
        var->alloc	= malloc;
    } else {
        var		= allocFunction(sizeof(XanVar));
        var->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        var->free		= free;
    } else {
        var->free		= freeFunction;
    }

    /* Make the variabl	*/
    assert(var != NULL);
    var->data = NULL;
    pthread_mutexattr_t mutex_attr;
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(var->mutex), &mutex_attr);

    /* Return		*/
    return var;
}

void * xanVar_read (XanVar *var) {
    return var->data;
}

void xanVar_write (XanVar *var, void *data) {
    var->data = data;
}

void xanVar_destroyVar (XanVar *var) {

    /* Assertions		*/
    assert(var != NULL);

    var->free(var);
}

void xanVar_destroyVarAndData (XanVar *var) {

    /* Assertions		*/
    assert(var != NULL);

    if (var->data != NULL) {
        var->free(var->data);
    }
    var->free(var);
}

void xanVar_lock (XanVar *var) {

    /* Assertions		*/
    assert(var != NULL);

    PLATFORM_lockMutex(&(var->mutex));
}

void xanVar_unlock (XanVar *var) {

    /* Assertions		*/
    assert(var != NULL);

    PLATFORM_unlockMutex(&(var->mutex));
}


/**
 * Synchronised functions.
 **/

void * xanVar_syncRead (XanVar *var) {
    void    *data;

    /* Assertions		*/
    assert(var != NULL);

    PLATFORM_lockMutex(&(var->mutex));
    data = xanVar_read(var);
    PLATFORM_unlockMutex(&(var->mutex));
    return data;
}

void xanVar_syncWrite (XanVar *var, void *data) {

    /* Assertions		*/
    assert(var != NULL);

    PLATFORM_lockMutex(&(var->mutex));
    xanVar_write(var, data);
    PLATFORM_unlockMutex(&(var->mutex));
}
