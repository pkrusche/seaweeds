;***************************************************************************
;*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
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

ALIGN 8
  ; bitmask templates: index: (start of ones at right) >> 5 or (end of ones at left)
bitmasks times 1024 dd (0)

  ; bitmask templates with all bits up to j set in bitmasks_64[j]
public_c_symbol bitmasks_64 
  times 65 dq (0)

  ; bitcounting helper. bitcount [w] = number of bits set in w
bitcount times 65536 db (0)

GLOBAL _bitmasks_64

SECTION .text

public_c_symbol initbitmasks
  push rbx

  lea rdi, [bitmasks wrt rip]
  xor eax, eax
  mov ebx, 1023
.loop:
  mov eax, 0ffffffffh
  mov ch, bl
  and ch, 1fh
  mov cl, 32
  sub cl, ch
  shl eax, cl
  shr eax, cl
  mov cx, bx
  shr cx, 5
  shr eax, cl
  shl eax, cl
  mov [rdi+4*rbx], eax
  dec ebx
  jns .loop

  ; 64 bit bitmasks
  lea rdi, [_bitmasks_64 wrt rip]
  mov cx, 64
  xor rax, rax
  mov rdx, 0ffffffffffffffffh
.loop2:
  mov [rdi], rax
  shld rax, rdx, 1
  add rdi, 8
  dec cx
  jnz .loop2
  mov [rdi], rax

  lea rdi, [bitcount wrt rip]
  mov ecx, 65536
  xor ebx, ebx
.loop3:
  xor al, al
  mov dx, bx
.loop3_i:
  mov ah, dl
  and ah, 1
  add al, ah
  shr dx, 1
  jnz .loop3_i
  stosb
  inc ebx
  dec ecx
  jnz .loop3

  pop rbx
  RET

public_c_symbol add_with_carry32
;PROC a:DWORD, b:DWORD, _c:ptr
  mov rax, rdi			; [a]
  mov rdi, rdx
  add eax, esi			; [b]
  setc dl			; remember first carry
  movzx ecx, byte [rdi]
  add eax, ecx
  setc cl
  or cl, dl				; we have a carry if either of the additions 
  and cl, 1				; generated an overflow
  mov byte [rdi], cl
  RET

public_c_symbol add_with_carry64
; PROC a:QWORD, b:QWORD, _c:PTR
  add rdi, rsi
  setc al
  movzx rcx, byte [rdx]
  add rdi, rcx
  setc cl
  or cl, al
  and cl, 1
  mov rax, rdi
  mov byte [rdx], cl
  RET

public_c_symbol bittest64
; PROC v:QWORD, b:BYTE
  mov rcx, rsi
  bt rdi, rcx
  setc al
  RET

public_c_symbol vecadd
;PROC data1:PTR, data2:PTR, len:DWORD, carry : BYTE
  clc
  lahf
  and cl, 1
  or ah, cl ; carry
  xor rcx, rcx
.loop:
  mov r8, [rsi + 8*rcx]
  sahf
  adc [rdi + 8*rcx], r8
  lahf
  inc rcx
  dec rdx
  jnz .loop
  mov al, ah
  and rax, 1

  RET

public_c_symbol vecsub
;PROC data1:PTR, data2:PTR, len:DWORD, carry : BYTE
  clc
  lahf
  and cl, 1
  or ah, cl ; carry
  xor rcx, rcx
.loop:
  mov r8, [rsi + 8*rcx]
  sahf
  sbb [rdi + 8*rcx], r8
  lahf
  inc rcx
  dec rdx
  jnz .loop
  mov al, ah
  and rax, 1
  RET

public_c_symbol vecadd_cipr
; PROC M:PTR, L:PTR, len:DWORD, carry : BYTE
  ; rdi contains M
  ; rsi contains L
  ; rdx contains len
  ; rcx contains carry
  
  mov al, cl
  xor rcx, rcx
.loop:
  mov r8, [rsi + 8*rcx]	; r8 = L[i]
  mov r9, [rdi + 8*rcx]	; r9 = M[i]
  mov r10, r9
  not r10
  and r10, r8			; r10 = V = L & ~M
  and r9,  r8			; r9  = U = L &  M
  bt ax, 0
  adc r8, r9			; L' = (L+U)|V
  setc al
  or r8, r10
  mov [rsi + 8*rcx], r8
  inc rcx
  dec rdx
  jnz .loop
  RET

%macro simple_vecop 1
public_c_symbol vec%1
 ;PROC data1:PTR, data2:PTR, len:DWORD
  xor rcx, rcx
.loop:
  mov r8, [rsi + 8*rcx]
  %1 [rdi + 8*rcx], r8
  inc rcx
  dec rdx
  jnz .loop
  RET
%endmacro

simple_vecop and
simple_vecop or
simple_vecop xor

public_c_symbol vecshl
; PROC data1:PTR, bits:DWORD, len:DWORD

  xor r8, r8  
  mov cx, si
.loop:
  mov rax, [rdi]  
  mov r9, rax
  shld rax, r8, cl
  mov r8, r9
  mov [rdi], rax
  add rdi, 8
  dec rdx
  jnz .loop

  RET

public_c_symbol vecshr
; PROC data1:PTR, bits:DWORD, len:DWORD
  ; rdi contains data1
  ; rsi contains bits
  ; rdx contains len 

  dec rdx	; MWW is padded by two QWORDs. We skip one here so we can read ahead a little
  mov rax, [rdi]
  mov cx, si	; move bits to cl
.loop:
  mov r8, [rdi+8]
  shrd rax, r8, cl
  stosq
  mov rax, r8
  dec rdx
  jnz .loop
  RET

public_c_symbol countbits
; PROC source:PTR, len:DWORD
 ; source = rdi
 ; len = rsi

  xor rax, rax
  xor rdx, rdx
  test rsi, rsi
  jz .finished
  xor r8, r8
  lea r10, [bitcount wrt rip]
.loop:
  mov rcx, [rdi]

  ; word 1
  mov r8w, cx
  mov dl, [r8+r10]
  add rax, rdx
  
  ; word 2
  rol rcx, 16
  mov r8w, cx
  mov dl, [r8+r10]
  add rax, rdx

  ; word 3
  rol rcx, 16
  mov r8w, cx
  mov dl, [r8+r10]
  add rax, rdx

  ; word 4
  rol rcx, 16
  mov r8w, cx
  mov dl, [r8+r10]
  add rax, rdx
  
  add rdi, 8
  dec rsi
  jnz .loop
.finished:
  ; return value is in rax
  RET

public_c_symbol incs_8
; PROC source: BYTE
  mov rax, rdi
  inc ax
  or dx, 0ffffh
  bt ax, 8
  cmovc ax, dx
  RET

public_c_symbol incs_16
; PROC source: WORD
  mov rax, rdi
  inc eax
  or dx, 0ffffh
  bt eax, 16
  cmovc eax, edx
  RET

public_c_symbol incs_32
; PROC source: WORD
  mov rax, rdi
  or edx, 0ffffffffh
  add eax, 1
  cmovc eax, edx
  RET
 
; reset the bits after the end of a vector
public_c_symbol fixending_64
; PROC data:ptr, value_bits:DWORD, vwords:DWORD, datalen:DWORD
  ; rdi contains data
  ; rsi contains value_bits
  ; rdx contains vwords
  ; rcx contains datalen
  mov rax, rsi   ; value_bits

  mov r8, rdx	 ; save vword_len
  test rdx, rdx
  jz .only_rest	 ; if the total number of bits is zero: only fill the rest
  mul r8	     ; value_bits*vwords = number of bits in total

  mov r9, rax

  and r9, 03fh   ; remaining bits are removed using a bitmask
  lea r10, [bitmasks_64 wrt rip]
  mov r8, [8*r9 + r10]
  
  shrd rax, rdx, 3     ; -> number of bytes in total
  and al, 0f8h   ; align 8
  add rdi, rax   ; 

  ; last relevant qword    
  and [rdi], r8
  add rdi, 8

.only_rest:  
  ; fill the rest with zeros
  ; rcx contains datalen
  ; rax/= 8 -> number in qwords
  shr rax, 3
  inc rax
  sub rcx, rax
  jbe .end
  xor rax, rax
.loop:
  mov [rdi], rax
  add rdi, 4
  dec rcx
  jne .loop
.end:
  RET
  
