# Deploy ChorusKit Application on Windows

# Tasks:
#   1. copy `qtmediate` and `choruskit` libraries, plugins and resources
#   2. copy other third-party libraries
#   3. copy Qt libraries, plugins and resources

# References:
#   1. qmake path: find `windeployqt.exe` path
#   2. windeps path: resolve other dependencies of PE files
#   3. library searching paths: find and copy dependencies, should be a UNIX `bin` directory
#   4. PE files: executable and plugins

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


def deploy_qt_binaries(qmake_dir: str, libdir: str, plugindir: str, files: list[str]) -> int:
    # Find Windows qt deploy tool
    windeployqt = qmake_dir + "/windeployqt.exe"
    if not os.path.isfile(windeployqt):
        print("windeployqt.exe not found!!!")
        return -1

    # Run windeployqt
    cmds: list[str] = [
        windeployqt,
        "--libdir", libdir,
        "--plugindir", plugindir,
        "--no-translations",
        "--no-system-d3d-compiler",
        "--no-compiler-runtime",
        "--no-opengl-sw",
        "--no-angle",
        "--force",
    ]
    if not Global.verbose:
        cmds += ["--verbose", "0"]
    cmds += files

    # Add qmake directory to PATH, this is very IMPORTANT
    # If we don't do this, the deploy process will fail if there's anohter qt directory in global PATH
    path_var = os.environ["PATH"]
    os.environ["PATH"] = qmake_dir + \
        os.pathsep + os.environ.get("PATH", "")

    # Run command
    result = subprocess.run(cmds)
    code = result.returncode

    # Restore PATH
    os.environ["PATH"] = path_var

    if code != 0:
        print("Deploy Qt binaries failed")

    return code


def deploy_3rdparty_libraries(petool: str, libdir: str, files: list[str], searching_dirs: list[str] = []) -> int:
    if not os.path.isfile(petool):
        print("petool not found!!!")
        return -1

    # Use a set to store library names which have been collected
    name_set: set[str] = set()
    for item in files:
        name_set.add(os.path.basename(item).lower())

    # Recursively resolve dependencies
    stack: list[str] = files
    dependencies: list[str] = []
    while len(stack) > 0:
        # Run command
        result = subprocess.run(
            [petool] + stack, stdout=subprocess.PIPE, text=True)
        code = result.returncode
        if code != 0:
            print("Deploy 3rdparty libraries failed")
            return code

        stack.clear()
        for item in result.stdout.split("\n"):
            item_lower = item.lower()

            # Skip Qt libraries
            if item.startswith("Qt"):
                continue
            # Skip MSVC libraries
            if item_lower.startswith("vcruntime") or item_lower.startswith("msvc"):
                continue
            # Skip win-crt libraries
            if item_lower.startswith("api-ms-win-crt"):
                continue
            # Skip Windows system libraries
            if os.path.exists("C:\\Windows\\system32\\" + item) or os.path.exists("C:\\Windows\\SysWow64\\" + item):
                continue
            # Skip if collected
            if item_lower in name_set:
                continue
            name_set.add(item_lower)

            # Search
            path = ""
            for dir in searching_dirs:
                temp_path = dir + "/" + item
                if os.path.exists(temp_path):
                    path = temp_path
                    break
            if len(path) == 0:
                continue
            dependencies.append(path)

            # Push to stack
            stack.append(path)

    # Copy dependencies
    for path in dependencies:
        print_verbose(f"Copy {os.path.normpath(os.path.abspath(path))}")
        shutil.copy2(path, libdir)

    return 0


def copy_files_with_extensions(src_directory: str, dst_directory: str, extensions: list[str]):
    # Create the destination directory if it does not exist
    if not os.path.exists(dst_directory):
        os.makedirs(dst_directory)

    # Walk through the source directory
    for foldername, _, filenames in os.walk(src_directory):
        # Create corresponding folders in the destination directory
        relative_folder = os.path.relpath(foldername, src_directory)
        dst_folder = os.path.join(dst_directory, relative_folder)
        if not os.path.exists(dst_folder):
            os.makedirs(dst_folder)

        for filename in filenames:
            # Check file extension
            if len(extensions) == 0 or any(filename.lower().endswith(ext.lower()) for ext in extensions):
                # Copy the file to the destination directory
                src_file = os.path.join(foldername, filename)
                dst_file = os.path.join(dst_folder, filename)
                shutil.copy2(src_file, dst_file)


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

    # Set global variables
    if args.verbose:
        Global.verbose = True

    if args.prefix == "":
        Global.prefix = os.curdir
    else:
        Global.prefix = args.prefix

    # Run windeployqt
    code = deploy_qt_binaries(os.path.dirname(args.qmake),
                              Global.prefix + "/bin",
                              Global.prefix + "/lib/Qt/plugins",
                              args.files)
    if code != 0:
        sys.exit(code)

    # Deploy other libraries
    code = deploy_3rdparty_libraries(args.petool,
                                     Global.prefix + "/bin",
                                     args.files,
                                     args.dirs)
    if code != 0:
        sys.exit(code)

    # Deploy `qtmediate` and `ChorusKit` plugins and resources
    def find_and_copy(src_rel_path: str, dest_rel_path: str, extensions: list[str] = []) -> bool:
        for dir in args.dirs:
            temp_path = dir + "/" + src_rel_path
            if os.path.isdir(temp_path):
                print_verbose(
                    f"Copy directory {os.path.normpath(os.path.abspath(temp_path))}")
                copy_files_with_extensions(
                    temp_path, Global.prefix + "/" + dest_rel_path, extensions)
                return True
        return False

    # Searching path is the `bin` directory, but install prefix is `root` directory,
    # we need to search the parent directory of each searching path for `lib` and `share`
    # directory.
    find_and_copy("../lib/qtmediate/plugins", "lib/qtmediate/plugins", ["dll"])

    # VCPKG has a `debug` directory containing debug version of the packages, we need 
    # to search the upper directory if the `share` directory is not found.
    if not find_and_copy("../share/qtmediate/translations", "share/qtmediate/translations"):
        find_and_copy("../../share/qtmediate/translations",
                      "share/qtmediate/translations")
    if not find_and_copy("../share/ChorusKit/translations", "share/ChorusKit/translations"):
        find_and_copy("../../share/ChorusKit/translations",
                      "share/ChorusKit/translations")


if __name__ == "__main__":
    main()
