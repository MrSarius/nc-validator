/*
 * Copyright 2021,2022		Tobias Juelg <tobias.juelg@tum.de>
 *				            Ben Riegel <ben.riegel@tum.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See COPYING for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdbool.h>

void setVerbose(bool v);

void logger(const char *format, ...);

double prop_linear_independent(size_t generation_size, int gftype);

void assert(bool exp, const char *format, ...);

void set_seed(unsigned int seed);

float randf();

int randint(int from, int to);

uint8_t randbyte();

void randbytes(size_t n, uint8_t *a);

size_t aligned_length(size_t len, size_t alignment);

#endif