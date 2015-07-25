The Seaweeds Alignment Plot Code
================================

This is a set of tools to compute alignment plots for pairs 
of sequences and score motifs.

I. Requirements and Building
============================

1. You will need a copy of the most recent version of [Bsponmpi](https://github.com/pkrusche/bsponmpi). 
2. Bsponmpi requires [Intel's Threading Building Blocks](http://threadingbuildingblocks.org/).
3. [Boost](www.boost.org) v. 1.48 or greater is required. 
4. You can (and should) use [Agner Fog's vectorclass](http://agner.org/optimize/#vectorclass). This will substantially speed up the code.
5. The [yasm assembler](http://yasm.tortall.net/).
6. You need [SCons](http://www.scons.org/) to build the code.

*Included Dependencies*:

Included dependency libraries are in src/external.

1. JsonCPP: (c) Baptiste Lepilleur, MIT License, 
   see `src/external/jsoncpp-src-0.5.0/LICENSE`
2. UnitTest++: 
   Copyright (c) 2006 Noel Llopis and Charles Nicholson, 
   MIT License, see `src/external/UnitTest++/COPYING`


a. General Remarks
------------------

The code has been tested on Windows, MacOS/X and Linux. 

The easiest way to get it going is to use Ubuntu Linux (either in a Virtual Machine, or an actual installation), 
and to install Boost and SCons via `apt`. 

On MacOS/X, you can use [Macports](http://www.macports.org/) to get the dependencies.

On Windows, the main obstacle is compiling Boost (I found 
[this entry on StackOverflow](http://stackoverflow.com/questions/2322255/64-bit-version-of-boost-for-64-bit-windows) 
very helpful). To use MPI on Windows, you can download the Microsoft Compute Cluster pack, and point the 
configuration file to that.

b. How to Build the Multi-threaded (non-MPI) version
----------------------------------------------------

The following instructions will build the code using a non-MPI version of BSPonMPI (you will need to compile 
BSPonMPI using the `sequential` flag first). 

After cloning the repository, you can start the build process by running:

```
scons -Q mode=release sequential=1
```

This will output the name of the configuration file which is used:

```
$ scons -Q mode=release sequential=1
Using options from opts_Darwin_i386.py
...
```

You will need to modify this configuration file to point it to the various libraries and dependencies.
In the case above, we would change `opts_Darwin_i386.py`.

Here are the default settings:

```
# cflags and lflags to point to tbb, bsponmpi and boost
additional_cflags = '-I../tbb40_20120201oss/include -I../bsponmpi/include -I/opt/local/include -O3 -g -msse2 -msse3 -msse4 -Wno-parentheses-equality -Wno-switch '
additional_lflags = '-L../bsponmpi/lib -L/opt/local/lib'
replacement_CC = '/opt/local/bin/clang-mp-3.2'
replacement_CXX = '/opt/local/bin/clang++-mp-3.2'
replacement_LINK = '/opt/local/bin/clang++-mp-3.2'

use_yasm=1

# These are optional, but highly recommended. See below.
asmlibdir = '/Users/peterkrusche/Documents/Code/Docs/Agner/asmlib'
veclibdir = '/Users/peterkrusche/Documents/Code/Docs/Agner/vectorclass'
```

c. How to Build the Multi-threaded MPI version
-----------------------------------------------

Like above, but run

```
scons -Q mode=release sequential=0
```

and provide valid settings for compiling MPI code.

One easy way to do so is to use `mpicc` as a replacement compiler:

```
replacement_CC = 'mpicc'
replacement_CXX = 'mpic++'
replacement_LINK = 'mpic++'
```

d. Testing
----------

There is an extensive suite of unit tests you can run. To run all of them, run (for the non-MPI version...)

```
scons -Q mode=release sequential=1 tests=1
```

This will generate binaries in `bin/unit_tests` and `bin/performance_tests`.

On my system I can then run

```
$ bin/unit_tests/seaweedtests
```

... which runs tests and should end with the following output:

```
...
Testing Test_Seaweeds_WindowlocalLCS_Allmatch 
Testing Test_Seaweeds_WindowlocalLCS_Allmismatch 
Testing Test_Windowlocal_LCS 
Testing Test_Add_with_carry_bittest 
Testing Test_Intvector_StreamIO 
Testing Test_Intvector_Shift 
Success: 322 tests passed.
Test time: 1.06 seconds.
```

There are a few lengthier tests, which are disabled by default. You can run these using 

```
$ bin/unit_tests/seaweedtests lengthy
```

You can also run a specific test only like this:

```
$ bin/unit_tests/seaweedtests only Test_Windowlocal_LCS
```

(see also `tests/unit_Main.cpp`).

II. Code Applications
=====================

The current version allows to perform three types of computation for DNA sequences:

1. Alignment Plot Computation
   
   Alignment plots give all window-pair alignment scores which are above a certain threshold
   (which can be chosen arbitrarily in advance).

2. Markov Sequence Model learning

   This is useful for generating background models for empirical motif scoring.

3. Empirical Motif Scoring

   The idea is to compute Motif scores and evaluate their significance using 
   a statistical background model.

   To compute p-values from motif scores, we compute score histograms for large sets of sequences.

III. More Documentation
=======================

More documentation can be found in the `docs` subfolder.
