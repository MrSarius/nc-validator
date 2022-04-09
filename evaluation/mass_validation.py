"""
Copyright 2021,2022		Tobias Juelg <tobias.juelg@tum.de>
			            Ben Riegel <ben.riegel@tum.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

See COPYING for more details.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from multiprocessing import Pool
from typing import Dict, Optional, List
import itertools


def validate_f(args: Dict):
    validate(**args)


def validate(
    gen_size: int,
    iterations: int = 5000,
    packet_size: int = 50,
    file_name: Optional[str] = None,
    prefill: bool = True,
    galois_field: int = 256
):
    # This function only works when the compiled validator lies in src/moepvalidator/build/main
    ex_cmd = "../src/moepvalidator/build/main -g {} -i {}{}-p {} -f {}{}"
    gf2id = {2: 0, 4: 1, 16: 2, 256: 3}
    if galois_field not in gf2id:
        raise RuntimeError(
            "Only the fields q in {2, 4, 16, 256} are supported!")
    re = os.system(ex_cmd.format(gen_size, iterations,
                   "" if not file_name else f" -c {file_name} ", packet_size, gf2id[galois_field], " -m"if prefill else ""))
    if re != 0:
        print("Problem in validation with following parameters: ")
        print(
            f"{gen_size=}, {iterations=}, {packet_size=}, {file_name=}, {prefill=}, {galois_field=}")
        assert False

# args: folder, field as array, iterations, packet size, bool prefill, generation size


def validate_parallel(
    folder: Optional[str] = None,
    file_name: str = "valdation_stats_{gf}_{gs}_{ps}_{prefill}.csv",
    gf: Optional[List[int]] = None,
    gs: Optional[List[int]] = None,
    ps: Optional[List[int]] = None,
    prefill: bool = True,
    n_processes: Optional[int] = None,
    iterations: int = 5000
):
    # default parameters
    if gs is None:
        gs = range(1, 100)

    if gf is None:
        gf = [256]
    if ps is None:
        ps = [50]

    if folder:
        os.makedirs(folder, exist_ok=True)
    if n_processes == None:
        n_processes = os.cpu_count()

    args = []
    for i, j, k in itertools.product(gs, gf, ps):
        args.append({"gen_size": i, "galois_field": j, "packet_size": k, "iterations": iterations,
                     "file_name": file_name.format(gf=j, gs=i, ps=k, prefill=prefill), "prefill": prefill})

    with Pool(n_processes) as p:
        p.map(validate_f, args)


if __name__ == "__main__":
    # command line args, with python style arrays
    exit(0)
