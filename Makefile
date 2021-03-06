
# := は即時評価
# =  は遅延評価

.PHONY: all clean c r v ast

#.SUFFIXES:
.SUFFIXES: .cc, .hh, .hpp, .exe

# --------------------------->>
#OPTIONS := -std=c++11 -Wall -Wextra -g -O0 -pipe
OPTIONS := -std=c++11 -Wall -Wextra -g -O0 -save-temps

CXX := g++
#CXX := g++-8
#CXX := clang++-6.0

CXXFLAGS := $(OPTIONS) -include std.hpp -DTEXTDOMAINDIR="$(shell pwd)/locale"
#CXXFLAGS := $(OPTIONS) -include-pch std.hpp.pch
# ---------------------------<<

DEPENDS_0 = Makefile

DEPENDS  = $(DEPENDS_0)
DEPENDS += std.hpp comm.hpp driver.hpp ddl.hpp dml.hpp

TARGET := mysqldump2spanner.exe

all: $(TARGET)

##astest: gcc -std=c99 -O0 -S -o- a.c
##.cpp.exe:
##	$(CXX) $(CXXFLAGS) -o $@ $<

#std.hpp.pch: std.hpp $(DEPENDS_0)
#	$(CXX) $(OPTIONS) -x c++-header -o $@ $<

parser.cc: parser.yy ddl.hpp $(DEPENDS)
	bison --report=all --report-file=bison-report.out --output=$@ $<

scanner.cc: scanner.ll scanner.hpp $(DEPENDS)
	flex --outfile=$@ $<

ddl.o: ddl.cpp $(DEPENDS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o: main.cpp $(DEPENDS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): parser.o scanner.o ddl.o main.o
	@echo
	@echo "*** link ***"
	$(CXX) $(CXXFLAGS) -o $@ $^
	msgfmt -o locale/ja/LC_MESSAGES/mysqldump2spanner.mo mysqldump2spanner.po
	@echo

print:
	make -p
	gcc -dM -E - < /dev/null

clean:
	clear
	rm -f *.cc *.hh *.o *.exe *.out *.pch ?.txt
#	cp a.cc.tmp a.cc
#	cp a.c.tmp a.c

#ast:
#	$(CXX) -x c -std=c99 -Xclang -ast-dump -fsyntax-only -fno-color-diagnostics program.c.txt

v:
	bison --version
	flex --version

c:
	clear
	make

r:
	make clean
	make

