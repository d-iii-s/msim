#!/usr/bin/env python3

from distutils.log import error
import os
import subprocess
import filecmp

TESTS = [
    "simple"
]

MSIM_PATH = "../../msim"

DEFAULT_PWD = os.getcwd()

OUTPUT_FILENAME = "out.txt"
EXPECTED_FILENAME = "expected-output.txt"

def run_test(test_folder):
    print("test:", test_folder)
    relative_path = os.path.relpath(MSIM_PATH, test_folder)
    try:
        os.chdir(test_folder)
        res = subprocess.run(relative_path, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        assert(res.returncode == 0)
        assert(filecmp.cmp(EXPECTED_FILENAME, OUTPUT_FILENAME))
        os.remove(OUTPUT_FILENAME)
    except:
        print("failure!")
        exit(-1)
    finally:
        os.chdir(DEFAULT_PWD)
    print("success")

def main():
    for test in TESTS:
        run_test(test)

if __name__ == "__main__":
    main()