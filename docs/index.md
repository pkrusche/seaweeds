The Seaweed Code
================

This file contains an outline of all the documentation available in this repository.

This code has three main applications:

1. Alignment Plot Computation
   
   Alignment plots give all window-pair alignment scores which are above a certain threshold
   (which can be chosen arbitrarily in advance).

2. Markov Sequence Model learning

   This is useful for generating background models for empirical motif scoring.

3. Empirical Motif Scoring

   The idea is to compute Motif scores and evaluate their significance using 
   a statistical background model.

   To compute p-values from motif scores, we compute score histograms for large sets of sequences.
