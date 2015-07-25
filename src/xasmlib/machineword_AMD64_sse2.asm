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

ALIGN 16
mmx_one_b: dq 0101010101010101h
           dq 0101010101010101h

SECTION .text

public_c_symbol do_emms
;  emms not necessary in SSE2 code
  RET


; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_8_mmx
; PROC data1:PTR, len:DWORD
  ; rcx contains data1
  ; rdx contains len  

  movdqa xmm1, [mmx_one_b]       ; xmm1 =     1     1     1 ... 
  
  mov rax, rdx
  shr rdx, 1
  jz .fin
    
.loop:
  movdqa xmm0, [rcx]
  paddusb xmm0, xmm1
  movdqa [rcx], xmm0
  add rcx, 16
  dec rdx
  jnz .loop
.fin:
  and al, 1
  jz .done

  movq xmm0, [rcx]
  paddusb xmm0, xmm1
  movq [rcx], xmm0

.done:
  RET

; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_16_mmx
; PROC data1:PTR, len:DWORD
  ; rcx contains data1
  ; rdx contains len  

  pcmpeqw  xmm1, xmm1        ; xmm1 = 0ffff 0ffff 0ffff ...
  psrlw    xmm1, 15         ; xmm1 =     1     1     1 ... 
  
  mov rax, rdx
  shr rdx, 1
  jz .fin
    
.loop:
  movdqa xmm0, [rcx]
  paddusw xmm0, xmm1
  movdqa [rcx], xmm0
  add rcx, 16
  dec rdx
  jnz .loop
.fin:
  and al, 1
  jz .done

  movq xmm0, [rcx]
  paddusw xmm0, xmm1
  movq [rcx], xmm0
.done:
  RET

; generate an 8 bit match mask from two 8 bit per character strings.
; we assume the strings are padded to 64 bits.
public_c_symbol generate_match_mask_c8_v8_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  ; rcx contains string1
  ; rdx contains string2
  ; r8 contains mask_out
  ; r9 contains len

  xor rax, rax  
  mov r10, r9
  shr r9, 1
  jz .fin
  
.loop:
  movdqa xmm0, [rcx+rax]
  movdqa xmm1, [rdx+rax]
  
  pcmpeqb xmm0, xmm1
  
  movdqa [r8+rax], xmm0
  
  add rax, 16
  dec r9
  jnz .loop
.fin:
  and r10b, 1
  jz .done
  
  movq xmm0, [rcx+rax]
  movq xmm1, [rdx+rax]
  
  pcmpeqb xmm0, xmm1
  
  movq [r8+rax], xmm0
.done:
  RET

; generate an 16 bit match mask from two 16 bit per character strings.
; we assume the strings are padded to 64 bits.
public_c_symbol generate_match_mask_c16_v16_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  ; rcx contains string1
  ; rdx contains string2
  ; r8 contains mask_out
  ; r9 contains len

  xor rax, rax  
  mov r10, r9
  shr r9, 1
  jz .fin
  
.loop:
  movdqa xmm0, [rcx+rax]
  movdqa xmm1, [rdx+rax]
  
  pcmpeqw xmm0, xmm1
  
  movdqa [r8+rax], xmm0
  
  add rax, 16
  dec r9
  jnz .loop
.fin:
  and r10b, 1
  jz .done
  
  movq xmm0, [rcx+rax]
  movq xmm1, [rdx+rax]
  
  pcmpeqw xmm0, xmm1
  
  movq [r8+rax], xmm0
.done:
  RET
  
; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_8_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  ; rcx contains data1
  ; rdx contains data2
  ; r8 contains len

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5
  ; xmm6 = 00010001....
  movdqa  xmm6, [mmx_one_b]

  xor r9, r9 
  mov rax, r8
  shr r8, 1
  jz .fin  
.loop:
  movdqa xmm0, [rdx+r9]
  movdqa xmm1, [rcx+r9]
  
  movdqa xmm2, xmm1
  psubusb xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddb xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddb xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddb xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddb xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movdqa [rcx+r9], xmm3 ; the smaller values
  movdqa [rdx+r9], xmm2 ; the larger values

  add r9, 16
  dec r8
  jnz .loop
.fin:
  and al, 1
  jz .done
  movq xmm0, [rdx+r9]
  movq xmm1, [rcx+r9]
  
  movdqa xmm2, xmm1
  psubusb xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddb xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddb xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddb xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddb xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movq [rcx+r9], xmm3 ; the smaller values
  movq [rdx+r9], xmm2 ; the larger values  
  
.done:
  RET

; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_16_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  ; rcx contains data1
  ; rdx contains data2
  ; r8 contains len

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5
  movdqa	xmm6, xmm5
  ; xmm6 = 00010001....
  psrlw xmm6, 15

  xor r9, r9
  mov rax, r8
  shr r8, 1
  jz .fin  
.loop:
  movdqa xmm0, [rdx+r9]
  movdqa xmm1, [rcx+r9]
  
  movdqa xmm2, xmm1
  psubusw xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddw xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddw xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddw xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddw xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movdqa [rcx+r9], xmm3 ; the smaller values
  movdqa [rdx+r9], xmm2 ; the larger values

  add r9, 16
  dec r8
  jnz .loop
.fin:
  and al, 1
  jz .done
  movq xmm0, [rdx+r9]
  movq xmm1, [rcx+r9]
  
  movdqa xmm2, xmm1
  psubusw xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddw xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddw xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddw xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddw xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movq [rcx+r9], xmm3 ; the smaller values
  movq [rdx+r9], xmm2 ; the larger values  
  
.done:
  RET

; replace bits in data with bits in data2 if corresponding bit in mask is set
public_c_symbol replace_if_mmx
;PROC data:PTR, data2:PTR, mask:PTR, len:DWORD
  ; rcx contains data
  ; rdx contains data2
  ; r8 contains mask
  ; r9 contains len

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5

  xor rax, rax
  mov r10, r9
  shr r9, 1
  jz .fin
  
.loop:
  movdqa xmm0, [rcx+rax]		; load first value
  movdqa xmm1, [rdx+rax]		; load second value
  movdqa xmm2, [r8+rax]			; load mask
  
  pand xmm1, xmm2			; exchange if bits in mask are set
  pandn xmm2, xmm5
  pand xmm0, xmm2
  por xmm0, xmm1  
  
  movdqa [rcx+rax], xmm0
  
  add rax, 16
  dec r9
  jnz .loop
.fin:
  and r10b, 1
  jz .done

  movq xmm0, [rcx+rax]		; load first value
  movq xmm1, [rdx+rax]		; load second value
  movq xmm2, [r8+rax]		; load mask
  
  pand xmm1, xmm2			; exchange if bits in mask are set
  pandn xmm2, xmm5
  pand xmm0, xmm2
  por xmm0, xmm1  
  
  movq [rcx+rax], xmm0  
.done:
  RET

 public_c_symbol vecshl_mmx
; PROC data1:PTR, bits:DWORD, len:DWORD
  ; rcx = data1
  ; rdx = bits
  ; r8 = len

  pxor xmm1, xmm1
  pxor xmm3, xmm3
  pxor xmm4, xmm4

  movd xmm1, edx
  mov eax, 64
  sub eax, edx
  movd xmm3, eax

  mov r9, r8
  shr r8, 1
  jz .fin

.loop:
  movdqa xmm0, [rcx]	; load 2 qwords
  movdqa xmm2, xmm0		; backup the value

  psllq  xmm0, xmm1		; shift left by offset (in xmm1)
  psrlq  xmm2, xmm3		; get the bits we shifted out
  por xmm0, xmm4		; xmm4 has the carryover from the last step

  movdqa xmm4, xmm2		; carryover for next step
  psrldq xmm4, 8		; the bits shifted out from the top qword carry over to the next step
  pslldq xmm2, 8		; the bits shifted out from the bottom qword need to be or'ed to the top one
  por xmm0, xmm2		; xmm2 has the remaining bits from the bottom half

  movdqa [rcx], xmm0	; write back

  add rcx, 16
  dec r8
  jnz .loop
.fin:
  test r9b, 1
  jz .done

  movq xmm0, [rcx]
  psllq  xmm0, xmm1
  por xmm0, xmm4
  movq [rcx], xmm0

.done:
  RET

