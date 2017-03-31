
/*
 * Copyright (C) 2012 - 2015  Niall Murphy
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

#include <tuple>
#include <cstdint>
using namespace std;

/* 
 * Returns (lowest clashing address, stride between clashes, number of clashes)
 * See https://smartech.gatech.edu/handle/1853/38798
 * Ranges must be at least overlapping
 */
typedef tuple<uintptr_t, uintptr_t, uintptr_t> AliasT;
AliasT dynamic_gcd(uintptr_t low1, uintptr_t low2, uintptr_t high1, uintptr_t high2, uintptr_t stride1, uintptr_t stride2);
