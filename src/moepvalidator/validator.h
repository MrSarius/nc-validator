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
#ifndef __VALIDATOR_H_
#define __VALIDATOR_H_

int validate(size_t iterations, size_t packet_size, size_t generation_size, float loss_rate, unsigned int seed, int gftype, bool csv, bool prefill);

#endif