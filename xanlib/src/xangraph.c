/*
 * Copyright (C) 2011 - 2012 Campanoni Simone
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

void xanGraph_destroyGraph (XanGraph *g) {
    XanHashTableItem	*item;

    /* Check the graph			*/
    if (g == NULL) {
        return ;
    }

    /* Free the memory			*/
    item	= xanHashTable_first(g->nodes);
    while (item != NULL) {
        XanGraphNode	*n;
        n	= item->element;
        xanHashTable_destroyTable(n->incomingEdges);
        xanHashTable_destroyTable(n->outgoingEdges);
        item	= xanHashTable_next(g->nodes, item);
    }
    xanHashTable_destroyTableAndData(g->edges);
    xanHashTable_destroyTableAndData(g->nodes);
    g->free(g);

    /* Return				*/
    return ;
}

XanGraph * xanGraph_new (void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void *(*reallocFunction)(void *addr, size_t newSize), void *data) {
    XanGraph	*g;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        g		= malloc(sizeof(XanGraph));
        g->alloc	= malloc;
    } else {
        g		= allocFunction(sizeof(XanGraph));
        g->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        g->free		= free;
    } else {
        g->free		= freeFunction;
    }
    if (reallocFunction == NULL) {
        g->realloc	= realloc;
    } else {
        g->realloc	= reallocFunction;
    }

    /* Fill up the structure		*/
    g->nodes	= xanHashTable_new(11, JITFALSE, g->alloc, g->realloc, g->free, NULL, NULL);
    g->edges	= xanHashTable_new(11, JITFALSE, g->alloc, g->realloc, g->free, NULL, NULL);
    g->data		= data;

    /* Return				*/
    return g;
}

XanGraphNode * xanGraph_addANewNodeIfNotExist (XanGraph *g, void *data) {
    XanGraphNode    *n;

    n   =  xanGraph_getNode(g, data);
    if (n == NULL){
        n   = xanGraph_addANewNode(g, data);
    }
    assert(n != NULL);
    assert(n->data == data);

    return n;
}

XanGraphNode * xanGraph_addANewNode (XanGraph *g, void *data) {
    XanGraphNode	*n;

    /* Assertions				*/
    assert(g != NULL);

    /* Allocate the node			*/
    n			= g->alloc(sizeof(XanGraphNode));

    /* Fill up the node			*/
    n->data			= data;
    n->incomingEdges	= xanHashTable_new(11, JITFALSE, g->alloc, g->realloc, g->free, NULL, NULL);
    n->outgoingEdges	= xanHashTable_new(11, JITFALSE, g->alloc, g->realloc, g->free, NULL, NULL);

    /* Add the node to the graph		*/
    xanHashTable_insert(g->nodes, data, n);

    /* Return				*/
    return n;
}

void xanGraph_addUndirectedEdge (XanGraph *g, XanGraphNode *n1, XanGraphNode *n2, void *data) {
    xanGraph_addDirectedEdge(g, n1, n2, data);
    xanGraph_addDirectedEdge(g, n2, n1, data);
}

XanGraphEdge * xanGraph_addDirectedEdge (XanGraph *g, XanGraphNode *n1, XanGraphNode *n2, void *data) {
    XanGraphEdge	*e;

    /* Assertions				*/
    assert(n1 != NULL);
    assert(n2 != NULL);

    /* Add the edge				*/
    e	= xanHashTable_lookup(n1->outgoingEdges, n2);
    if (e != NULL) {
        fprintf(stderr, "XanLib: Edge already exists.\n");
        abort();
    }
    assert(xanHashTable_lookup(n2->incomingEdges, n1) == NULL);
    e	= g->alloc(sizeof(XanGraphEdge));
    e->data	= data;
    e->p1	= n1;
    e->p2	= n2;
    xanHashTable_insert(g->edges, data, e);
    xanHashTable_insert(n1->outgoingEdges, n2, e);
    xanHashTable_insert(n2->incomingEdges, n1, e);

    /* Return				*/
    return e;
}

int xanGraph_existDirectedEdge (XanGraphNode *n1, XanGraphNode *n2) {
    return (xanHashTable_lookup(n1->outgoingEdges, n2) != NULL);
}

XanGraphNode * xanGraph_getNode (XanGraph *g, void *data) {
    return xanHashTable_lookup(g->nodes, data);
}

XanGraphEdge * xanGraph_getEdge (XanGraph *g, void *data) {
    return xanHashTable_lookup(g->edges, data);
}

XanGraphEdge * xanGraph_getDirectedEdge (XanGraphNode *n1, XanGraphNode *n2) {
    return xanHashTable_lookup(n1->outgoingEdges, n2);
}
