; MollenOS
; Copyright 2011-2016, Philip Meulengracht
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation?, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
;
; x86 framebuffer present methods

bits 64
segment .text

;Functions in this asm
global present_basic
global present_sse
global present_sse2

present_basic:
	ret

present_sse:
	ret

; present_sse2(void *Framebuffer, void *Backbuffer, int Rows, int RowLoops, int RowRemaining, int LeftoverBytes)
; Copies data <Rows> times from _Backbuffer to Framebuffer
; @abi set to ms
; rcx => Framebuffer
; rdx => Backbuffer
; r8  => Rows
; r9  => RowLoops
; r10 => RowRemaining
; r11 => LeftoverBytes
present_sse2:
	; Store space for XMM6 and XMM7, they are considered non-volatile in
	; ms abi
	sub     rsp, 32
    movdqu [rsp], xmm6
    movdqu [rsp + 16], xmm7

    ; Store state
    push    rdi
    push    rsi
    push    rbx

	; Get destination/source
	mov		rdi, rcx
	mov		rsi, rdx

	; get loop counters
	mov		rbx, r8
	mov		rcx, r9
	mov		rax, r10
	mov		rdx, r11

    ; Iterate for rbx (Rows) times
    .NextRow:

        ; Iterate for rcx (RowLoops) times
        push    rcx
        push    rdi
        .NextCopy:
            movdqa xmm0, [rsi]
            movdqa xmm1, [rsi + 16]
            movdqa xmm2, [rsi + 32]
            movdqa xmm3, [rsi + 48]
            movdqa xmm4, [rsi + 64]
            movdqa xmm5, [rsi + 80]
            movdqa xmm6, [rsi + 96]
            movdqa xmm7, [rsi + 112]

            movntdq [rdi], xmm0
            movntdq [rdi + 16], xmm1
            movntdq [rdi + 32], xmm2
            movntdq [rdi + 48], xmm3
            movntdq [rdi + 64], xmm4
            movntdq [rdi + 80], xmm5
            movntdq [rdi + 96], xmm6
            movntdq [rdi + 112], xmm7
            add		rsi, 128
            add		rdi, 128
            dec		rcx
            jnz     .NextCopy
        
        ; Copy remainder bytes
        mov     rcx, rax
        rep     movsb
        pop     rdi
        pop     rcx
        add     rdi, rdx

        ; Loop Epilogue
        dec     rbx
        jnz     .NextRow
    
    ; Store state
    pop     rbx
    pop     rsi
    pop     rdi
    
	; Restore the saved registers
    movdqu  xmm6, [rsp]
    movdqu  xmm7, [rsp + 16]
	add     rsp, 32
	ret
