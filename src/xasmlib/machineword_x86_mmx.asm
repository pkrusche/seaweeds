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

mmx_one_b: dq 0101010101010101h

SECTION .text

public_c_symbol do_emms
  emms
  RET

; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_8_mmx
; PROC data1:PTR, len:DWORD
  push esi
  
  mov esi, [esp+8]  ; data1
  mov edi, esi
  mov ecx, [esp+12]	; len  
  movq mm1, [mmx_one_b]       ; mm1 =     1     1     1 ... 
.loop:
  movq mm0, [esi]
  paddusb mm0, mm1
  movq [esi], mm0
  add esi, 8
  dec ecx
  jnz .loop

.done:
  pop esi
  RET



; saturated increment for a vector of words, mmx version
public_c_symbol vecsatinc_16_mmx
; PROC data1:PTR, len:DWORD
  push esi
  
  mov esi, [esp+8]  ; data1
  mov edi, esi
  mov ecx, [esp+12]	; len
  pcmpeqw  mm1, mm1        ; mm1 = 0ffff 0ffff 0ffff ...
  psrlw    mm1, 15         ; mm1 =     1     1     1 ... 
.loop:
  movq mm0, [esi]
  paddusw mm0, mm1
  movq [esi], mm0
  add esi, 8
  dec ecx
  jnz .loop

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
  xor eax, eax
  
.loop:
  movq mm0, [esi+8*eax]
  movq mm1, [edi+8*eax]
  
  pcmpeqb mm0, mm1
  
  movq [ebx+8*eax], mm0
  
  inc eax  
  dec ecx
  jnz .loop

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
  xor eax, eax
  
.loop:
  movq mm0, [esi+8*eax]
  movq mm1, [edi+8*eax]
  
  pcmpeqw mm0, mm1
  
  movq [ebx+8*eax], mm0
  
  inc eax  
  dec ecx
  jnz .loop

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
  
  ; mm5 == 0ffffff....ff
  pcmpeqd mm5, mm5
  movq	mm6, mm5
  ; mm6 = 00010001....
  psrlw mm6, 15
  
.loop:
  movq mm0, [esi+8*ebx]
  movq mm1, [edi+8*ebx]
  
  movq mm2, mm1
  psubusw mm2, mm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddw mm2, mm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movq mm3, mm2
  pandn mm3, mm5	; mm3 = !mm3
  paddw mm3, mm6	; mm3 = -mm2 (two's complement), i.e. 
					; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddw mm3, mm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddw mm3, mm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movq [esi+8*ebx], mm3	; the smaller values
  movq [edi+8*ebx], mm2 ; the larger values

  inc ebx
  dec ecx
  jnz .loop
  
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
  
  ; mm5 == 0ffffff....ff
  pcmpeqd mm5, mm5
  ; mm6 = 00010001....
  movq  mm6, [mmx_one_b]
  
.loop:
  movq mm0, [esi+8*ebx]
  movq mm1, [edi+8*ebx]
  
  movq mm2, mm1
  psubusb mm2, mm0	; mm2 = 0   if mm0 > mm1 ; mm2 = mm1-mm0 otherwise
  paddb mm2, mm0	; mm2 = mm0 if mm0 > mm1 ; mm2 = mm1     otherwise

  movq mm3, mm2
  pandn mm3, mm5	; mm3 = !mm3
  paddb mm3, mm6	; mm3 = -mm2 (two's complement), i.e. 
					; mm2 = -mm0 if mm0 > mm1 ; mm2 = -mm1     otherwise
  paddb mm3, mm0	; mm2 = 0    if mm0 > mm1 ; mm2 = mm0-mm1     otherwise
  paddb mm3, mm1	; mm2 = mm1  if mm0 > mm1 ; mm2 = mm0     otherwise

  movq [esi+8*ebx], mm3	; the smaller values
  movq [edi+8*ebx], mm2 ; the larger values

  inc ebx
  dec ecx
  jnz .loop
  
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
  ; mm5 == 0ffffff....ff
  pcmpeqd mm5, mm5

.loop:
  movq mm0, [edi+8*eax]		; load first value
  movq mm1, [esi+8*eax]		; load second value
  movq mm2, [ebx+8*eax]		; load mask
 
; TODO could specialize for bytes:
;  maskmovq mm0, mm2
;  pandn mm2, mm5
;  maskmovq mm1, mm2

  pand mm1, mm2				; exchange if bits in mask are set
  pandn mm2, mm5
  pand mm0, mm2
  por mm0, mm1  
  
  movq [edi+8*eax], mm0
  
  inc eax
  dec ecx
  jnz .loop

  pop ebx
  pop edi
  pop esi
  RET
