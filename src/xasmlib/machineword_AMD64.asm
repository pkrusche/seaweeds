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
  push rdi

  lea rdi, [bitmasks]
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
  lea rdi, [_bitmasks_64]
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

  lea rdi, [bitcount]
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

  pop rdi
  pop rbx
  RET

public_c_symbol add_with_carry32
;PROC a:DWORD, b:DWORD, _c:ptr
  add ecx, edx			; [a] + [b]
  setc dl			; remember first carry
  movzx eax, byte [r8]
  add eax, ecx
  setc cl
  or cl, dl				; we have a carry if either of the additions 
  and cl, 1				; generated an overflow
  mov byte [r8], cl
  RET

public_c_symbol add_with_carry64
; PROC a:QWORD, b:QWORD, _c:PTR
  add rdx, rcx ; [a] + [b]
  setc bl
  movzx rax, byte [r8] 
  add rax, rdx
  setc cl
  or cl, bl
  and cl, 1
  mov byte [r8], cl
  RET

public_c_symbol bittest64
; PROC v:QWORD, b:BYTE
  xchg rcx, rdx
  bt rdx, rcx
  setc al
  RET

; extern UINT64 extractword(UINT64 * source, BYTE bitofs, BYTE bits);
public_c_symbol extractword
; PROC source:PTR, bitofs:BYTE, bits:BYTE
;        rcx          rdx         r8

  ; load the data
  and r8, 03fh
  xchg rcx, rdx     ; now, rcx = bitofs , rdx = source
  mov rax, [rdx]	; rcx contains source
  mov r9,  [rdx+8]	; source is padded with >= 1 QWORD
  ; shift right into rax
  shrd rax, r9, cl
  and rax, [8*r8 + bitmasks_64]	; apply bitmask to only get relevant bits
  RET

public_c_symbol insertword
;PROC source:QWORD, target:ptr, bitofs:BYTE, bits:BYTE
  ; source is in rcx
  ; target is in rdx
  ; bitofs is in r8
  ; bits is in r9
  
  xchg rcx, r8	; bitofs -> rcx, source -> r8
  xchg rdx, r9	; bits -> rdx, target -> r9
  and rcx, 03fh
  and rdx, 03fh

  ; first part of bitmask: set the first bitofs bits
  mov r10, [bitmasks_64 + 8*rcx]

  ; last part of bitmask
  add dx, cx
  mov rax, rdx	 ; copy so we don't mess up the value and can...
  sub dx, 64
  jns .big_write
  
  ; this is what happens if our word is contained within a qword
  and rax, 03fh  ; ...make sure we don't read where we shouldn't
  mov r11, [bitmasks_64 + 8*rax]
  not r11				; everything interesting happens in the first qword
  or r11, r10			; set all bits up to bitofs and after bitofs+bits
  mov rax, [r9]			; get vector data
  shl r8, cl			; shift input into place
  and rax, r11			; make sure we only get the relevant bits
  not r11				; reset the bits in the vector data
  and r8, r11			; ...
  or rax, r8			; and put in our data
  mov [r9], rax			; put qword back
  RET

.big_write:				; this is what happens if our word crosses a qword boundary
  mov rax, [r9]			; we first write the first half
  xor r11, r11			; for shifting, remember the bits that cross the boundary
  shld r11, r8, cl		; and hence goes into the next qword
  shl r8, cl			; shift the first half into place
  and rax, r10			; remove the bits we will replace
  or rax, r8			; put in the first half
  mov [r9], rax			; write back

  mov r8, [bitmasks_64 + 8*rdx] ; get second half of mask
  and r11, r8			; r11 contains the upper half of the word
  not r8				; invert write mask
  mov rax, [r9+8]		; read data
  and rax, r8			; reset bits
  or rax, r11  			; insert our bits
  mov [r9+8], rax		; put back
  RET


public_c_symbol vecadd
;PROC data1:PTR, data2:PTR, len:DWORD, carry : BYTE
;       rcx       rdx         r8        r9
  clc
  lahf
  mov al, r9b
  and al, 1
  or ah, al ; carry
  xor r9, r9
.loop:
  mov r10, [rdx + 8*r9]
  sahf
  adc [rcx + 8*r9], r10
  lahf
  inc r9
  dec r8
  jnz .loop
  sahf
  setc al

  RET

public_c_symbol vecsub
;PROC data1:PTR, data2:PTR, len:DWORD, carry : BYTE
;       rcx       rdx         r8        r9
  clc
  lahf
  mov al, r9b
  and al, 1
  or ah, al ; carry
  xor r9, r9
.loop:
  mov r10, [rdx + 8*r9]
  sahf
  sbb [rcx + 8*r9], r10
  lahf
  inc r9
  dec r8
  jnz .loop
  sahf
  setc al
  RET

public_c_symbol vecadd_cipr
; PROC M:PTR, L:PTR, len:DWORD, carry : BYTE
  ; rcx contains M
  ; rdx contains L
  ; r8 contains len
  ; r9 contains carry
  push rsi
  
  mov al, r9b	; carry -> al
  mov rsi, rcx
  xor rcx, rcx
.loop:
  mov r11, [rdx + 8*rcx]	; r11 = L[i]
  mov r9,  [rsi + 8*rcx]	; r9  = M[i]
  mov r10, r9
  not r10				; r10 = ~M
  and r10, r11			; r10 = V = L & ~M
  and r9, r11			; r9 = U = L &  M  [= M & L]
  bt ax, 0
  adc r11, r9			; L' = (L+U) ... 
  setc al
  or r11, r10			; ... | V
  mov [rdx + 8*rcx], r11 ; write back to L
  inc rcx
  dec r8
  jnz .loop
  
  pop rsi
  RET

%macro simple_vecop 1
public_c_symbol vec%1
 ; PROC data1:PTR, data2:PTR, len:DWORD
 ;        rcx        rdx         r8
  xor rax, rax
.loop:
  mov r9, [rdx + 8*rax]	; load from data2
  %1 [rcx + 8*rax], r9	; store to data1
  inc rax
  dec r8
  jnz .loop
  RET
%endmacro

simple_vecop and
simple_vecop or
simple_vecop xor

public_c_symbol vecshl
; PROC data1:PTR, bits:DWORD, len:DWORD
;       rcx         rdx         r8
  xor r10, r10
  xchg rcx, rdx
.loop:
  mov rax, [rdx]	; load data
  mov r9, rax		; save value 
  shld rax, r10, cl	; shift left, getting the lower bits from last value in r10
  mov r10, r9		; save last value for getting upper bits in next step
  mov [rdx], rax	; write back
  add rdx, 8
  dec r8			; r8 contains len
  jnz .loop

  RET

public_c_symbol vecshr
; PROC data1:PTR, bits:DWORD, len:DWORD
;        rcx         rdx        r8
  dec r8	; MWW is padded by two QWORDs. 
			; We skip one here so we can read ahead a little
  xchg rcx, rdx		; we want bits in rcx for shifting
  mov rax, [rdx]	; get first qword
.loop:
  mov r9, [rdx+8]
  shrd rax, r9, cl
  mov [rdx], rax
  add rdx, 8
  mov rax, r9
  dec r8
  jnz .loop
  RET

public_c_symbol countbits
; PROC source:PTR, len:DWORD
;         rcx        rdx
  xor rax, rax
  test rdx, rdx
  jz .finished
  
  xor r8, r8
  xor r10, r10
.loop:
  mov r9, [rcx]

  ; word 1
  mov r8w, r9w
  mov r10b, [r8+bitcount]
  add rax, r10
  
  ; word 2
  rol r9, 16
  mov r8w, r9w
  mov r10b, [r8+bitcount]
  add rax, r10

  ; word 3
  rol r9, 16
  mov r8w, r9w
  mov r10b, [r8+bitcount]
  add rax, r10

  ; word 4
  rol r9, 16
  mov r8w, r9w
  mov r10b, [r8+bitcount]
  add rax, r10

  add rcx, 8
  dec rdx
  jnz .loop
.finished:
  ; return value is in rax
  RET

public_c_symbol incs_8
; PROC source: BYTE
  mov rax, rcx
  inc ax
  or dx, 0ffffh
  bt ax, 8
  cmovc ax, dx
  RET

public_c_symbol incs_16
; PROC source: WORD
  mov rax, rcx
  inc eax
  or dx, 0ffffh
  bt eax, 16
  cmovc eax, edx
  RET

public_c_symbol incs_32
; PROC source: DWORD
  mov rax, rcx
  or edx, 0ffffffffh
  add eax, 1
  cmovc eax, edx
  RET
 
; reset the bits after the end of a vector
public_c_symbol fixending_64
; PROC data:ptr, value_bits:DWORD, vwords:DWORD, datalen:DWORD
  ; rcx contains data
  ; rdx contains value_bits
  ; r8 contains vwords
  ; r9 contains datalen
  mov rax, rdx   ; value_bits
  
  test r8, r8
  jz .only_rest	 ; if the total number of bits is zero: only fill the rest
  mul r8	     ; value_bits*vwords = number of bits in total in rdx:rax

  mov r10, rax
  and r10, 03fh   ; remaining bits are removed using a bitmask
  mov r11, [8*r10 + bitmasks_64]
  
  shrd rax, rdx, 3 ; -> number of bytes in total
  and al, 0f8h   ; align 8
  add rcx, rax   ; move to first qword we need to mask

  ; last qword that contains data
  and [rcx], r11
  add rcx, 8

.only_rest:  
  ; fill the rest with zeros
  ; r9 contains datalen, rax contains 8-aligned number of bytes containing data
  ; rax/= 8 -> number in qwords
  shr rax, 3
  inc rax		; we look for the first qword that does not contain data
  sub r9, rax	; qword count -= offset of first qword without data
  jbe .end	
  xor rax, rax
.fill:
  mov [rcx], rax
  add rcx, 8
  dec r9
  jnz .fill

.end:
  RET
  
