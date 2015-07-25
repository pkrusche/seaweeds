
MODE=debug
TARGET=bin/unit_tests/seaweedtests_darwin_default_debug

all: 
	scons -Q mode=${MODE} sequential=1 configure=1 -j 4 ${TARGET}

clean:
	scons -Q -c mode=${MODE} sequential=1
