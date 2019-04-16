#!/bin/sh

unalias -a

cd $(dirname $(readlink -f "$0"))

clear

insrc='input.sql.txt'
exefile='mysqldump2spanner.exe'

#cc='/usr/bin/clang-6.0'
cc='cc'

echo "*** make ***"
make || exit
echo

echo "**** run ***"

echo "LEAK CHECK <!"
echo "cat ${insrc} | valgrind --leak-check=full -v ./${exefile}"
echo "!>"
echo "DEBUG <!"
echo "gdb -q -ex 'set args < ${insrc}' -ex 'run' -ex 'where' -ex 'kill' -ex 'quit' ./${exefile}"
echo "!>"
echo

#if [ "$1" != '' ]
#then
#  cat ${insrc} | valgrind --leak-check=full -v ./${exefile} > result.out
#else
#  cat ${insrc} | ./${exefile} "$@" > result.out
#fi

cat ${insrc} | ./${exefile} "$@" 2>&1 | expand -t 2 > result.out
cat result.out

if [ -f copy.sh ]
then
  sh copy.sh
  [ $? -ne 0 ] && exit
fi

echo
echo "# done."

exit 0

