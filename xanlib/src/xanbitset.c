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


/* number of bits in a word	*/
#define BITS_IN_A_WORD ( 8 * sizeof(size_t) )

/* mask with ths pos-st bit set. Ex: pos = 9 => 0000000001000000000000000000000	*/
#define MASK(pos) ( ((size_t) 1) << ((pos) % BITS_IN_A_WORD) )

/* the number of the word on which the mask should be applied	*/
#define WORD_NUMBER(pos) ((pos) / BITS_IN_A_WORD)

/* the whole word corresponding to a bit, on which the mask should be applied. */
#define WHOLE_WORD(set, pos) ( (set)->data[WORD_NUMBER(pos)] )

/* number of words necessary to hold a bitset with length length	*/
#define WORDS_NECESSARY(length)	\
	( ((length) + (BITS_IN_A_WORD - 1)) / BITS_IN_A_WORD )

/* minimum of two numbers */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* maximum of two numbers */
#define MAX(a, b) ((a) < (b) ? (b) : (a))

/* Allocate extra capacity (1.5x) to allow the bitset to expand without reallocation */
#define CAPACITY_NECESSARY(length) WORDS_NECESSARY(length + (length >> 1))

XanBitSet* xanBitSet_new (size_t length) {
    XanBitSet *set;

    set = malloc(sizeof(XanBitSet));
    assert(set != NULL);

    set->capacity = CAPACITY_NECESSARY(length);
    set->data = calloc(set->capacity, sizeof(size_t));
    set->length = length;
    assert(set->data);

    return set;
}

static inline void _xanBitSet_expand (XanBitSet *set, size_t length) {
    size_t i, oldWords, newWords;
    size_t newCapacity;
    size_t *newData;

    assert(length > set->length);
    oldWords = WORDS_NECESSARY(set->length);
    newWords = WORDS_NECESSARY(length);
    if (newWords <= set->capacity) {
        /* We have the capacity, just set new length and return */
        set->length = length;
        return;
    }

    /* Set needs more capacity, allocate and copy */
    newCapacity = CAPACITY_NECESSARY(length);
    newData = calloc(newCapacity, sizeof(size_t));
    assert(newData);

    for (i = 0; i < oldWords; ++i) {
        newData[i] = set->data[i];
    }

    free(set->data);
    set->data = newData;
    set->length = length;
    set->capacity = newCapacity;
}

void xanBitSet_intersect (XanBitSet *dest, XanBitSet *src) {
    size_t i;
    size_t words;
    size_t srcWords;
    size_t destWords;

    srcWords = WORDS_NECESSARY(src->length);
    destWords = WORDS_NECESSARY(dest->length);
    words = MIN(srcWords, destWords);
    for (i = 0; i < words; ++i) {
        dest->data[i] &= src->data[i];
    }
    for (; i < destWords; ++i) {
        dest->data[i] = 0;
    }
}

void xanBitSet_union (XanBitSet *dest, XanBitSet *src) {
    size_t i;

    if (src->length > dest->length) {
        _xanBitSet_expand(dest, src->length);
    }

    for (i = 0; i < WORDS_NECESSARY(src->length); ++i) {
        dest->data[i] |= src->data[i];
    }
}

bool xanBitSet_equal (XanBitSet *bs1, XanBitSet *bs2) {
    if (bs1->length == bs2->length) {
        return memcmp(bs1->data, bs2->data, WORDS_NECESSARY(bs1->length) * sizeof(size_t)) == 0;
    } else {
        XanBitSet *shorter;
        XanBitSet *longer;
        size_t wordsShort;
        if (bs1->length < bs2->length) {
            shorter = bs1;
            longer = bs2;
        } else {
            shorter = bs2;
            longer = bs1;
        }
        wordsShort = WORDS_NECESSARY(shorter->length);
        if (memcmp(shorter->data, longer->data, wordsShort * sizeof(size_t)) == 0) {
            size_t i, wordsLong = WORDS_NECESSARY(longer->length);
            for (i = wordsShort; i < wordsLong; ++i) {
                if (longer->data[i] != 0) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

XanBitSet * xanBitSet_clone (XanBitSet *src) {
    XanBitSet       *dest;

    dest = xanBitSet_new(src->length);
    memcpy(dest->data, src->data, WORDS_NECESSARY(src->length) * sizeof(size_t));
    return dest;
}

void xanBitSet_copy (XanBitSet *dest, XanBitSet *src) {

    if (src->length > dest->length) {
        _xanBitSet_expand(dest, src->length);
    }

    memcpy(dest->data, src->data, WORDS_NECESSARY(src->length) * sizeof(size_t));

    /* Could reduce length of dest here. */
    if (dest->length > src->length) {
        size_t i;
        for (i = WORDS_NECESSARY(src->length); i < WORDS_NECESSARY(dest->length); ++i) {
            dest->data[i] = 0;
        }
    }
}

void xanBitSet_print (XanBitSet *set, int cr) {
    size_t i;

    for (i = 0; i < set->length; ++i) {
        fprintf(stdout, "%d", ((WHOLE_WORD(set, i) & MASK(i))!=0));
    }
    if (cr) {
        fprintf(stdout, "\n");
    }
}

void xanBitSet_clearAll (XanBitSet *set) {
    //WORDS_NECESSARY(set->length) * sizeof(*set->data) = n° di Byte occupati dal xan_bitset
    memset(set->data, 0, WORDS_NECESSARY(set->length) * sizeof(*set->data));
}

void xanBitSet_setAll (XanBitSet *set) {
    //0xFF = 11111111
    memset(set->data, 0xFF, WORDS_NECESSARY(set->length) * sizeof(*set->data));
}

void xanBitSet_invertBits (XanBitSet *set) {
    size_t i;

    for (i = 0; i < WORDS_NECESSARY(set->length); ++i) {
        set->data[i] = ~(set->data[i]);
    }
}

void xanBitSet_subtract (XanBitSet *fromThis, XanBitSet *subtractThis) {
    size_t i, words;

    words = MIN(WORDS_NECESSARY(subtractThis->length), WORDS_NECESSARY(fromThis->length));
    for (i = 0; i < words; ++i) {
        size_t bitmask = ~(subtractThis->data[i]);
        fromThis->data[i] &= bitmask;
    }
}

void xanBitSet_free (XanBitSet *set) {
    free(set->data);
    free(set);
}

void xanBitSet_clearBit (XanBitSet *set, size_t pos) {
    if (pos < set->length) {
        WHOLE_WORD(set, pos) &= ~MASK(pos);
    }
}

void xanBitSet_setBit (XanBitSet *set, size_t pos) {
    if (pos >= set->length) {
        _xanBitSet_expand(set, pos + 1);
    }
    WHOLE_WORD(set, pos) |= MASK(pos);
}

void xanBitSet_setBits (XanBitSet *set, size_t start, size_t end) {
    size_t pos;
    if (end >= set->length) {
        _xanBitSet_expand(set, end + 1);
    }
    for (pos = start; pos <= end; ++pos) {
        WHOLE_WORD(set, pos) |= MASK(pos);
    }
}

static inline bool is_bit_set (XanBitSet *set, size_t pos) {
    return (WHOLE_WORD(set, pos) & MASK(pos)) != 0;
}

bool xanBitSet_isBitSet (XanBitSet *set, size_t pos) {
    if (pos < set->length) {
        return is_bit_set(set, pos);
    } else {
        return false;
    }
}

bool xanBitSet_isSubSetOf (XanBitSet *setA, XanBitSet *setB) {
    size_t i, length, aLength;

    aLength = setA->length;
    length = MIN(aLength, setB->length);
    for (i = 0; i < length; ++i) {
        if (is_bit_set(setA, i) && !is_bit_set(setB, i)) {
            return false;
        }
    }
    for (; i < aLength; ++i) {
        if (is_bit_set(setA, i)) {
            return false;
        }
    }

    return true;
}

bool xanBitSet_isIntersectionEmpty (XanBitSet *setA, XanBitSet *setB) {
    size_t i, words;

    words = MIN(WORDS_NECESSARY(setA->length), WORDS_NECESSARY(setB->length));
    for (i = 0; i < words; ++i) {
        if ((setA->data[i] & setB->data[i]) != 0) {
            return false;
        }
    }

    return true;
}

bool xanBitSet_isEmpty (XanBitSet *set) {
    size_t i, words;

    words = WORDS_NECESSARY(set->length);
    for (i = 0; i < words; ++i) {
        if (set->data[i] != 0) {
            return false;
        }
    }

    return true;
}

bool xanBitSet_areEqual (XanBitSet *setA, XanBitSet *setB) {
    size_t i, words, wordsA, wordsB;

    wordsA = WORDS_NECESSARY(setA->length);
    wordsB = WORDS_NECESSARY(setB->length);
    words = MIN(wordsA, wordsB);
    for (i = 0; i < words; ++i) {
        if (setA->data[i] != setB->data[i]) {
            return false;
        }
    }

    if (wordsA > wordsB) {
        for (i = words; i < wordsA; ++i) {
            if (setA->data[i] != 0) {
                return false;
            }
        }
    } else if (wordsB > wordsA) {
        for (i = words; i < wordsB; ++i) {
            if (setB->data[i] != 0) {
                return false;
            }
        }
    }

    return true;
}

int xanBitSet_getCountOfBitsSet (XanBitSet *set) {
    size_t 	i;
    size_t	count;

    count = 0;

    if (sizeof(size_t) == 4) {
        size_t	value;
        size_t	words;

        words	= (set->length + 31)/32;
        assert(words > 0);
        for (i=0; i < words; i++) {
            value	= set->data[i];
            value 	= value - ((value >> 1) & 0x55555555);
            value 	= (value & 0x33333333) + ((value >> 2) & 0x33333333);
            count	+= ((((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24);
        }

    } else {

        for (i = 0; i < set->length; ++i) {
            if (is_bit_set(set, i)) {
                count++;
            }
        }
    }

    return count;
}

int xanBitSet_getFirstBitSetInRange(XanBitSet *set, size_t start, size_t end) {
    size_t w, sWord, eWord;

    end = MIN(end, set->length - 1);
    sWord = WORD_NUMBER(start);
    eWord = WORD_NUMBER(end);
    for (w = sWord; w <= eWord; ++w) {
        if (set->data[w] != 0) {
            size_t pos = w * BITS_IN_A_WORD;
            size_t wordEnd = pos + BITS_IN_A_WORD - 1;
            pos = MAX(pos, start);
            wordEnd = MIN(wordEnd, end);
            for (; pos <= wordEnd; ++pos) {
                if (is_bit_set(set, pos)) {
                    return (int)pos;
                }
            }
        }
    }

    return -1;
}

int xanBitSet_getFirstBitUnsetInRange(XanBitSet *set, size_t start, size_t end) {
    size_t w, minEnd, sWord, eWord;

    minEnd = MIN(end, set->length - 1);
    sWord = WORD_NUMBER(start);
    eWord = WORD_NUMBER(minEnd);
    for (w = sWord; w <= eWord; ++w) {
        if (set->data[w] != (~((size_t)0))) {
            size_t pos = w * BITS_IN_A_WORD;
            size_t wordEnd = pos + BITS_IN_A_WORD - 1;
            pos = MAX(pos, start);
            wordEnd = MIN(wordEnd, minEnd);
            for (; pos <= wordEnd; ++pos) {
                if (!is_bit_set(set, pos)) {
                    return (int)pos;
                }
            }
        }
    }

    if (end >= set->length) {
        return set->length;
    }

    return -1;
}

size_t xanBitSet_length(XanBitSet *set) {
    return set->length;
}

size_t xanBitSet_capacity(XanBitSet *set){
    return set->capacity * sizeof(size_t);
}
