# `libmoeprlnc` Validator


The tool simulates two communication partners trying to exchange random data with each other using
the rlcn library. If both communication partners do not have the same data at the end or unexpected
events occur, the program terminates and displays the error on the console. The tool's random nature
is aimed at detecting particularly hard-to-find errors.

The tool can also be used for statistical evaluations of the rlcn library.

For more information and documentation, we recommend reading the project report.


## Dependencies

This tool requires the installation of the `moep80211ncm` library, which in turn depends on the
`libmoep` injection library. For more information read the respective README files of the libraries.
To install the `moep80211ncm` use `sudo make install` as a last step after you followed the
library's readme.



## Build and Run
To build the tool you can just use `make` inside the `src/moepvalidator` which will create a binary
called `main` in the `build` folder in `src/moepvalidator`. Run `./build/main --help` to see all
available arguments or see the section below.

Please note that we have expierenced that on some systems the environment variable `LD_LIBRARY_PATH`
need to be set to `/usr/local/lib` (or the place where you isntalled `libmoep` and `moep8021ncm`) in
order for the linker to find the libraries.



Further `make` options are
* `make debug` which create `build/debug/main` without optimization for debugging
* `make clean` and `make clean_debug` to delete the respective build files

We have created `launch.json` and `task.json` files which define a task that automatically sets the
required environment variable, builds the tool and turns on VS Code's debugger.

## Program Arguments
```
    -c, --csv_stats=NAME
    Print statistics to CSV file with the given path.

    -f, --field_size=SIZE
    Set underlying Galois Field size. (0: GF2, 1: GF4, 2: GF16, 3: GF256).

    -g, --gen_size=SIZE
    Set the generation size.

    -i, --nr_iterations=ITER
    Set the amount of test iterations

    -l, --loss_rate=LOSS
    Set probability with which coded data is lost during transmission.

    -m, --mode
    Executes the program in 'pre fill' mode.

    -p, --pkt_size=SIZE
    Set the frame size.

     -s, --seed=ADDR
    Set the seed which is used to generate random test input.

    -v, --verbose
    Produce verbose output.
```

## Python Tool and Jupyter Notebook

We have created a python tool for easy multithreaded execution of the validation tool when
one want to test several different configurations. Furthermore, we used a jupyter notebook which
uses this tool to create the CSV files of our plots. The notebook also provides code to read out
the files and actually created the plots. So given the notebook it should be relatively easy to
recreate the plots.

Both files are located in `evaluation` as `eval.ipynb` and `mass_validation.py`. The latter even provides
a command line tool. See the inline documentation for further details.

In order to use our python tools one has to install our python dependencies. This can for example
be done within a virtual python environment. (Code has only been tested on Python 3.9):
```shell
cd evaluation
virualenv --python=python3.9 venv
source venv/bin/activate
pip install -r requirements.txt
# To see the available arguments of the python tool run
python mass_validation.py --help
# To start jupyter notebook run the following command and select the file `eval.ipynb` in your browser
jupyter notebook
```

