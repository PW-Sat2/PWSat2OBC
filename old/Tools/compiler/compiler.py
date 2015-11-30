#!/usr/bin/python
import os
import sys
import subprocess
import string
import argparse
from subprocess import check_output
from subprocess import CalledProcessError
from shutil import copyfile
from distutils.spawn import find_executable

if (__file__ is None):
    __file__ = sys.argv[0]

pwd = os.getcwd()
projects_dir = os.path.join(pwd, "Projects")
cmake_file = "CMakeLists.txt"
build_dir_prefix = "build"
cross_compile_file_name = "cross-compile.cmake"
last_compiled_project = "default"

def compile_cmake_repo(repo, cross_compile_file):
    print "Compiling CMakeLists.txt:", repo
    print "System:", os.name
    if (os.name is "posix"):
        system = "Unix Makefiles"
    else:
        system = "Unix Makefiles"
        print "This system is not supported. Fix it!"
        #sys.exit(-1)
    #cmd = ["cmake", "=".join(["-DROOT_DIR:STRING",pwd]), "=".join(["-DCMAKE_TOOLCHAIN_FILE",cross_compile_file]), "-G", system]
    cmd = ["cmake", "=".join(["-DROOT_DIR:STRING",pwd]), "=".join(["-DCMAKE_TOOLCHAIN_FILE",cross_compile_file]), "-G", system, "-DBUILD_SHARED_LIBS=OFF", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-static"]
    proc = subprocess.Popen(cmd)
    proc.wait()
    #out = check_output(cmd)
    print "Compiling Makefile:", repo
    proc = subprocess.Popen(["make"])
    proc.wait()
    #out = check_output(["make"])


def remove_hidden(objects):
    cnt = len(objects)
    while (cnt > 0):
        obj = objects[cnt - 1]
        if (obj[0] == '.'):
            del objects[cnt - 1]
        cnt -= 1

def remove_not_directory(objects, path):
    cnt = len(objects)
    while (cnt > 0):
        obj = objects[cnt - 1]
        obj_path = os.path.join(path, obj)
        if (os.path.isdir(obj_path) == False):
            del objects[cnt - 1]
        cnt -= 1

def try_compile_project(project):
    # Create paths to project files : CMakeLists.txt and cross-compile.cmake
    project_dir_path = os.path.join(projects_dir, project)
    cmake_file_path = os.path.join(project_dir_path, cmake_file)
    # Create path to build directory
    build_dir = os.path.join(pwd, "-".join([build_dir_prefix, project]))
    # Check if CMakeFile.txt exists
    if (os.path.isfile(cmake_file_path) == False):
        print "Project {0} is invalid." .format(project)
        print "There is no CmakeLists.txt file."

    cross_compile_file_path = os.path.join(project_dir_path, cross_compile_file_name)
    dest_file = os.path.join(build_dir, cmake_file)

    print "Project:", project
    print "Project path:", project_dir_path
    print "Build directory:", build_dir

    # Create build dir
    if (os.path.isdir(build_dir) == False):
        os.mkdir(build_dir)

    try:
        # coping CMakeFile.txt to build directory
        copyfile(cmake_file_path, dest_file)

        os.chdir(build_dir)
        compile_cmake_repo(dest_file, cross_compile_file_path)
    except CalledProcessError as error:
        print "Command {0} failed." .format(error.cmd)
        print "Error occures: {0} with value {1} " .format(error.output, error.returncode)
        print "Update Your git resources!"
        sys.exit(-1)

default_project = None

try:
# At first we search for projects in Project directory
    projects = os.listdir(projects_dir)
    projects_cnt = len(projects)
    if (projects_cnt == 0):
        print "There aren't any projects at : ", projects_dir
        print "Update Your git resources!"
        sys.exit(-1)

# remove files and hidden directories
    remove_hidden(projects)
    remove_not_directory(projects, projects_dir)
# after removing useless files and directories there maid be empty array
    projects_cnt = len(projects)
    if (projects_cnt == 0):
        print "There aren't any projects at : ", projects_dir
        print "Update Your git resources!"
        sys.exit(-1)
    elif (projects_cnt == 1):
        try_compile_project(projects[0])
        sys.exit(0)

# print projects list
    print "Please, choose project:"
    project_idx = 1
    for project_tmp in projects:
        # last compiled project is selected by default (*)
        last_compiled_project_path = os.path.join(projects_dir, project_tmp, last_compiled_project)
        if (os.path.isfile(last_compiled_project_path)):
            print "*{0}) {1}" .format(project_idx, project_tmp)
            default_project = project_tmp
            os.remove(last_compiled_project_path)
        else:
            print "{0}) {1}" .format(project_idx, project_tmp)
        project_idx += 1

except OSError as error:
    print "Error occures: {0} {1}" .format(error.errno, error.strerror)
    print "To compile system there must be directory: ", projects_dir
    print "Update Your git resources!"
    sys.exit(-1)

while (True):
    try:
        sign = raw_input()
        if (sign is '' and default_project is not None):
            # compile default project
            project = default_project
        else:
            res = int(sign.strip())
            if (res >= 1 and res <= projects_cnt):
                project = projects[res - 1]
        # create default file in project that was last compiled
        selected_project_path = os.path.join(projects_dir, project, last_compiled_project)
        os.mknod(selected_project_path, 0777)
        break
    except ValueError as error:
        continue

try_compile_project(project)
