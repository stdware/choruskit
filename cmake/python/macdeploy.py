# Deploy ChorusKit Application on MacOS

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys


class Global:
    prefix: str = ""
    verbose: bool = False


def print_verbose(*va_args):
    if Global.verbose:
        print(*va_args)


def main():
    parser = argparse.ArgumentParser(
        description="Deploy ChorusKit Application on Windows")
    parser.add_argument("--prefix", metavar="<path>",
                        help="Install prefix, default to current dir.", type=str, default="")
    parser.add_argument("--qmake", metavar="<path>",
                        help="Qt qmake path.", type=str, required=True)
    parser.add_argument("--petool", metavar="<path>",
                        help="PE file dump tool.", type=str, required=True)
    parser.add_argument("--dirs", metavar="<path>",
                        help="Library searching paths.", nargs="+", required=True)
    parser.add_argument(
        "--debug", help="Assume debug binaries.", action="store_true")
    parser.add_argument(
        "--verbose", help="Show deploy progress.", action="store_true")
    parser.add_argument("--files", nargs="+")
    args = parser.parse_args()


if __name__ == "__main__":
    main()
