/*
 * Copyright (C) 2007 - 2012 Campanoni Simone, Luca Rocchini, Michele Tartara
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

/* Tree				*/
static inline XanNode * xanNodeGetParent (XanNode *node);
static inline XanNode * xanNodeGetNextChildren (XanNode *node, XanNode *child);
static inline XanNode * xanNodeAddNewChildren (XanNode *node, void *childData);
static inline void xanNodeAddChildren (XanNode *node, XanNode *child);
static inline void xanNodeSetParent (XanNode *node, XanNode *parent);
static inline void xanNodeSetData (XanNode *node, void *data);
static inline void * xanNodeGetData (XanNode *node);
static inline XanNode * _xanNodeGetParent (XanNode *node);
static inline XanNode * _xanNodeGetNextChildren (XanNode *node, XanNode *child);
static inline XanNode * _xanNodeAddNewChildren (XanNode *node, void *childData);
static inline void _xanNodeAddChildren (XanNode *node, XanNode *child);
static inline void _xanNodeSetParent (XanNode *node, XanNode *parent);
static inline void _xanNodeSetData (XanNode *node, void *data);
static inline void * _xanNodeGetData (XanNode *node);
static inline XanNode * xanNodeCloneTree (XanNode *node);
static inline XanNode * _xanNodeCloneTree (XanNode *node);
static inline void xanNodeDeleteChildren (XanNode *node, XanNode *child);
static inline void _xanNodeDeleteChildren (XanNode *node, XanNode *child);
static inline XanList * xanNodeGetChildrens (XanNode *node);
static inline XanList * _xanNodeGetChildrens (XanNode *node);
static inline void xanNodeSetCloneFunction (XanNode *node, void * (*cloneFunction)(void *data));
static inline void _xanNodeSetCloneFunction (XanNode *node, void * (*cloneFunction)(void *data));
static inline void xanNodeDestroyTreeAndData (XanNode *node);
static inline void _xanNodeDestroyTreeAndData (XanNode *node);
static inline void xanNodeDestroyTree (XanNode *node);
static inline void _xanNodeDestroyTree (XanNode *node);
static inline void _xanNodeDestroyNode (XanNode *node);
static inline XanList * xanNodeToPreOrderList (XanNode *node);
static inline XanList * _xanNodeToPreOrderList (XanNode *node);
static inline XanList * xanNodeToPostOrderList (XanNode *node);
static inline XanList * _xanNodeToPostOrderList (XanNode *node);
static inline XanList * xanNodeToInOrderList (XanNode *node);
static inline XanList * _xanNodeToInOrderList (XanNode *node);
static inline XanNode * xanNodeFind (XanNode *rootNode, void *data);
static inline XanNode * _xanNodeFind (XanNode *rootNode, void *data);
static inline unsigned int _xanNodeGetDepth (XanNode *rootNode);

/* Internal function		*/
static inline void internalPreOrderListHelpFunction (XanNode *tree, XanList *list);
static inline void internalPostOrderListHelpFunction (XanNode *tree, XanList *list);
static inline void internalInOrderListHelpFunction (XanNode *tree, XanList *list);

XanNode * xanNode_new(void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void * (*cloneFunction)(void *data)) {
    XanNode *newNode;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        newNode		= malloc(sizeof(XanNode));
        newNode->alloc	= malloc;
    } else {
        newNode		= allocFunction(sizeof(XanNode));
        newNode->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        newNode->free		= free;
    } else {
        newNode->free		= freeFunction;
    }

    /* Initialize the pipe			*/
    assert(newNode != NULL);
    newNode->clone = cloneFunction;
    newNode->synchGetParent = xanNodeGetParent;
    newNode->synchGetNextChildren = xanNodeGetNextChildren;
    newNode->synchAddNewChildren = xanNodeAddNewChildren;
    newNode->synchAddChildren = xanNodeAddChildren;
    newNode->synchSetParent = xanNodeSetParent;
    newNode->synchSetData = xanNodeSetData;
    newNode->synchGetData = xanNodeGetData;
    newNode->synchCloneTree = xanNodeCloneTree;
    newNode->synchDeleteChildren = xanNodeDeleteChildren;
    newNode->synchGetChildrens = xanNodeGetChildrens;
    newNode->synchSetCloneFunction = xanNodeSetCloneFunction;
    newNode->synchDestroyTreeAndData = xanNodeDestroyTreeAndData;
    newNode->synchDestroyTree = xanNodeDestroyTree;
    newNode->synchToPreOrderList = xanNodeToPreOrderList;
    newNode->synchToPostOrderList = xanNodeToPostOrderList;
    newNode->synchToInOrderList = xanNodeToInOrderList;
    newNode->synchFind = xanNodeFind;
    newNode->getParent = _xanNodeGetParent;
    newNode->getNextChildren = _xanNodeGetNextChildren;
    newNode->addNewChildren = _xanNodeAddNewChildren;
    newNode->addChildren = _xanNodeAddChildren;
    newNode->setParent = _xanNodeSetParent;
    newNode->setData = _xanNodeSetData;
    newNode->getData = _xanNodeGetData;
    newNode->cloneTree = _xanNodeCloneTree;
    newNode->deleteChildren = _xanNodeDeleteChildren;
    newNode->getChildrens = _xanNodeGetChildrens;
    newNode->setCloneFunction = _xanNodeSetCloneFunction;
    newNode->destroyTreeAndData = _xanNodeDestroyTreeAndData;
    newNode->destroyTree = _xanNodeDestroyTree;
    newNode->destroyNode = _xanNodeDestroyNode;
    newNode->toPreOrderList = _xanNodeToPreOrderList;
    newNode->toPostOrderList = _xanNodeToPostOrderList;
    newNode->toInOrderList = _xanNodeToInOrderList;
    newNode->find = _xanNodeFind;
    newNode->getDepth = _xanNodeGetDepth;
    newNode->childrens = xanList_new(allocFunction, freeFunction, NULL);
    newNode->parent = NULL;
    newNode->data = NULL;
    pthread_mutexattr_t mutex_attr;
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(newNode->mutex), &mutex_attr);
    assert(newNode->childrens != NULL);

    /* Return the node			*/
    return newNode;
}

static inline XanList * xanNodeToPreOrderList (XanNode *node) {
    XanList *list;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    list = _xanNodeToPreOrderList(node);
    assert(list != NULL);
    PLATFORM_unlockMutex(&(node->mutex));

    return list;
}

static inline XanList * xanNodeToPostOrderList (XanNode *node) {
    XanList *list;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    list = _xanNodeToPostOrderList(node);
    assert(list != NULL);
    PLATFORM_unlockMutex(&(node->mutex));

    return list;
}

static inline XanNode * xanNodeFind (XanNode *rootNode, void *data) {
    XanNode *node;

    /* Assertions			*/
    assert(rootNode != NULL);

    PLATFORM_lockMutex(&(rootNode->mutex));
    node = _xanNodeFind(rootNode, data);
    PLATFORM_unlockMutex(&(rootNode->mutex));

    return node;
}

static inline XanList * xanNodeToInOrderList (XanNode *node) {
    XanList *list;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    list = _xanNodeToInOrderList(node);
    assert(list != NULL);
    PLATFORM_unlockMutex(&(node->mutex));

    return list;
}

static inline void xanNodeDestroyTree (XanNode *node) {

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeDestroyTree(node);
    PLATFORM_unlockMutex(&(node->mutex));
}

static inline void xanNodeDestroyTreeAndData (XanNode *node) {

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeDestroyTreeAndData(node);
    PLATFORM_unlockMutex(&(node->mutex));
}

static inline void xanNodeSetCloneFunction (XanNode *node, void * (*cloneFunction)(void *data)) {

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeSetCloneFunction(node, cloneFunction);
    PLATFORM_unlockMutex(&(node->mutex));
}

static inline XanList * xanNodeGetChildrens (XanNode *node) {
    XanList *childrens;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    childrens = _xanNodeGetChildrens(node);
    PLATFORM_unlockMutex(&(node->mutex));

    return childrens;
}

static inline XanNode * xanNodeCloneTree (XanNode *node) {
    XanNode *clone;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    clone = _xanNodeCloneTree(node);
    PLATFORM_unlockMutex(&(node->mutex));

    return clone;
}

static inline void * xanNodeGetData (XanNode *node) {
    void    *data;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    data = _xanNodeGetData(node);
    PLATFORM_unlockMutex(&(node->mutex));

    return data;
}

static inline void xanNodeSetData (XanNode *node, void *data) {

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeSetData(node, data);
    PLATFORM_unlockMutex(&(node->mutex));
}

static inline XanNode * xanNodeGetParent (XanNode *node) {
    XanNode *parent;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    parent = _xanNodeGetParent(node);
    PLATFORM_unlockMutex(&(node->mutex));

    return parent;
}

static inline XanNode * xanNodeGetNextChildren (XanNode *node, XanNode *child) {
    XanNode *nextChild;

    /* Assertions			*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    nextChild = _xanNodeGetNextChildren(node, child);
    PLATFORM_unlockMutex(&(node->mutex));

    return nextChild;
}

static inline void xanNodeDeleteChildren (XanNode *node, XanNode *child) {

    /* Assertions		*/
    assert(node != NULL);
    assert(child != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeDeleteChildren(node, child);
    PLATFORM_unlockMutex(&(node->mutex));
}

static inline void xanNodeAddChildren (XanNode *node, XanNode *child) {

    /* Assertions		*/
    assert(node != NULL);
    assert(child != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeAddChildren(node, child);
    PLATFORM_unlockMutex(&(node->mutex));
}

static inline XanNode * xanNodeAddNewChildren (XanNode *node, void *childData) {
    XanNode *newNode;

    /* Assertions		*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    newNode = _xanNodeAddNewChildren(node, childData);
    PLATFORM_unlockMutex(&(node->mutex));

    return newNode;
}

static inline void xanNodeSetParent (XanNode *node, XanNode *parent) {

    /* Assertions		*/
    assert(node != NULL);

    PLATFORM_lockMutex(&(node->mutex));
    _xanNodeSetParent(node, parent);
    PLATFORM_unlockMutex(&(node->mutex));

    return;
}

static inline void _xanNodeSetData (XanNode *node, void *data) {

    /* Assertions		*/
    assert(node != NULL);

    node->data = data;
}

static inline void * _xanNodeGetData (XanNode *node) {

    /* Assertions		*/
    assert(node != NULL);

    return node->data;
}

static inline void _xanNodeDestroyTreeAndData (XanNode *node) {
    XanNode *child;
    XanNode *childNext;

    /* Assertions		*/
    assert(node != NULL);

    child = _xanNodeGetNextChildren(node, NULL);
    while (child != NULL) {
        childNext = _xanNodeGetNextChildren(node, child);
        _xanNodeDestroyTreeAndData(child);
        child = childNext;
    }

    if (node->data != NULL) {
        node->free(node->data);
    }
    xanList_destroyList(node->childrens);
    node->free(node);
}

static inline void _xanNodeDestroyNode (XanNode *node) {

    /* Assertions		*/
    assert(node != NULL);
    assert(node->childrens != NULL);

    xanList_destroyList(node->childrens);
    node->free(node);
}


static inline void _xanNodeDestroyTree (XanNode *node) {
    XanNode *child;
    XanNode *childNext;

    /* Assertions		*/
    assert(node != NULL);
    assert(node->childrens != NULL);

    child = _xanNodeGetNextChildren(node, NULL);
    while (child != NULL) {
        childNext = _xanNodeGetNextChildren(node, child);
        _xanNodeDestroyTree(child);
        child = childNext;
    }
    xanList_destroyList(node->childrens);
    node->free(node);
}

static inline XanList * _xanNodeToPreOrderList (XanNode *node) {
    XanList *list;

    /* Assertions		*/
    assert(node != NULL);

    /* Make the list	*/
    list = xanList_new(node->alloc, node->free, node->clone);
    assert(list != NULL);
    internalPreOrderListHelpFunction(node, list);
    assert(xanList_length(list) > 0);

    return list;
}

static inline XanList * _xanNodeToPostOrderList (XanNode *node) {
    XanList *list;

    /* Assertions		*/
    assert(node != NULL);

    /* Make the list	*/
    list = xanList_new(node->alloc, node->free, node->clone);
    assert(list != NULL);
    internalPostOrderListHelpFunction(node, list);
    assert(xanList_length(list) > 0);

    return list;
}

static inline XanNode * _xanNodeFind (XanNode *rootNode, void *data) {
    XanNode *child;
    XanNode *node;

    /* Assertions		*/
    assert(rootNode != NULL);

    if (rootNode->data == data) {
        return rootNode;
    }
    child = rootNode->getNextChildren(rootNode, NULL);
    while (child != NULL) {
        node = _xanNodeFind(child, data);
        if (node != NULL) {
            return node;
        }
        child = rootNode->getNextChildren(rootNode, child);
    }

    return NULL;
}

static inline XanList * _xanNodeToInOrderList (XanNode *node) {
    XanList *list;

    /* Assertions		*/
    assert(node != NULL);

    /* Make the list	*/
    list = xanList_new(node->alloc, node->free, node->clone);
    assert(list != NULL);
    internalInOrderListHelpFunction(node, list);
    assert(xanList_length(list) > 0);

    return list;
}

static inline void _xanNodeSetCloneFunction (XanNode *node, void * (*cloneFunction)(void *data)) {

    /* Assertions		*/
    assert(node != NULL);

    node->clone = cloneFunction;
    xanList_setCloneFunction(node->childrens, cloneFunction);
}

static inline XanNode * _xanNodeCloneTree (XanNode *node) {
    XanNode *clone;
    XanNode *child;
    XanNode *childClone;
    void    *cloneData;

    /* Assertions		*/
    assert(node != NULL);

    /* Clone the node	*/
    clone = xanNode_new(node->alloc, node->free, node->clone);
    assert(clone != NULL);
    if (node->clone != NULL) {
        cloneData = node->clone(node->data);
    } else {
        cloneData = node->data;
    }
    _xanNodeSetData(clone, cloneData);

    /* Clone the child	*/
    child = node->getNextChildren(node, NULL);
    while (child != NULL) {
        childClone = _xanNodeCloneTree(child);
        assert(childClone != NULL);
        xanList_insert(clone->childrens, childClone);
        child = node->getNextChildren(node, child);
    }

    /* Return the clone	*/
    return clone;
}

static inline XanNode * _xanNodeGetParent (XanNode *node) {

    /* Assertions			*/
    assert(node != NULL);

#ifdef DEBUG
    if (node->parent != NULL) {
        XanNode *item;
        item = node->parent->getNextChildren(node->parent, NULL);
        while (item != NULL) {
            if (item == node) {
                break;
            }
            item = node->parent->getNextChildren(node->parent, item);
        }
        assert(item != NULL);
    }
#endif

    return node->parent;
}

static inline XanNode * _xanNodeGetNextChildren (XanNode *node, XanNode *child) {
    XanListItem     *item;
    XanNode         *nextChild;

    /* Assertions			*/
    assert(node != NULL);

    /* Initialize the variables	*/
    item = NULL;
    nextChild = NULL;

    if (child == NULL) {
        item = xanList_first(node->childrens);
        if (item != NULL) {
            nextChild = (XanNode *) item->data;
            assert(nextChild != NULL);
        }
        return nextChild;
    }
    item = xanList_first(node->childrens);
    assert(item != NULL);
    while (item != NULL) {
        if (item->data == child) {
            break;
        }
        item = item->next;
    }
    assert(item != NULL);
    item = item->next;
    if (item != NULL) {
        nextChild = item->data;
        assert(nextChild != NULL);
    }
    return nextChild;
}

static inline void _xanNodeDeleteChildren (XanNode *node, XanNode *child) {

    /* Assertions			*/
    assert(node != NULL);
    assert(child != NULL);

    /* Set the parent of the child	*/
    _xanNodeSetParent(child, NULL);

    /* Delete the children          */
    xanList_removeElement(node->childrens, child, JITTRUE);

    return ;
}

static inline void _xanNodeAddChildren (XanNode *node, XanNode *child) {
    XanNode         *oldParent;

    /* Assertions			*/
    assert(node != NULL);
    assert(child != NULL);

    /* Update the old parent of the	*
     * child			*/
    oldParent = _xanNodeGetParent(child);
    if (oldParent != NULL) {
        _xanNodeDeleteChildren(oldParent, child);
    }

    /* Set the parent of the child	*/
    _xanNodeSetParent(child, node);

    /* Add the new child            */
    xanList_append(node->childrens, child);
}

static inline XanNode * _xanNodeAddNewChildren (XanNode *node, void *childData) {
    XanNode *newChild;

    /* Assertions			*/
    assert(node != NULL);

    /* Make a new node		*/
    newChild = xanNode_new(node->alloc, node->free, node->clone);
    assert(newChild != NULL);

    /* Set the parent of the child	*/
    _xanNodeSetParent(newChild, node);
    _xanNodeSetData(newChild, childData);

    /* Add the new child            */
    xanList_append(node->childrens, newChild);

    /* Return the new child		*/
    return newChild;
}

static inline void _xanNodeSetParent (XanNode *node, XanNode *parent) {

    /* Assertions		*/
    assert(node != NULL);

    node->parent = parent;
}

static inline XanList * _xanNodeGetChildrens (XanNode *node) {
    XanList         *childrens;

    /* Assertions		*/
    assert(node != NULL);

    /* Initialize the variables	*/
    childrens = NULL;

    /* Clone the list	*/
    if (xanList_length(node->childrens) > 0) {
        childrens = xanList_cloneList(node->childrens);
    }

    /* Return		*/
    return childrens;
}

static inline unsigned int _xanNodeGetDepth (XanNode *rootNode) {
    XanNode *child = NULL;
    unsigned int depth = 1;
    assert(rootNode);
    while ((child = rootNode->getNextChildren(rootNode, child))) {
        unsigned int childDepth = _xanNodeGetDepth(child) + 1;
        if (childDepth > depth) {
            depth = childDepth;
        }
    }
    return depth;
}

void print_ascii_err (signed char * message, int err) {
    print_err((char *) message, err);
}

void print_err (char * message, int err) {
    fprintf(stderr, "%s", message);
    if (err==0) {
        fprintf(stderr, "\n");
        return;
    }
    fprintf(stderr, "Error:\n           ");
    switch (err) {
        case EACCES:
            fprintf(stderr, "EACCES=%s\n", strerror(err));
            break;
        case EAGAIN:
            fprintf(stderr, "EAGAIN=%s\n", strerror(err));
            break;
        case EBADF:
            fprintf(stderr, "EBADF=%s\n", strerror(err));
            break;
        case EBUSY:
            fprintf(stderr, "EBUSY=%s\n", strerror(err));
            break;
        case ECONNREFUSED:
            fprintf(stderr, "ECONNREFUSED=%s\n", strerror(err));
            break;
        case EEXIST:
            fprintf(stderr, "EEXIST=%s\n", strerror(err));
            break;
        case EFAULT:
            fprintf(stderr, "EFAULT=%s\n", strerror(err));
            break;
        case EINVAL:
            fprintf(stderr, "EINVAL=%s\n", strerror(err));
            break;
        case EINTR:
            fprintf(stderr, "EINTR=%s\n", strerror(err));
            break;
        case EMFILE:
            fprintf(stderr, "EMFILE=%s\n", strerror(err));
            break;
        case EMSGSIZE:
            fprintf(stderr, "EMSGSIZE=%s\n", strerror(err));
            break;
        case ENAMETOOLONG:
            fprintf(stderr, "ENAMETOOLONG=%s\n", strerror(err));
            break;
        case ENFILE:
            fprintf(stderr, "ENFILE=%s\n", strerror(err));
            break;
        case ENOENT:
            fprintf(stderr, "ENOENT=%s\n", strerror(err));
            break;
        case ENOSPC:
            fprintf(stderr, "ENOSPC=%s\n", strerror(err));
            break;
        case ENOMEM:
            fprintf(stderr, "ENOMEM=%s\n", strerror(err));
            break;
        case ENOTCONN:
            fprintf(stderr, "ENOTCONN=%s\n", strerror(err));
            break;
        case ENXIO:
            fprintf(stderr, "ENXIO=%s\n", strerror(err));
            break;
        case EPERM:
            fprintf(stderr, "EPERM=%s\n", strerror(err));
            break;
        case ERANGE:
            fprintf(stderr, "ERANGE=%s\n", strerror(err));
            break;
        case ETIMEDOUT:
            fprintf(stderr, "ETIMEDOUT=%s\n", strerror(err));
            break;
        default:
            fprintf(stderr, "%s\n", strerror(err));
    }
}

int str_has_suffix (char *string, char *suffix) {
    unsigned int count;
    unsigned int count2;

    for (count = 0, count2 = 0; count<strlen(string) && count2<strlen(suffix); count++) {
        if (string[count]==suffix[count2]) {
            count2++;
        } else {
            count2 = 0;
        }
    }
    if (count==strlen(string) && count2==strlen(suffix)) {
        return 1;
    }
    return 0;
}

static inline void internalPreOrderListHelpFunction (XanNode *tree, XanList *list) {
    XanNode *child;
    void    *data;

    /* Assertions			*/
    assert(tree != NULL);
    assert(list != NULL);

    /* Get the data of the node	*/
    data = _xanNodeGetData(tree);

    /* Append the root to the list	*/
    xanList_append(list, data);

    /* Append the childrens		*/
    child = tree->getNextChildren(tree, NULL);
    while (child != NULL) {
        internalPreOrderListHelpFunction(child, list);
        child = tree->getNextChildren(tree, child);
    }
}

static inline void internalPostOrderListHelpFunction (XanNode *tree, XanList *list) {
    XanNode *child;
    void    *data;

    /* Assertions			*/
    assert(tree != NULL);
    assert(list != NULL);

    /* Get the data of the node	*/
    data = tree->getData(tree);

    /* Append the childrens		*/
    child = tree->getNextChildren(tree, NULL);
    while (child != NULL) {
        internalPostOrderListHelpFunction(child, list);
        child = tree->getNextChildren(tree, child);
    }

    /* Append the root to the list	*/
    xanList_append(list, data);
}

static inline void internalInOrderListHelpFunction (XanNode *tree, XanList *list) {
    XanNode *child;
    void    *data;

    /* Assertions			*/
    assert(tree != NULL);
    assert(list != NULL);

    /* Get the data of the node	*/
    data = tree->getData(tree);

    /* Append the first children	*/
    child = tree->getNextChildren(tree, NULL);
    if (child != NULL) {
        internalInOrderListHelpFunction(child, list);
    }

    /* Append the root to the list	*/
    xanList_append(list, data);

    /* Append the last childrens	*/
    child = tree->getNextChildren(tree, child);
    while (child != NULL) {
        internalPreOrderListHelpFunction(child, list);
        child = tree->getNextChildren(tree, child);
    }
}

void libxanCompilationFlags (char *buffer, int bufferLength) {

    /* Assertions				*/
    assert(buffer != NULL);

    snprintf(buffer, sizeof(char)*bufferLength, " ");
#ifdef DEBUG
    strncat(buffer, "DEBUG ", sizeof(char)*bufferLength);
#endif
#ifdef PRINTDEBUG
    strncat(buffer, "PRINTDEBUG ", sizeof(char)*bufferLength);
#endif
#ifdef PROFILE
    strncat(buffer, "PROFILE ", sizeof(char)*bufferLength);
#endif
}

void libxanCompilationTime (char *buffer, int bufferLength) {

    /* Assertions				*/
    assert(buffer != NULL);

    snprintf(buffer, sizeof(char) * bufferLength, "%s %s", __DATE__, __TIME__);
}

char * libxanVersion () {
    return VERSION;
}
