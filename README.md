# shamir secret sharing

## original code

### copyright

/*
 *  ssss version 0.5  -  Copyright 2005,2006 B. Poettering
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 */

### message

/*
 * http://point-at-infinity.org/ssss/
 *
 * This is an implementation of Shamir's Secret Sharing Scheme. See
 * the project's homepage http://point-at-infinity.org/ssss/ for more
 * information on this topic.
 *
 * This code links against the GNU multiprecision library "libgmp".
 * I compiled the code successfully with gmp 4.1.4.
 * You will need a system that has a /dev/random entropy source.
 *
 * Compile with
 * "gcc -O2 -lgmp -o ssss-split ssss.c && ln ssss-split ssss-combine"
 *
 * Compile with -DNOMLOCK to obtain a version without memory locking.
 *
 * Report bugs to: ssss AT point-at-infinity.org
 *
 */

## Compilation

* Use the included Makefile to compile
* requires libgmp to be installed
* accesses /dev/random

## What changed

Code refactored into two parts:
* `shamir.c` is made to be usable as a separate library
* `main.c` is mostly commandline compatible driver
* clean up of syntax such as:
    + use of C99 like `for (int i =...`
    * Always using braces (to avoid any *goto fails*)
* `shamir.h` is the API
* rename ssss to shamir
