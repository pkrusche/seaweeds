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
  ; rdi contains data1
  ; rsi contains len  

  mov rcx, rsi
  mov rsi, rdi

  movdqa xmm1, [mmx_one_b wrt rip]       ; xmm1 =     1     1     1 ... 
  
  mov rax, rcx
  shr rcx, 1
  jz .fin
    
.loop:
  movdqa xmm0, [rsi]
  paddusb xmm0, xmm1
  movdqa [rsi], xmm0
  add rsi, 16
  dec rcx
  jnz .loop
.fin:
  and al, 1
  jz .done

  movq xmm0, [rsi]
  paddusb xmm0, xmm1
  movq [rsi], xmm0

.done:
  RET

; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_16_mmx
; PROC data1:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains len  

  mov rcx, rsi
  mov rsi, rdi

  pcmpeqw  xmm1, xmm1        ; xmm1 = 0ffff 0ffff 0ffff ...
  psrlw    xmm1, 15         ; xmm1 =     1     1     1 ... 
  
  mov rax, rcx
  shr rcx, 1
  jz .fin
    
.loop:
  movdqa xmm0, [rsi]
  paddusw xmm0, xmm1
  movdqa [rsi], xmm0
  add rsi, 16
  dec rcx
  jnz .loop
.fin:
  and al, 1
  jz .done

  movq xmm0, [rsi]
  paddusw xmm0, xmm1
  movq [rsi], xmm0
.done:
  RET

; generate an 8 bit match mask from two 8 bit per character strings.
; we assume the strings are padded to 64 bits.
public_c_symbol generate_match_mask_c8_v8_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  ; rdi contains string1
  ; rsi contains string2
  ; rdx contains mask_out
  ; rcx contains len

  xor rax, rax  
  mov r8, rcx
  shr rcx, 1
  jz .fin
  
.loop:
  movdqa xmm0, [rsi+rax]
  movdqa xmm1, [rdi+rax]
  
  pcmpeqb xmm0, xmm1
  
  movdqa [rdx+rax], xmm0
  
  add rax, 16
  dec rcx
  jnz .loop
.fin:
  and r8, 1
  jz .done
  
  movq xmm0, [rsi+rax]
  movq xmm1, [rdi+rax]
  
  pcmpeqb xmm0, xmm1
  
  movq [rdx+rax], xmm0
.done:
  RET

; generate an 16 bit match mask from two 16 bit per character strings.
; we assume the strings are padded to 64 bits.
public_c_symbol generate_match_mask_c16_v16_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  ; rdi contains string1
  ; rsi contains string2
  ; rdx contains mask_out
  ; rcx contains len

  xor rax, rax  
  mov r8, rcx
  shr rcx, 1
  jz .fin
  
.loop:
  movdqa xmm0, [rsi+rax]
  movdqa xmm1, [rdi+rax]
  
  pcmpeqw xmm0, xmm1
  
  movdqa [rdx+rax], xmm0
  
  add rax, 16
  dec rcx
  jnz .loop
.fin:
  and r8, 1
  jz .done
  
  movq xmm0, [rsi+rax]
  movq xmm1, [rdi+rax]
  
  pcmpeqw xmm0, xmm1
  
  movq [rdx+rax], xmm0
.done:
  RET

; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_8_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains data2
  ; rdx contains len

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5
  ; xmm6 = 00010001....
  movdqa  xmm6, [mmx_one_b wrt rip]

  xor rcx, rcx 
  mov rax, rdx
  shr rdx, 1
  jz .fin  
.loop:
  movdqa xmm0, [rsi+rcx]
  movdqa xmm1, [rdi+rcx]
  
  movdqa xmm2, xmm1
  psubusb xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddb xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddb xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddb xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddb xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movdqa [rdi+rcx], xmm3 ; the smaller values
  movdqa [rsi+rcx], xmm2 ; the larger values

  add rcx, 16
  dec rdx
  jnz .loop
.fin:
  and al, 1
  jz .done
  movq xmm0, [rsi+rcx]
  movq xmm1, [rdi+rcx]
  
  movdqa xmm2, xmm1
  psubusb xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddb xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddb xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddb xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddb xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movq [rdi+rcx], xmm3 ; the smaller values
  movq [rsi+rcx], xmm2 ; the larger values  
  
.done:
  RET

; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_16_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  ; rdi contains data1
  ; rsi contains data2
  ; rdx contains len

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5
  movdqa	xmm6, xmm5
  ; xmm6 = 00010001....
  psrlw xmm6, 15

  xor rcx, rcx 
  mov rax, rdx
  shr rdx, 1
  jz .fin  
.loop:
  movdqa xmm0, [rsi+rcx]
  movdqa xmm1, [rdi+rcx]
  
  movdqa xmm2, xmm1
  psubusw xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddw xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddw xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddw xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddw xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movdqa [rdi+rcx], xmm3 ; the smaller values
  movdqa [rsi+rcx], xmm2 ; the larger values

  add rcx, 16
  dec rdx
  jnz .loop
.fin:
  and al, 1
  jz .done
  movq xmm0, [rsi+rcx]
  movq xmm1, [rdi+rcx]
  
  movdqa xmm2, xmm1
  psubusw xmm2, xmm0	; xmm2 = 0   if xmm0 > xmm1 ; xmm2 = xmm1-xmm0 otherwise
  paddw xmm2, xmm0	; xmm2 = xmm0 if xmm0 > xmm1 ; xmm2 = xmm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; xmm3 = !xmm3
  paddw xmm3, xmm6	; xmm3 = -xmm2 (two's complement), i.e. 
			; xmm2 = -xmm0 if xmm0 > xmm1 ; xmm2 = -xmm1     otherwise
  paddw xmm3, xmm0	; xmm2 = 0    if xmm0 > xmm1 ; xmm2 = xmm0-xmm1     otherwise
  paddw xmm3, xmm1	; xmm2 = xmm1  if xmm0 > xmm1 ; xmm2 = xmm0     otherwise

  movq [rdi+rcx], xmm3 ; the smaller values
  movq [rsi+rcx], xmm2 ; the larger values  
  
.done:
  RET

; replace bits in data with bits in data2 if corresponding bit in mask is set
public_c_symbol replace_if_mmx
;PROC data:PTR, data2:PTR, mask:PTR, len:DWORD
  ; rdi contains data
  ; rsi contains data2
  ; rdx contains mask
  ; rcx contains len

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5

  xor rax, rax
  mov r8, rcx
  shr rcx, 1
  jz .fin
  
.loop:
  movdqa xmm0, [rdi+rax]		; load first value
  movdqa xmm1, [rsi+rax]		; load second value
  movdqa xmm2, [rdx+rax]		; load mask
  
  pand xmm1, xmm2			; exchange if bits in mask are set
  pandn xmm2, xmm5
  pand xmm0, xmm2
  por xmm0, xmm1  
  
  movdqa [rdi+rax], xmm0
  
  add rax, 16
  dec rcx
  jnz .loop
.fin:
  and r8, 1
  jz .done

  movq xmm0, [rdi+rax]		; load first value
  movq xmm1, [rsi+rax]		; load second value
  movq xmm2, [rdx+rax]		; load mask
  
  pand xmm1, xmm2			; exchange if bits in mask are set
  pandn xmm2, xmm5
  pand xmm0, xmm2
  por xmm0, xmm1  
  
  movq [rdi+rax], xmm0  
.done:
  RET

 public_c_symbol vecshl_mmx
; PROC data1:PTR, bits:DWORD, len:DWORD
  ; rdi = data1
  ; rsi = bits
  ; rdx = len

  pxor xmm1, xmm1
  pxor xmm3, xmm3
  pxor xmm4, xmm4

  movd xmm1, esi
  mov ecx, 64
  sub ecx, esi
  movd xmm3, ecx

  mov r8, rdx
  shr rdx, 1
  jz .fin

.loop:
  movdqa xmm0, [rdi]	; load 2 qwords
  movdqa xmm2, xmm0		; backup the value

  psllq  xmm0, xmm1		; shift left by offset (in xmm1)
  psrlq  xmm2, xmm3		; get the bits we shifted out
  por xmm0, xmm4		; xmm4 has the carryover from the last step

  movdqa xmm4, xmm2		; carryover for next step
  psrldq xmm4, 8		; the bits shifted out from the top qword carry over to the next step
  pslldq xmm2, 8		; the bits shifted out from the bottom qword need to be or'ed to the top one
  por xmm0, xmm2		; xmm2 has the remaining bits from the bottom half

  movdqa [rdi], xmm0	; write back

  add rdi, 16
  dec rdx
  jnz .loop
.fin:
  test r8b, 1
  jz .done

  movq xmm0, [rdi]
  psllq  xmm0, xmm1
  por xmm0, xmm4
  movq [rdi], xmm0

.done:
  RET

