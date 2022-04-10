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

import os
from multiprocessing import Pool
from typing import Dict, Optional, List
import itertools
import argparse


def validate_f(args: Dict):
    """Helper function to convert dict args into python args.
    """
    validate(**args)


def validate(
    gen_size: int,
    iterations: int = 5000,
    packet_size: int = 50,
    file_name: Optional[str] = None,
    prefill: bool = True,
    galois_field: int = 256
):
    """Python wrapper for the validator C program

    The only difference compared to the arguments of the C program is that the Galois field is given as its actual q value instead of the enum.

    This function only works when the compiled validator lies in src/moepvalidator/build/main

    Args:
        gen_size (int): generation size in bytes
        iterations (int, optional): Number of iterations of the validator. Defaults to 5000.
        packet_size (int, optional): Packet size in bytes for the validator. Defaults to 50.
        file_name (Optional[str], optional): Path the the CSV file. None means that the statistically output is not saved. Defaults to None.
        prefill (bool, optional): Whether or not prefill mode should be used. If no prefill mode should be used than random order mode is used
            See the reademe for more details of the modes. Defaults to True.
        galois_field (int, optional): Galois field q, possible values are 2, 4, 16 and 256. Defaults to 256.

    Raises:
        RuntimeError: If invalid Galois field is given
    """
    ex_cmd = "../src/moepvalidator/build/main -g {} -i {}{}-p {} -f {}{}"
    gf2id = {2: 0, 4: 1, 16: 2, 256: 3}
    if galois_field not in gf2id:
        raise RuntimeError(
            "Only the fields q in {2, 4, 16, 256} are supported!")
    # print(ex_cmd.format(gen_size, iterations,
    #                " " if not file_name else f" -c {file_name} ", packet_size, gf2id[galois_field], " -m"if prefill else ""))
    re = os.system(ex_cmd.format(gen_size, iterations,
                   " " if not file_name else f" -c {file_name} ", packet_size, gf2id[galois_field], " -m"if prefill else ""))
    if re != 0:
        print("Problem in validation with following parameters: ")
        print(
            f"{gen_size=}, {iterations=}, {packet_size=}, {file_name=}, {prefill=}, {galois_field=}")
        assert False


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
    """Executes the python wrapper for the C validator `validate` in a parallel fession in order to speed up mass validations that use several configurations.

    galois field, generation size and packet size can all be a list of integers or None which will use the default values described below.
    This function will create the cross products of these list and execute all possible combinations.

    Args:
        folder (Optional[str], optional): Path to the folder where the csv files should be saved.
            If this is set to None then no csv files will be created. Defaults to None.
        file_name (str, optional): Name of the resulting csv files, this can be defined in python
            format string syntax with the values `gf`, `gs`, `ps` and `prefill` which will be replace 
            with the respective values of galois field, generation size, packet size and whether prefill
            mode is used for each run. Defaults to "valdation_stats_{gf}_{gs}_{ps}_{prefill}.csv".
        gf (Optional[List[int]], optional): Galois field. Defaults to None which is translated to [256].
        gs (Optional[List[int]], optional): Generation size bytes. Defaults to None which is translated to range(1, 100).
        ps (Optional[List[int]], optional): Packet size in bytes. Defaults to None which is translated to [50].
        prefill (bool, optional): Prefill mode if True else random mode. See readme for more details. Defaults to True.
        n_processes (Optional[int], optional): Number of processes that work through the given configurations in parallel. Zero means same thread, None
            means number of CPUs of the machine. Defaults to None.
        iterations (int, optional): Number of iterations for each validation. Defaults to 5000.
    """
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
        path = os.path.join(folder, file_name.format(
            gf=j, gs=i, ps=k, prefill=prefill)) if folder else None
        args.append({"gen_size": i, "galois_field": j, "packet_size": k, "iterations": iterations,
                     "file_name": path, "prefill": prefill})

    if n_processes == 0:
        print(f"Starting to validate single threaded")
        for arg in args:
            validate_f(arg)
    else:
        print(f"Starting to validate with {n_processes} processes")
        with Pool(n_processes) as p:
            p.map(validate_f, args)


def main():
    """Parses command line args and validates"""
    # define args
    parser = argparse.ArgumentParser(
        description='Python tool to run the libmoeprlnc C validation with several settings in parallel.')
    parser.add_argument('-fo', '--folder', type=str, default=None,
                        help='Path to the location where the CSV files should be saved. If not set then no CSV files will be created.')
    parser.add_argument('-f', '--file_name', type=str, default="valdation_stats_{gf}_{gs}_{ps}_{prefill}.csv",
                        help='file name template for the csv files. The filename can include `{gf}`, `{gs}`, `{ps}`, and `{prefill}` tags which will be replaced with the respective values of galois field size, generation size, packet size and whether prefill mode was used.')
    parser.add_argument('-gf', '--galois_field', type=str, default="[256]",
                        help='Actual Galois field sizes (q values) which should be validated. The value needs to be a string with a valid definition of a python int iterable. Everything else is not checked and leads to undefined behavior. You can for example use "[2, 4]".')
    parser.add_argument('-gs', '--generation_size', type=str, default="range(1, 100)",
                        help='Array of generation sizes to validate. The value needs to be a string with a valid definition of a python int iterable. Everything else is not checked and leads to undefined behavior. You can for example use "[50, 100]" or "range(1, 100, 2)".')
    parser.add_argument('-ps', '--packet_size', type=str, default="[50]",
                        help='Packet sizes to validate. The value needs to be a string with a valid definition of a python int iterable. Everything else is not checked and leads to undefined behavior. You can for example use "[50, 100]" or "range(50, 1000, 50)".')
    parser.add_argument('-pf', '--prefill', type=bool, default=True,
                        help='Whether or not the prefill mode should be used. If not the random order mode will be used. For more details on the modes see the readme page.')
    parser.add_argument('-j', '--n_processes', type=int, default=None,
                        help='Number of worker processes that should be used for the parallel validation. If set to zero no extra process will be spawned and the existing python thread will be used to execute the validation sequentially.')
    parser.add_argument('-n', '--iterations', type=int, default=5000,
                        help='Number of iterations for each setting / execution of the validation tool.')
    # parse args and evaluate array expressions
    args = parser.parse_args()
    args.galois_field = eval(args.galois_field)
    args.generation_size = eval(args.generation_size)
    args.packet_size = eval(args.packet_size)

    # execute the given setting in parallel
    validate_parallel(args.folder, args.file_name, args.galois_field, args.generation_size,
                      args.packet_size, args.prefill, args.n_processes, args.iterations)


if __name__ == "__main__":
    main()
    exit(0)
