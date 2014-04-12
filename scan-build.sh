# This script uses the clang static analyser to perform some analysis on the
# code base in order to catch errors.

# To be honest it doesn't catch much that clang doesn't warn about anyway, but
# it's good to have a working clang compile that fakes windows support using
# dummy headers from src/vendor/dummy/ in order to get the warnings that gcc
# misses. It also works as a minimal example of how to build the software.

echo -e "\n\nSCAN-BUILD::HEXGEN2014\n======================"

set -o xtrace
set -e

OPTS="-analyze-headers --status-bugs"

INPUTS=
INPUTS+="src/*.c "
INPUTS+="src/rng/*.c "
# INPUTS+="src/vendor/stb_image/*.c "
FLAGS=
FLAGS+="-m64 -std=c99 -g3 -O0 -iquote src "
FLAGS+="-D_XOPEN_SOURCE_EXTENDED -DHG14_LINUX -DHG14_64BIT "

# uses clang static analyser to detect errors
echo -e "\n\nSCAN-BUILD::HEXGEN2014-LINUX\n============================"
scan-build $OPTS clang $FLAGS -c $INPUTS
rm *.o

echo -e "\n\nSCAN-BUILD::HEXGEN2014-WINDOWS\n=============================="
scan-build $OPTS clang $FLAGS -c $INPUTS
rm *.o


