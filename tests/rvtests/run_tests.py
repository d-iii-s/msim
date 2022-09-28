#!/usr/bin/env python3

from distutils.log import error
import os
import subprocess
import filecmp
from sys import stdout

TESTS = [
    "simple",
    "loads",
    "stores",
    "ops",
    "jumps",
    "m-extension"
]

MSIM_PATH = "../../msim"

DEFAULT_PWD = os.getcwd()

OUTPUT_FILENAME = "out.txt"
EXPECTED_FILENAME = "expected-output.txt"

def run_test(test_folder):
    print("test: {t}".format(t=test_folder).ljust(30, ' '), end="")
    relative_path = os.path.relpath(MSIM_PATH, test_folder)
    try:
        os.chdir(test_folder)
        res = subprocess.run(relative_path, capture_output=True, timeout=1000, check=True, text=True)
        
        # Test didn't use printer, probably because it uses register dumps instead
        # Then use stdout as reference
        if not os.path.exists(OUTPUT_FILENAME):
            with open(OUTPUT_FILENAME, 'w') as f:
                for b in res.stdout:
                    f.write(b)

        assert filecmp.cmp(EXPECTED_FILENAME, OUTPUT_FILENAME), "Files do not match!"
        os.remove(OUTPUT_FILENAME)
    except BaseException  as e:
        print("failure! ({e})".format(e=e))
        exit(-1)

    os.chdir(DEFAULT_PWD)
    print("success")

def main():
    for test in TESTS:
        run_test(test)

if __name__ == "__main__":
    main()