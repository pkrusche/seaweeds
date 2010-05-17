;***************************************************************************
;*   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
;*   peter.dcs.warwick.ac.uk                                               *
;***************************************************************************

; C function macro which deals with the leading underscore issue by
; declaring multiple labels
%macro public_c_symbol 1
GLOBAL %1,_%1
%1:
_%1:
%endmacro


SECTION .data

SECTION .text

public_c_symbol do_emms
  RET
