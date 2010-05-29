#!/bin/bash

./bin/seaweedtesting_reference_posix_gcc_release > out_e.txt 
./bin/seaweedtesting_posix_gcc_release > out.txt 
kdiff3 out.txt out_e.txt


