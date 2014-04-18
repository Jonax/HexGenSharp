# This script uses the clang static analyser and cppcheck to perform
# some static analysis on the code base in order to catch errors.

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


echo -e "\n\nCPPCHECK\n=============================="
cppcheck --std=c99 --std=posix -f -j4 src 2> cppcheck.txt
cat cppcheck.txt
