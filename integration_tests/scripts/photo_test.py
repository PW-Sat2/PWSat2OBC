# run from PyConsole with %run -i

import sys

download_folder = sys.argv[1]

print 'Taking photos'
system.obc._command('photo')

files = system.obc.list_files('/')

files = filter(lambda x: x[0] == 'p', files)

print 'Downloading photos'
for f in files:
    print f
    data = system.obc.read_file('/' + f)
    with open(download_folder + '/' + f, 'wb') as b:
        b.write(data)

local_files = map(lambda f: download_folder + '/' + f, files)
system.camera.decode_files(local_files)