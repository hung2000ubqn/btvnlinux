#  If the shared object is in a non standard location, we
#  need to tell where it is via the LD_LIBRARY_PATH
#  environment variable
#
# ./use-shared-object
#    ./use-shared-object: error while loading shared libraries: libtq84.so: cannot open shared object file: No such file or directory

LD_LIBRARY_PATH=$(pwd)/bin/shared bin/use-shared-library

... 

https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index
