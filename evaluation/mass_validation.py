import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from multiprocessing import Pool

def validate_f(args):
    validate(**args)


def validate(gen_size, iterations=5000, packet_size=50, file_name=None, prefill=True, galois_field=256):
    # This function only works when the compiled validator lies in source/moepvalidator/build/main
    ex_cmd = "../source/moepvalidator/build/main -g {} -i {}{}-p {} -f {}{}"
    gf2id = {2: 0, 4: 1, 16: 2, 256: 3}
    if galois_field not in gf2id:
        raise RuntimeError("Only the fields q in {2, 4, 16, 256} are supported!")
    re = os.system(ex_cmd.format(gen_size, iterations, "" if not file_name else f" -c {file_name} ", packet_size, gf2id[galois_field], " -m"if prefill else ""))
    if re != 0:
        print("Problem in validation with following parameters: ")
        print(f"{gen_size=}, {iterations=}, {packet_size=}, {file_name=}, {prefill=}, {galois_field=}")
        assert False

# args: folder, field as array, iterations, packet size, bool prefill, generation size
def validate_parallel(folder=None, file_name="valdation_stats_{gf}_{gs}_{ps}_{prefill}.csv", gf=None, gs=None, ps=None, prefill=True, n_processes=None, iterations=5000):
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
    # this can be done more beautiful with cross product
    for i in gs:
        for j in gf:
            for k in ps:
                args.append({"gen_size": i, "galois_field": j, "packet_size": k, "iterations": iterations, "file_name": file_name.format(gf=j, gs=i, ps=k, prefill=prefill), "prefill": prefill})


    with Pool(n_processes) as p:
        p.map(validate_f, args)



if __name__ == "__main__":
    # command line args, with python style arrays
    exit(0)
