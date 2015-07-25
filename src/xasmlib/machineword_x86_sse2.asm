;***************************************************************************
;*   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
;*   pkrusche@gmail.com                                                    *
;***************************************************************************

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
  push esi
  
  mov esi, [esp+8]  ; data1
  mov ecx, [esp+12]	; len  
  mov eax, ecx

  movdqa xmm1, [mmx_one_b]       ; mm1 =     1     1     1 ... 

  shr ecx, 1
  jz .fin
.loop:
  movdqa xmm0, [esi]
  paddusb xmm0, xmm1
  movdqa [esi], xmm0
  add esi, 16
  dec ecx
  jnz .loop
.fin:
  and eax, 1
  jz .done
  ; do last qword if necessary
  movq xmm0, [esi]
  paddusb xmm0, xmm1
  movq [esi], xmm0
.done:
  pop esi
  RET



; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_16_mmx
; PROC data1:PTR, len:DWORD
  push esi
  
  mov esi, [esp+8]  ; data1
  mov ecx, [esp+12]	; len  
  mov eax, ecx

  pcmpeqw  xmm1, xmm1        ; mm1 = 0ffff 0ffff 0ffff ...
  psrlw    xmm1, 15         ; mm1 =     1     1     1 ... 

  shr ecx, 1
  jz .fin
.loop:
  movdqa xmm0, [esi]
  paddusw xmm0, xmm1
  movdqa [esi], xmm0
  add esi, 16
  dec ecx
  jnz .loop
.fin:
  and eax, 1
  jz .done
  ; do last qword if necessary
  movq xmm0, [esi]
  paddusw xmm0, xmm1
  movq [esi], xmm0
.done:
  pop esi
  RET


; generate an 8 bit match mask from two 8 bit per character strings.
; we assume the strings are padded to 64 bits.
; zero matches all.
public_c_symbol generate_match_mask_c8_v8_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  push esi
  push edi
  push ebx

  mov esi, [esp+16]  ; string1
  mov edi, [esp+20]  ; string2
  mov ebx, [esp+24]  ; mask_out
  mov ecx, dword [esp+28] ; len in 64 bit blocks
  
  mov edx, ecx
  shr ecx, 1
  jz .fin
  
  xor eax, eax
.loop:
  movdqa xmm0, [esi+eax]
  movdqa xmm1, [edi+eax]
  
  pcmpeqb xmm0, xmm1
  
  movdqa [ebx+eax], xmm0
  
  add eax, 16
  dec ecx
  jnz .loop
.fin:
  and edx, 1
  jz .done

  movdqa xmm0, [esi+eax]
  movdqa xmm1, [edi+eax]
  
  pcmpeqb xmm0, xmm1
  
  movdqa [ebx+eax], xmm0
  
.done:
  pop ebx
  pop edi
  pop esi
  RET

; generate an 16 bit match mask from two 16 bit per character strings.
; we assume the strings are padded to 64 bits.
; zero matches all.
public_c_symbol generate_match_mask_c16_v16_mmx
;PROC string1:PTR, string2:PTR, mask_out:PTR, len:DWORD
  push esi
  push edi
  push ebx

  mov esi, [esp+16]  ; string1
  mov edi, [esp+20]  ; string2
  mov ebx, [esp+24]  ; mask_out
  mov ecx, dword [esp+28] ; len in 64 bit blocks
  
  mov edx, ecx
  shr ecx, 1
  jz .fin
  
  xor eax, eax
.loop:
  movdqa xmm0, [esi+eax]
  movdqa xmm1, [edi+eax]
  
  pcmpeqw xmm0, xmm1
  
  movdqa [ebx+eax], xmm0
  
  add eax, 16
  dec ecx
  jnz .loop
.fin:
  and edx, 1
  jz .done
  movdqa xmm0, [esi+eax]
  movdqa xmm1, [edi+eax]
  
  pcmpeqw xmm0, xmm1
  
  movdqa [ebx+eax], xmm0
  
.done:

  pop ebx
  pop edi
  pop esi
  RET

; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_16_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  push esi
  push edi
  push ebx

  mov esi, [esp+16] ; data1
  mov edi, [esp+20] ; data2
  mov ecx, [esp+24] ; len
  xor ebx, ebx
  
  mov edx, ecx

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5
  movdqa xmm6, xmm5
  ; xmm6 = 00010001....
  psrlw xmm6, 15

  shr ecx, 1
  jz .fin
    
.loop:
  movdqa xmm0, [esi+ebx]
  movdqa xmm1, [edi+ebx]
  
  movdqa xmm2, xmm1
  psubusw xmm2, xmm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddw xmm2, xmm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; mm3 = !mm3
  paddw xmm3, xmm6	; mm3 = -mm2 (two's complement), i.e. 
					; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddw xmm3, xmm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddw xmm3, xmm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movdqa [esi+ebx], xmm3	; the smaller values
  movdqa [edi+ebx], xmm2	; the larger values

  add ebx, 16
  dec ecx
  jnz .loop

.fin:
  and edx, 1
  jz .done
  movq xmm0, [esi+ebx]
  movq xmm1, [edi+ebx]
  
  movq xmm2, xmm1
  psubusw xmm2, xmm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddw xmm2, xmm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movq xmm3, xmm2
  pandn xmm3, xmm5	; mm3 = !mm3
  paddw xmm3, xmm6	; mm3 = -mm2 (two's complement), i.e. 
					; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddw xmm3, xmm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddw xmm3, xmm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movq [esi+ebx], xmm3	; the smaller values
  movq [edi+ebx], xmm2	; the larger values
  
.done:
  pop ebx
  pop edi
  pop esi
  RET

; elementwise compare and sort
; data1 gets the smaller elements
public_c_symbol cmpxchg_8_mmx
;PROC data1:PTR, data2:PTR, len:DWORD
  push esi
  push edi
  push ebx

  mov esi, [esp+16] ; data1
  mov edi, [esp+20] ; data2
  mov ecx, [esp+24] ; len
  xor ebx, ebx
  
  mov edx, ecx

  ; xmm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5
  ; xmm6 = 01010101....
  movdqa xmm6, [mmx_one_b]

  shr ecx, 1
  jz .fin
    
.loop:
  movdqa xmm0, [esi+ebx]
  movdqa xmm1, [edi+ebx]
  
  movdqa xmm2, xmm1
  psubusb xmm2, xmm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddb xmm2, xmm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movdqa xmm3, xmm2
  pandn xmm3, xmm5	; mm3 = !mm3
  paddb xmm3, xmm6	; mm3 = -mm2 (two's complement), i.e. 
					; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddb xmm3, xmm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddb xmm3, xmm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movdqa [esi+ebx], xmm3	; the smaller values
  movdqa [edi+ebx], xmm2	; the larger values

  add ebx, 16
  dec ecx
  jnz .loop

.fin:
  and edx, 1
  jz .done
  movq xmm0, [esi+ebx]
  movq xmm1, [edi+ebx]
  
  movq xmm2, xmm1
  psubusb xmm2, xmm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddb xmm2, xmm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movq xmm3, xmm2
  pandn xmm3, xmm5	; mm3 = !mm3
  paddb xmm3, xmm6	; mm3 = -mm2 (two's complement), i.e. 
					; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddb xmm3, xmm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddb xmm3, xmm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movq [esi+ebx], xmm3	; the smaller values
  movq [edi+ebx], xmm2	; the larger values
  
.done:
  pop ebx
  pop edi
  pop esi
  RET

; replace bits in data with bits in data2 if corresponding bit in mask is set
public_c_symbol replace_if_mmx
;PROC data:PTR, data2:PTR, mask:PTR, len:DWORD
  push esi
  push edi
  push ebx

  mov edi, [esp+16] ; data
  mov esi, [esp+20] ; data2
  mov ebx, [esp+24] ; mask
  mov ecx, [esp+28] ; len
  xor eax, eax

  mov edx, ecx
  
  ; mm5 == 0ffffff....ff
  pcmpeqd xmm5, xmm5

  shr ecx, 1
  jz .fin
  
.loop:
  movdqa xmm0, [edi+eax]		; load first value
  movdqa xmm1, [esi+eax]		; load second value
  movdqa xmm2, [ebx+eax]		; load mask
 
  pand xmm1, xmm2				; exchange if bits in mask are set
  pandn xmm2, xmm5
  pand xmm0, xmm2
  por xmm0, xmm1  
  
  movdqa [edi+eax], xmm0
  
  add eax, 16
  dec ecx
  jnz .loop
.fin:
  movq xmm0, [edi+eax]		; load first value
  movq xmm1, [esi+eax]		; load second value
  movq xmm2, [ebx+eax]		; load mask
 
  pand xmm1, xmm2				; exchange if bits in mask are set
  pandn xmm2, xmm5
  pand xmm0, xmm2
  por xmm0, xmm1  
  
  movq [edi+eax], xmm0

.done:
  pop ebx
  pop edi
  pop esi
  RET

public_c_symbol vecshl_mmx
; PROC data1:PTR, bits:DWORD, len:DWORD
  ; [esp+4]  = data1
  ; [esp+8]  = bits
  ; [esp+12] = len
; PROC data1:PTR, bits:DWORD, len:DWORD
  push edi
  mov edi, [esp+8] ; data1
  mov ecx, [esp+12] ; bits

  pxor xmm1, xmm1
  pxor xmm3, xmm3
  pxor xmm4, xmm4

  movd xmm1, ecx
  mov eax, 64
  sub eax, ecx
  movd xmm3, eax

  mov edx, [esp+16] ; len
  mov eax, edx
  shr edx, 1
  jz .fin

.loop:
  movdqa xmm0, [edi]	; load 2 qwords
  movdqa xmm2, xmm0		; backup the value

  psllq  xmm0, xmm1		; shift left by offset (in xmm1)
  psrlq  xmm2, xmm3		; get the bits we shifted out
  por xmm0, xmm4		; xmm4 has the carryover from the last step

  movdqa xmm4, xmm2		; carryover for next step
  psrldq xmm4, 8		; the bits shifted out from the top qword carry over to the next step
  pslldq xmm2, 8		; the bits shifted out from the bottom qword need to be or'ed to the top one
  por xmm0, xmm2		; xmm2 has the remaining bits from the bottom half

  movdqa [edi], xmm0	; write back

  add edi, 16
  dec edx
  jnz .loop
.fin:
  test al, 1
  jz .done

  movq xmm0, [edi]
  psllq  xmm0, xmm1
  por xmm0, xmm4
  movq [edi], xmm0

.done:
  pop edi
  RET
