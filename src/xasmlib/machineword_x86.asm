;***************************************************************************
;*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
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
  push edi
  push esi
  push ebx

  lea edi, [bitmasks]
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
  mov [edi+4*ebx], eax
  dec ebx
  jns .loop

  ; 64 bit bitmasks
  lea edi, [_bitmasks_64]
  mov ebx, 64
  xor eax, eax
  xor edx, edx
  mov esi, 0ffffffffh
.loop2:
  mov [edi], eax
  mov [edi+4], edx
  shld edx, eax, 1
  shld eax, esi, 1
  add edi, 8
  dec ebx
  jnz .loop2
  mov [edi], eax
  mov [edi+4], edx

  lea edi, [bitcount]
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

  pop ebx
  pop esi
  pop edi
  RET

public_c_symbol add_with_carry32
;PROC a:DWORD, b:DWORD, _c:ptr
  push esi
  mov eax, [esp+8]		; [a]
  add eax, [esp+12]		; [b]
  setc dl				; remember first carry
  mov esi, [esp+16]		; [&c]
  movzx ecx, byte [esi]
  add eax, ecx
  setc cl
  or cl, dl				; we have a carry if either of the additions 
  and cl, 1				; generated an overflow
  mov byte [esi], cl
  pop esi
  RET

public_c_symbol add_with_carry64
; PROC a:QWORD, b:QWORD, _c:PTR
  push esi
  mov eax, dword [esp+8]	; [a] low
  mov edx, dword [esp+12]	; [a] high
  add eax, dword [esp+16]	; [b] low
  adc edx, dword [esp+20]	; [b] high
  setc bl
  mov esi, dword [esp+24]	; &_c
  movzx ecx, byte [esi]
  add eax, ecx
  setc cl
  or cl, bl
  and cl, 1
  mov byte [esi], cl
  pop esi
  RET

public_c_symbol bittest64
; PROC v:QWORD, b:BYTE
  movzx edx, byte [esp+12]	; = b
  mov ecx, edx
  and ecx, 0x1f
  shr edx, 5
  shl edx, 2
  add edx, 4
  bt dword [esp+edx], ecx
  setc al
  RET

public_c_symbol vecor_elemwise
; data1:PTR, len:DWORD, bits:DWORD, wordlen:BYTE
  push esi
  push edi
  push ebp
  push ebx

  ; fill bits...
  mov eax, [esp+28]		;	bits
  mov esi, eax
  xor ebx, ebx

  mov cl, 32
  sub cl, [esp+32]				; wordlen
  shld eax, ebx, cl
  mov ebx, eax

  mov cl, [esp+32] 				; wordlen
  mov ch, 32
.filling:
  shld esi, ebx, cl
  sub ch, cl
  jge .filling

  ; determine shift
  add ch, cl ; ch = 32%w
  mov al, cl
  sub al, ch
  mov ch, al ; ch = w - 32%w
  mov al, cl
  xor cl, cl ; al = w

  mov edi, [esp+20] ; data1
  ; length in qwords.
  mov ebp, [esp+24] ; len
  shl ebp, 1
.loop:
  mov edx, esi
  shld edx, ebx, cl
  or [edi], edx

  ; determine next shift
  add cl, ch
  mov dx, cx
  sub cl, al
  cmovs cx, dx

  add edi, 4
  dec ebp
  jnz .loop

  pop ebx
  pop ebp
  pop edi
  pop esi
  RET

public_c_symbol vecand_elemwise
; PROC data1:PTR, len:DWORD, bits:DWORD, wordlen:BYTE
  push esi
  push edi
  push ebp
  push ebx

  ; fill bits...
  mov eax, [esp+28] ; bits
  mov esi, eax
  xor ebx, ebx

  mov cl, 32
  sub cl, [esp+32] ; wordlen
  shld eax, ebx, cl
  mov ebx, eax

  mov cl, [esp+32] ; wordlen
  mov ch, 32
.filling:
  shld esi, ebx, cl
  sub ch, cl
  jge .filling

  ; determine shift
  add ch, cl ; ch = 32%w
  mov al, cl
  sub al, ch
  mov ch, al ; ch = w - 32%w
  mov al, cl
  xor cl, cl ; al = w

  mov edi, [esp+20] ; data1
  ; length in qwords.
  mov ebp, [esp+24] ; len
  shl ebp, 1
.loop:
  mov edx, esi
  shld edx, ebx, cl
  and [edi], edx

  ; determine next shift
  add cl, ch
  mov dx, cx
  sub cl, al
  cmovs cx, dx

  add edi, 4
  dec ebp
  jnz .loop

  pop ebx
  pop ebp
  pop edi
  pop esi
  RET

; extern UINT64 extractword(UINT64 * source, BYTE bitofs, BYTE bits);
public_c_symbol extractword
; PROC source:PTR, bitofs:BYTE, bits:BYTE
  push esi
  push ebx
  
  mov esi, [esp+12] ; source

  xor dx, dx
  mov eax, 32
  ; skip empty dwords
  mov cl, [esp+16] ; bitofs
  cmp cl, 32
  cmovb ax, dx
  sub cl, al
  shr al, 3
  add esi, eax

  ; we know now that the real bit offset in cl is < 32
  mov [esp+16], cl ; bitofs
  ; compute the ending position
  add cl, [esp+20] ; bits

  ; load the data
  xor edx, edx
  xor ebx, ebx
  mov eax, [esi]
  cmp cl, 32
  cmovae edx, [esi+4]
  cmp cl, 64
  cmovae ebx, [esi+8]
  mov cl, [esp+16] ; bitofs
  ; shift right into edx:eax
  shrd eax, edx, cl
  shrd edx, ebx, cl

  xor ebx, ebx
  xor cx, cx
  mov cl, [esp+20] ; bits
  cmp cx, 32
  cmovb bx, cx
  shl bx, 2
  and eax, [ebx+bitmasks]

  mov ebx, 1023*4
  sub cx, 32
  cmovns bx, cx
  and edx, [ebx+bitmasks]

  pop ebx
  pop esi

  RET

public_c_symbol vecadd_elemwise
; PROC data1:PTR, len:DWORD, bits:DWORD, wordlen:BYTE
  push esi
  push edi
  push ebp
  push ebx

  ; fill bits...
  mov eax, [esp+28] ; bits
  mov esi, eax
  xor ebx, ebx

  mov cl, 32
  sub cl, [esp+32] ; wordlen
  shld eax, ebx, cl
  mov ebx, eax

  mov cl, [esp+32] ; wordlen
  mov ch, 32
.filling:
  shld esi, ebx, cl
  sub ch, cl
  jge .filling

  ; determine shift
  add ch, cl ; ch = 32%w
  mov al, cl
  sub al, ch
  mov ch, al ; ch = w - 32%w
  mov al, cl
  xor cl, cl ; al = w

  mov edi, [esp+20] ; data1
  ; length in qwords.
  mov ebp, [esp+24] ; len
  shl ebp, 1
  clc
  lahf
.loop:
  mov edx, esi
  shld edx, ebx, cl
  sahf
  adc [edi], edx
  lahf

  ; determine next shift
  add cl, ch
  mov dx, cx
  sub cl, al
  cmovs cx, dx

  add edi, 4
  dec ebp
  jnz .loop

  pop ebx
  pop ebp
  pop edi
  pop esi
  RET

public_c_symbol vecsub_elemwise
; PROC data1:PTR, len:DWORD, bits:DWORD, wordlen:BYTE
  push esi
  push edi
  push ebp
  push ebx

  ; fill bits...
  mov eax, [esp+28] ; bits
  mov esi, eax
  xor ebx, ebx

  mov cl, 32
  sub cl, [esp+32] ; wordlen
  shld eax, ebx, cl
  mov ebx, eax

  mov cl, [esp+32] ; wordlen
  mov ch, 32
.filling:
  shld esi, ebx, cl
  sub ch, cl
  jge .filling

  ; determine shift
  add ch, cl ; ch = 32%w
  mov al, cl
  sub al, ch
  mov ch, al ; ch = w - 32%w
  mov al, cl
  xor cl, cl ; al = w

  mov edi, [esp+20] ; data1
  ; length in qwords.
  mov ebp, [esp+24] ; len
  shl ebp, 1
  clc
  lahf
.loop:
  mov edx, esi
  shld edx, ebx, cl
  sahf
  sbb [edi], edx
  lahf

  ; determine next shift
  add cl, ch
  mov dx, cx
  sub cl, al
  cmovs cx, dx

  add edi, 4
  dec ebp
  jnz .loop

  pop ebx
  pop ebp
  pop edi
  pop esi
  RET

public_c_symbol vecxor_elemwise
; PROC data1:PTR, len:DWORD, bits:DWORD, wordlen:BYTE
  push esi
  push edi
  push ebp
  push ebx
  
  ; fill bits...
  mov eax, [esp+28] ; bits
  mov esi, eax
  xor ebx, ebx

  mov cl, 32
  sub cl, [esp+32] ; wordlen
  shld eax, ebx, cl
  mov ebx, eax

  mov cl, [esp+32] ; wordlen
  mov ch, 32
.filling:
  shld esi, ebx, cl
  sub ch, cl
  jge .filling

  ; determine shift
  add ch, cl ; ch = 32%w
  mov al, cl
  sub al, ch
  mov ch, al ; ch = w - 32%w
  mov al, cl
  xor cl, cl ; al = w

  mov edi, [esp+20] ; data1
  ; length in qwords.
  mov ebp, [esp+24] ; len
  shl ebp, 1
.loop:
  mov edx, esi
  shld edx, ebx, cl
  xor [edi], edx

  ; determine next shift
  add cl, ch
  mov dx, cx
  sub cl, al
  cmovs cx, dx

  add edi, 4
  dec ebp
  jnz .loop

  pop ebx
  pop ebp
  pop edi
  pop esi
  RET

public_c_symbol vecadd
;PROC data1:PTR, data2:PTR, len:DWORD, carry : BYTE
  push esi
  push edi

  mov edi, [esp+12] ; data1
  mov esi, [esp+16] ; data2
  mov ecx, [esp+20] ; len
  shl ecx, 1

  xor edx, edx
  cld
  clc
  lahf
  or ah, [esp+24] ; carry
.loop:
  mov edx, [esi]
  sahf
  adc [edi], edx
  lahf
  add edi, 4
  add esi, 4
  dec ecx
  jnz .loop
  sahf
  setc al
  pop edi
  pop esi
  RET

public_c_symbol vecsub
;PROC data1:PTR, data2:PTR, len:DWORD, carry : BYTE
  push esi
  push edi

  mov edi, [esp+12] ; data1
  mov esi, [esp+16] ; data2
  mov ecx, [esp+20] ; len
  shl ecx, 1

  xor edx, edx
  cld
  clc
  lahf
  or ah, [esp+24] ; carry
.loop:
  mov edx, [esi]
  sahf
  sbb [edi], edx
  lahf
  add edi, 4
  add esi, 4
  dec ecx
  jnz .loop
  sahf
  setc al
  pop edi
  pop esi
  RET

public_c_symbol vecadd_cipr
; PROC M:PTR, L:PTR, len:DWORD, carry : BYTE
  push esi
  push edi
  push ebp
  push ebx

  mov esi, [esp+20] ; M
  mov edi, [esp+24] ; L
  mov ecx, [esp+28] ; len
  shl ecx, 1

  xor edx, edx
  mov al, [esp+32] ; carry
.loop:
  mov ebx, [esi]
  mov ebp, [edi]
  mov edx, ebx
  not edx
  and ebx, ebp	; ebx = U = L & M
  and edx, ebp  ; edx = V = L & ~M
  bt ax, 0
  adc ebp, ebx
  setc al
  or ebp, edx
  mov [edi], ebp
  add edi, 4
  add esi, 4
  dec ecx
  jnz .loop

  pop ebx
  pop ebp
  pop edi
  pop esi
  RET

public_c_symbol vecand
;PROC data1:PTR, data2:PTR, len:DWORD
  push esi
  push edi

  mov edi, [esp+12] ; data1
  mov esi, [esp+16] ; data2
  mov ecx, [esp+20] ; len
  shl ecx, 1

  cld
.loop:
  lodsd
  and eax, dword [edi]
  stosd
  dec ecx
  jnz .loop
  pop edi
  pop esi
  RET

public_c_symbol vecor
;PROC data1:PTR, data2:PTR, len:DWORD
  push esi
  push edi

  mov edi, [esp+12] ; data1
  mov esi, [esp+16] ; data2
  mov ecx, [esp+20] ; len
  shl ecx, 1

  cld
.loop:
  lodsd
  or eax, [edi]
  stosd
  dec ecx
  jnz .loop
  pop edi
  pop esi
  RET

public_c_symbol vecxor
;PROC data1:PTR, data2:PTR, len:DWORD
  push esi
  push edi

  mov edi, [esp+12] ; data1
  mov esi, [esp+16] ; data2
  mov ecx, [esp+20] ; len
  shl ecx, 1

  cld
.loop:
  lodsd
  xor eax, [edi]
  stosd
  dec ecx
  jnz .loop
  pop edi
  pop esi
  RET

public_c_symbol vecshl
; PROC data1:PTR, bits:DWORD, len:DWORD
  push edi
  push ebp
  push ebx
  
  cld
  mov ebx, [esp+24] ; len
  shl ebx, 1 ; len is in qwords
  jz .ldone
  
  mov edi, [esp+16] ; data1
  mov ecx, [esp+20] ; bits
  
  xor eax, eax		; we'll need that later
  xor edx, edx
  cmp ecx, 32
  ja .long_shift
  je .medium_shift	; shift by exactly a dword

.short_shift:
  shr ebx, 1
.sloop:
  mov eax, [edi]	 ; ...next dword
  mov ebp, eax		 ; remember the msbs
  shld eax, edx, cl	 ; shift data
  mov edx, ebp		 ; msbs from this one will go to next one
  stosd				 ; save this one
  mov ebp, [edi]	 ; ...next dword
  mov eax, ebp		 ; remember the msbs
  shld eax, edx, cl	 ; shift data
  mov edx, ebp		 ; msbs from this one will go to next one
  stosd				 ; save this one
  dec ebx			 ; one done
  jnz .sloop

.ldone:
  pop ebx
  pop ebp
  pop edi
  RET

.long_shift: 
  push esi
  
  mov edx, [edi]	; get first dword 
  sub ecx, 32		; shift -32 bits
  stosd				; first one becomes zero
  dec ebx			; is that all?
  jz .ldone
  mov eax, [edi]	; back up second one
  mov ebp, edx	    ; remember briefly
  shl edx, cl		; shift left
  mov [edi], edx	; first one with data
  mov edx, ebp		; restore (need this for msbs)
  add edi, 4
  dec ebx			; is that all?
  jz .lldone  
.lloop:
  mov ebp, [edi]	 ; ...backup next dword
  mov esi, eax
  shld eax, edx, cl	 ; shift data
  stosd				 ; save this one
  mov edx, esi		 ; edx gets old eax value (saved msbs)
  mov eax, ebp		 ; rotate ebp -> eax -> edx => 0 -> eax -> edx

  dec ebx			 ; one done
  jnz .lloop

.lldone:
  pop esi
  pop ebx
  pop ebp
  pop edi
  RET

.medium_shift:
  mov edx, [edi]	; get first dword 
  stosd
  dec ebx
  jz .ldone
.mloop:
  mov eax, [edi]	 ; ...next dword
  mov [edi], edx	 ; shift data by 32 bit
  mov edx, eax		 ; msbs from this one will go to next one
  add edi, 4		 ; increment to next dword
  dec ebx			 ; one done
  jnz .mloop
  
  pop ebx
  pop ebp
  pop edi
  RET

public_c_symbol vecshr
; PROC data1:PTR, bits:DWORD, len:DWORD
  push edi
  push ebx

  cld
  mov ebx, [esp+20] ; len
  dec ebx			; MWW is padded by two QWORDs. We skip one here so we can read ahead a little
  shl ebx, 1
  jz .end

  mov edi, [esp+12] ; data1
  xor ecx, ecx
  or ecx, [esp+16] ; bits
  jz .end
  cmp ecx, 32
  je .medium_shift
  jb .short_shift
.long_shift:

  push ebp  
  
  xor edx, edx
  sub cl, 32	; shorten as first dword is unnecessary
  mov eax, [edi]
  cmp ebx, 1
  cmova  edx, [edi+4]
.lloop:
  xor ebp, ebp
  mov eax, edx
  cmp ebx, 2
  cmova ebp, [edi+8]  
  shrd eax, ebp, cl	; short top half in ebp
  stosd				; and this goes back to memory
  mov edx, ebp		; rotate registers
  dec ebx
  jnz .lloop
  
  pop ebp
  pop ebx
  pop edi
  RET

.medium_shift:
  mov eax, [edi]
.mloop:
  xor edx, edx
  cmp ebx, 1
  cmova  edx, [edi+4]
  mov eax, edx
  stosd
  mov eax, edx
  dec ebx
  jnz .mloop

  pop ebx
  pop edi
  RET

.short_shift:
  mov eax, [edi]
.loop:
  xor edx, edx
  cmp ebx, 1
  cmova  edx, [edi+4]
  shrd eax, edx, cl
  stosd
  mov eax, edx
  dec ebx
  jnz .loop
.end:
  pop ebx
  pop edi
  RET

public_c_symbol insertword
;PROC source:QWORD, target:ptr, bitofs:BYTE, bits:BYTE
;  LOCAL savedbits : DWORD
;  LOCAL dwords_left : BYTE
  push ebp
  mov ebp, esp
  sub esp, 8

  push edi
  push esi
  push ebx
  lea esi, [ebp+8]	; source
  mov edi, [ebp+16] ; target
  mov byte [ebp-4], 2 ; dwords_left

  ; if bitofs > 32 bitofs-= 32 and edi+=4
  mov cl, [ebp+20] ; bitofs
  sub cl, 32
  js .small_ofs
  add edi, 4
  mov [ebp+20], cl	; bitofs
.small_ofs:

  cld
  ; edx holds the next few bits
  mov dword [ebp-8], 0 ; savedbits

  ; get first bit mask: bits bitofs...min(31, bitofs+bits) = 1
  movzx ecx, byte [ebp+20] ; bitofs
  mov ebx, ecx
  shl bx, 5
  xor ax, ax
  mov al, cl
  add al, [ebp+24]	; bits
  xor dx, dx
  cmp ax, 32
  cmovae ax, dx
  or bx, ax
  shl bx, 2
  mov eax, [ebx+bitmasks]
  mov ebx, eax
  mov ch, cl

.dword_loop:
  xor eax, eax
  dec byte [ebp-4] ; dwords_left
  cmovns eax, [esi]
  add esi, 4
  ; shift left bitofs values
  xor edx, edx
  shld edx, eax, cl
  ; save excess bits
  xchg edx, [ebp-8]
  ; put in saved bits from last time
  shl eax, cl
  or eax, edx
  and eax, ebx
  not ebx
  mov edx, [edi]
  and edx, ebx
  or eax, edx
  stosd

  ; compute how many bits we have left
  mov bl, 32
  sub bl, ch
  sub [ebp+24], bl	; bits
  jle .done
  mov ch, 0

  ; get next bit mask: bits 0...min(31, bitofs+bits remaining) = 1
  xor ebx, ebx
  xor ax, ax
  mov al, [ebp+24] ; bits
  cmp ax, 32
  cmovb bx, ax
  shl bx, 2
  mov eax, [ebx+bitmasks]
  mov ebx, eax
  jmp short .dword_loop
.done:
  pop ebx
  pop esi
  pop edi
  
  mov esp, ebp
  pop ebp
  RET

public_c_symbol countbits
; PROC source:PTR, len:DWORD
  push esi
  push edi
  push ebx 
  
  mov esi, [esp+16] ; source
  xor edi, edi
  xor eax, eax
  xor ebx, ebx
  xor edx, edx
  mov ecx, [esp+20]
  ; len is in QWORDs, we want WORDs
  shl ecx, 2
  jz .finished
.loop:
  mov bx, word [esi]
  mov dl, [ebx+bitcount]
  add eax, edx
  adc edi, 0
  add esi, 2
  dec ecx
  jnz .loop
.finished:
  mov edx, edi

  pop ebx
  pop edi
  pop esi
  RET

public_c_symbol incs_8
; PROC source: BYTE
  movzx eax, byte [esp+4]
  inc ax
  or dx, 0ffh
  bt ax, 8
  cmovc ax, dx
  RET

public_c_symbol incs_16
; PROC source: WORD
  movzx eax, word [esp+4]
  inc eax
  or dx, 0ffffh
  bt eax, 16
  cmovc eax, edx
  RET

public_c_symbol incs_32
; PROC source: WORD
  mov eax, [esp+4]
  or edx, 0ffffffffh
  add eax, 1
  cmovc eax, edx
  RET
 
; reset the bits after the end of a vector
public_c_symbol fixending_64
; PROC data:ptr, value_bits:DWORD, vwords:DWORD, datalen:DWORD
  push edi
  push ebx

  mov edi, [esp+12]   ; data
  mov eax, [esp+16]  ; value_bits
  mov ebx, [esp+20]  ; vwords

  mul ebx		; total number of bits -> eax
  test ebx, ebx
  jz .only_rest ; if the total number of bits is zero: only fill the rest
  
  mov ebx, eax	; save total number of bits
  shr eax, 3	; bits -> bytes
  and al, 0f8h	; align 8
  add edi, eax	; go there.
  and ebx, 03fh	; remaining bits -> bitmask
  mov edx, dword [8*ebx + bitmasks_64]
  mov ecx, dword [8*ebx + bitmasks_64 + 4] ; mask for last qword

  ; last relevant qword    
  and [edi], edx		; and the last relevant qword with mask
  and [edi+4], ecx		; ... second half
  add edi, 8			; next step
  
.only_rest:
  ; fill the rest with zeros
  mov ecx, [esp+24]		; datasize
  shr eax, 3			; eax has number of bytes -> convert to qwords
  inc eax				; we have done one using the bit mask
  sub ecx, eax			; data length - qwords done = qwords to do
  jbe .end				; 
  shl ecx, 1			; qwords -> dwords
  xor eax, eax
  cld
  repne stosd
.end:
  pop ebx
  pop edi
  RET
