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
echo "cat ${insql} | valgrind --leak-check=full -v ./${exefile} -z 'Asia/Tokyo' $*"
echo "!>"
echo "DEBUG <!"
echo "gdb -q -ex 'set args < ${insql}' -ex 'run' -ex 'where' -ex 'kill' -ex 'quit' ./${exefile} -z 'Asia/Tokyo' $*"
echo "!>"
echo

#if [ "$1" != '' ]
#then
#  cat ${insql} | valgrind --leak-check=full -v ./${exefile} > ${outsql}
#else
#  cat ${insql} | ./${exefile} "$@" > ${outsql}
#fi

cat ${insql} | ./${exefile} -z 'Asia/Tokyo' "$@" > ${outsql}
rc=${PIPESTATUS[1]}

if [[ $rc = 0 ]]
then
  cat << EOS

# === output ===>>>
$(cat ${outsql})
# === output ===<<<
EOS
fi

echo
echo "# done. [$rc]"

exit $rc

