The cam-alias-checker is a set of tools for tracing the memory references and 
control flow of loops in programs, and generating dynamic data dependence 
graphs (DDG) from program runs. This includes an implementation of the SD3 
algorithm for compressing strided memory references and efficiently detecting 
aliases between compressed records (SD3: An Efficient Dynamic Data-Dependence 
Profiling Mechanism, Minjang Kim, Hyesoon Kim, and Chi-Keung Luk).

# Usage

To profile a program and generate a dynamic DDG you should take the following
steps:

1. Instrument the program with callbacks to the cam API and compile the 
instrumented program, linking with the cam library. The API is in 
libcam/src/cam.h. 

2. Run the instrumented binary, this will create the memory_accesses directory
with compressed records of instrumented memory accesses, and files 
loop_trace.txt.bz2 and call_trace.txt.bz2 which contain compressed traces of
the program control flow.

3. Run `cam -p` in the same directory to generate the dynamic DDG file
dependence_pairs.txt. This contains a list of instruction pairs which aliased
in different iterations of the loop.

## Repository contents

The repository contains the following components:

* libcam

  Contains all the code for generating the compressed traces and analysing
  them to produce the dynamic DDG.

* xanlib

  Data structure library, required for generating the loop trace, author:
  Simone Campanoni.

* libplatform

  Some basic data type definitions required by xanlib, author: Simone 
  Campanoni.
