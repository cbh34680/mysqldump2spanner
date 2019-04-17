#!/bin/sh

unalias -a

cd $(dirname $(readlink -f "$0"))

clear

insql='input.sql.txt'
outsql='output.sql.txt'
exefile='mysqldump2spanner.exe'

#cc='/usr/bin/clang-6.0'
cc='cc'

echo "*** make ***"
make || exit
echo

echo "**** run ***"

echo "LEAK CHECK <!"
echo "cat ${insql} | valgrind --leak-check=full -v ./${exefile}"
echo "!>"
echo "DEBUG <!"
echo "gdb -q -ex 'set args < ${insql}' -ex 'run' -ex 'where' -ex 'kill' -ex 'quit' ./${exefile}"
echo "!>"
echo

#if [ "$1" != '' ]
#then
#  cat ${insql} | valgrind --leak-check=full -v ./${exefile} > ${outsql}
#else
#  cat ${insql} | ./${exefile} "$@" > ${outsql}
#fi

cat ${insql} | ./${exefile} "$@" > ${outsql}
rc=${PIPESTATUS[1]}

echo
echo "# === output ===>>>"
cat ${outsql}
echo "# === output ===<<<"

if [ -f copy.sh ]
then
  sh copy.sh
  [ $? -ne 0 ] && exit
fi

echo
echo "# done. [$rc]"

exit $rc

