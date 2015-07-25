;***************************************************************************
;*   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
;*   peter.dcs.warwick.ac.uk                                               *
;***************************************************************************
BITS 64
; C function macro which deals with the leading underscore issue by
; declaring multiple labels
%macro public_c_symbol 1
GLOBAL %1,_%1
%1:
_%1:
%endmacro


SECTION .data

GLOBAL mmx_one_b 

mmx_one_b: dq 0101010101010101h

SECTION .text

public_c_symbol do_emms
  emms
  RET


; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_8_mmx
; PROC data1:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains len  

  mov rcx, rsi
  mov rsi, rdi
  movq mm1, [mmx_one_b]       ; mm1 =     1     1     1 ... 
.loop:
  movq mm0, [rsi]
  paddusb mm0, mm1
  movq [rsi], mm0
  add rsi, 8
  dec rcx
  jnz .loop
.done:
  RET

; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_16_mmx
; PROC data1:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains len  
  
  mov rcx, rsi
  mov rsi, rdi
  pcmpeqw  mm1, mm1        ; mm1 = 0ffff 0ffff 0ffff ...
  psrlw    mm1, 15         ; mm1 =     1     1     1 ... 
.loop:
  movq mm0, [rsi]
  paddusw mm0, mm1
  movq [rsi], mm0
  add rsi, 8
  dec rcx
  jnz .loop
.done:
  RET

; generate an 8 bit match mask from two 8 bit per character strings.
; we assume the strings are padded to 64 bits.
; zero matches all.
public_c_symbol generate_match_mask_c8_v8_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  ; rdi contains string1
  ; rsi contains string2
  ; rdx contains mask_out
  ; rcx contains len

  xor rax, rax  
.loop:
  movq mm0, [rsi+8*rax]
  movq mm1, [rdi+8*rax]
  
  pcmpeqb mm0, mm1
  
  movq [rdx+8*rax], mm0
  
  inc rax  
  dec rcx
  jnz .loop
  RET

; generate an 16 bit match mask from two 16 bit per character strings.
; we assume the strings are padded to 64 bits.
; zero matches all.
public_c_symbol generate_match_mask_c16_v16_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  ; rdi contains string1
  ; rsi contains string2
  ; rdx contains mask_out
  ; rcx contains len

  xor rax, rax  
.loop:
  movq mm0, [rsi+8*rax]
  movq mm1, [rdi+8*rax]
  
  pcmpeqw mm0, mm1
  
  movq [rdx+8*rax], mm0
  
  inc rax  
  dec rcx
  jnz .loop
  RET

; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_8_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains data2
  ; rdx contains len

  xor rcx, rcx 

  ; mm5 == 0ffffff....ff
  pcmpeqd mm5, mm5
  movq	mm6, mm5
  ; mm6 = 00010001....
  movq  mm6, [mmx_one_b]
  
.loop:
  movq mm0, [rsi+8*rcx]
  movq mm1, [rdi+8*rcx]
  
  movq mm2, mm1
  psubusb mm2, mm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddb mm2, mm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movq mm3, mm2
  pandn mm3, mm5	; mm3 = !mm3
  paddb mm3, mm6	; mm3 = -mm2 (two's complement), i.e. 
			; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddb mm3, mm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddb mm3, mm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movq [rdi+8*rcx], mm3 ; the smaller values
  movq [rsi+8*rcx], mm2 ; the larger values

  inc rcx
  dec rdx
  jnz .loop
  
  RET


; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_16_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains data2
  ; rdx contains len

  xor rcx, rcx 

  ; mm5 == 0ffffff....ff
  pcmpeqd mm5, mm5
  movq	mm6, mm5
  ; mm6 = 00010001....
  psrlw mm6, 15
  
.loop:
  movq mm0, [rsi+8*rcx]
  movq mm1, [rdi+8*rcx]
  
  movq mm2, mm1
  psubusw mm2, mm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddw mm2, mm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movq mm3, mm2
  pandn mm3, mm5	; mm3 = !mm3
  paddw mm3, mm6	; mm3 = -mm2 (two's complement), i.e. 
			; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddw mm3, mm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddw mm3, mm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movq [rdi+8*rcx], mm3 ; the smaller values
  movq [rsi+8*rcx], mm2 ; the larger values

  inc rcx
  dec rdx
  jnz .loop
  
  RET

; replace bits in data with bits in data2 if corresponding bit in mask is set
public_c_symbol replace_if_mmx
;PROC data:PTR, data2:PTR, mask:PTR, len:DWORD
  ; rdi contains data
  ; rsi contains data2
  ; rdx contains mask
  ; rcx contains len

  xor rax, rax
  ; mm5 == 0ffffff....ff
  pcmpeqd mm5, mm5

.loop:
  movq mm0, [rdi+8*rax]		; load first value
  movq mm1, [rsi+8*rax]		; load second value
  movq mm2, [rdx+8*rax]		; load mask
  
  pand mm1, mm2			; exchange if bits in mask are set
  pandn mm2, mm5
  pand mm0, mm2
  por mm0, mm1  
  
  movq [rdi+8*rax], mm0
  
  inc rax
  dec rcx
  jnz .loop
  RET
