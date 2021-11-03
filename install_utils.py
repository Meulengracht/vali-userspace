import os, sys
import glob
import re
import zipfile
import subprocess
import shutil
import argparse

def fatal(error_msg):
    print(error_msg)
    sys.exit(-1)

def empty_folder_if_exist(folder_path):
    print("Unpack to " + folder_path)
    if os.path.exists(folder_path):
        print("Path exists already, emptying it")
        for the_file in os.listdir(folder_path):
            file_path = os.path.join(folder_path, the_file)
            try:
                if os.path.isfile(file_path):
                    os.unlink(file_path)
                elif os.path.isdir(file_path):
                    shutil.rmtree(file_path)
            except Exception as e:
                print(e)

def copy_files(source_path, destination_path, pattern, overwrite=False):
    """
    Recursive copies files from source  to destination directory.
    :param pattern: the file pattern to copy
    :param source_path: source directory
    :param destination_path: destination directory
    :param overwrite if True all files will be overwritten otherwise skip if file exist
    :return: count of copied files
    """
    files_count = 0
    if not os.path.exists(destination_path):
        os.mkdir(destination_path)
    items = glob.glob(source_path + '/**/' + pattern, recursive=True)
    for item in items:
        if os.path.isdir(item):
            path = os.path.join(destination_path, item.split('/')[-1])
            files_count += copy_files(source_path=item, destination_path=path, pattern=pattern,
                                                overwrite=overwrite)
        else:
            file = os.path.join(destination_path, item.split('/')[-1])
            if not os.path.exists(file) or overwrite:
                shutil.copyfile(item, file)
                files_count += 1
    return files_count


def cp_main(args):
    if args.source and args.dest:
        #empty_folder_if_exist(args.dest)
        copy_files(args.source, args.dest, args.pattern, overwrite=True)
        sys.exit(0)
    else:
        print("utils: missing arg --source or --dest")

  
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Installation utilities for building and releasing userspace apps for Vali.')
    parser.add_argument('--cp', default=False, action='store_true',
                        help='invoke script in copy-file mode, use --source, --dest and --pattern')

    cpArguments = parser.add_argument_group('cp')
    cpArguments.add_argument('--source', default=None, help='source directory for cp')
    cpArguments.add_argument('--dest', default=None, help='destination directory for cp')
    cpArguments.add_argument('--pattern', default="*", help='the file pattern to use for cp')

    args = parser.parse_args()
    if args.cp:
        cp_main(args)
    else:
        fatal("No mode specified")
