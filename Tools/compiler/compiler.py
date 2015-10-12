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
make_file = "Makefile"
build_dir_prefix = "build"
cross_compile_file_name = "cross-compile.cmake"
last_compiled_project = "default"
is_cmake = False

def compile_cmake_repo(repo, cross_compile_file):
    print "Compiling CMakeLists.txt:", repo
    print "System:", os.name 
    if (os.name is "posix"):
        system = "Unix Makefiles"
    else:
        print "This system is not supported. Fix it!"
        sys.exit(-1)
    #cmd = ["cmake", "=".join(["-DROOT_DIR:STRING",pwd]), "=".join(["-DCMAKE_TOOLCHAIN_FILE",cross_compile_file]), "-G", system]
    cmd = ["cmake", "=".join(["-DROOT_DIR:STRING",pwd]), "=".join(["-DCMAKE_TOOLCHAIN_FILE",cross_compile_file]), "-G", system, "-DBUILD_SHARED_LIBS=OFF", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-static"]
    proc = subprocess.Popen(cmd)
    proc.wait()
    #out = check_output(cmd)
    print "Compiling Makefile:", repo
    proc = subprocess.Popen(["make"])
    proc.wait()
    #out = check_output(["make"])


def compile_make_repo(project_dir, build_dir):
    makefile = "/".join([project_dir, "Makefile"])
    print "Compiling Makefile:", makefile
    print "Build dir:", build_dir
    proc = subprocess.Popen(["make", "-f", makefile, "=".join(["ROOT_PATH", pwd]), "=".join(["BUILD_PATH", build_dir])])
    proc.wait()

def remove_hidden(objects):
    length = len(objects)
    while (length > 0):
        obj = objects[length - 1]
        if (obj[0] == '.'):
            del objects[length - 1]
        length -= 1

def remove_not_directory(objects, path):
    length = len(objects)
    while (length > 0):
        obj = objects[length - 1]
        obj_path = os.path.join(path, obj)
        if (os.path.isdir(obj_path) == False):
            del objects[length - 1]
        length -= 1

def install():
    bin_path = os.path.join(os.environ['HOME'], "bin")
    if (bin_path is None):
        print "There isn't HOME environment variable, so You are trying to install it on other OS than Linux."
        print "Please install it manually and add support."
        return 0
    print "bin_path:", bin_path
    if (os.path.isdir(bin_path) == False):
        print "There isn't such directory like {0} or it's not directory. Fix it! " .format(bin_path)
        return -1
    script_path = os.path.realpath(__file__)
    dest_script_file = os.path.join(bin_path, __file__)
    print "Coping script {0} to bin directory {1}" .format(__file__, bin_path)
    copyfile(script_path, dest_script_file)
    os.chmod(dest_script_file, 0777)
    return 0

def verify_required():
    if (is_cmake == True):
        executables="cmake"
    else:
        executables="make"
    path = find_executable(executables)
    if (path is None):
        print "There wasn't any '{0}' installation found on this system." .format(executables)
        print "Please install: '{0}' and run this script again." .format(executables)
        sys.exit(0)

parser = argparse.ArgumentParser()
parser.add_argument('--install', '-i', action='store_true',
                    help='install script in $(HOME)/bin directory')

args = parser.parse_args()

if (args.install):
    ret = install()
    sys.exit(ret)



default_project = None

try:
    projects = os.listdir(projects_dir)
    length = len(projects)
    if (length == 0):
        print "There aren't any projects at : ", projects_dir
        print "Update Your git resources!"
        sys.exit(-1)

# remove files and hidden directories
    remove_hidden(projects)
    remove_not_directory(projects, projects_dir)
    print "Please, choose project:"
    project_idx = 1
    for proj in projects:
        path = os.path.join(projects_dir, proj, last_compiled_project)
        if (os.path.isfile(path)):
            print "*{0}) {1}" .format(project_idx, proj)
            default_project = proj
            os.remove(path)
        else:
            print "{0}) {1}" .format(project_idx, proj)
        project_idx += 1


except OSError as error:
    print "Error occures: {0} {1}" .format(error.errno, error.strerror)
    print "To compile system there must be directory: ", projects_dir
    print "Update Your git resources!"
    sys.exit(-1)

proj_cnt = len(projects)

while (True):
    try:
        sign = raw_input()
        if (sign is '' and default_project is not None):
            # take default if exist
            project = default_project
        else:
            res = int(sign.strip())
            if (res >= 1 and res <= proj_cnt):
                project = projects[res - 1]
        path = os.path.join(projects_dir, project, last_compiled_project)
        os.mknod(path, 0777)
        break
    except ValueError as error:
        continue

#Create paths to project files
project_dir_path = os.path.join(projects_dir, project)
project_file_path = os.path.join(project_dir_path, cmake_file)
build_dir = os.path.join(pwd, "-".join([build_dir_prefix, project]))
if (os.path.isfile(project_file_path) == True):
    cross_compile_file_path = os.path.join(project_dir_path, cross_compile_file_name)
    dest_file = os.path.join(build_dir, cmake_file)
    is_cmake = True
else:
    project_file_path = os.path.join(project_dir_path, make_file)
    if (os.path.isfile(project_file_path) == False):
        print "Project {0} is invalid." .format(project)


print "Project:", project
print "Project path:", project_dir_path
print "Build directory:", build_dir

# Create build dir
if (os.path.isdir(build_dir) == False):
    os.mkdir(build_dir)

verify_required()

try:
    if (is_cmake == True):
        print "Coping project {0} to build directory {1}" .format(project, build_dir)
        copyfile(project_file_path, dest_file)

        os.chdir(build_dir)
        compile_cmake_repo(dest_file, cross_compile_file_path)
    else:
        compile_make_repo(project_dir_path, build_dir)
except CalledProcessError as error:
    print "Command {0} failed." .format(error.cmd)
    print "Error occures: {0} with value {1} " .format(error.output, error.returncode)
    print "Update Your git resources!"
    sys.exit(-1)
