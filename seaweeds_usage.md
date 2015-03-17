# Introduction #
The main part of the seaweed code consists of various implementations of alignment plot and LCS computation, some unit tests to ensure correct results and tools allow the user to obtain running time measurements. The individual tools will be described in the following sections, followed by instructions for compiling the code and a brief outline of the source tree.

# The alignment plot tools #
The three main executables for computing alignment plots are _Windowalignment_ and _Postprocesswindows_.

## Windowalignment ##
This program takes two sequence files as its input, the first sequence file must contain the parameters for the comparison, which are the first and second step sizes (only window pairs with starting positions aligning at these step sizes are compared), the window length, and the minimum alignment score threshold.

The following example shows such a first input sequence file. In this file, the first step size is 5, the second step size is~1, and the window length is~100.
```
5	1	100	55	
TAAGCTAGGGGCCAGGAC...
```

The command line usage on Linux is as follows.

```
$ ./WindowAlignment [firstsequencefile] [secondsequencefile]
```

Example versions of sequence files can be found in the subfolders of the "benchmark" directory.

The output consists of two profile files which give the score for
the highest-scoring window in each row and column, and a file named
\texttt{results.txt} which contains locations and scores for all windows scoring
above the threshold.

Moreover, the locations of the output files and profiles can be specified separately:
```
$ ./WindowAlignment [firstsequencefile] [secondsequencefile] \
         [resultfilename] [firstprofilename] [secondprofilename] 
         {options} 
```

**Windowalignment command line options**

|**Option**|**Explanation**|
|:---------|:--------------|
| -c | enable checkpointing (resuming if checkpoint file exists for the job)|
| -m lcs|blcs|seaweeds|scores|scoresoverlap | Choose method to use. **LCS:** This computes the scores by computing the LCS separately for each window pair.  **BLCS:** This uses bit-parallel LCS computation to obtain the window scores, again separately for each window pair. **Seaweeds:** This uses the seaweed algorithm from Section~\ref{sec:seawinwin}, and counts scores in a sliding window using a queue. **Scores:** This uses the seaweed algorithm but computes implicit highest-score matrices to allow faster window score queries.  **Scoresoverlap:** This method uses the seaweed algorithm and uses strip overlap to speed up the computation.|
| -os | specify overlap size for scoresoverlap method |
|-legalchars_`[`a`|`b`]`_| Specify input alphabet translation. The default is to match characters ABGCTz normally, and mismatch N and x by setting legalchars\_a = ABGCTNxz, and legalchars\_b = ABGCTxNz|

## Postprocesswindows ##
This is a tool to filter the output of WindowAlignment and reduce the
window count. It also sorts the output files such that outputs from Alignment
and WindowAlignment can be compared using diff.

Command line usage:
```
$ PostprocessWindows [resultfilename] -m [number] 
```
The number specified after the -m option gives the maximum number of
windows which are exported. If more windows are found, only the ones with the
highest scores are reported.