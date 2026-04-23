global sepia_sse_apply

section .rodata align=16
coef_rr:  dd 0.393, 0.393, 0.393, 0.393
coef_rg:  dd 0.769, 0.769, 0.769, 0.769
coef_rb:  dd 0.189, 0.189, 0.189, 0.189

coef_gr:  dd 0.349, 0.349, 0.349, 0.349
coef_gg:  dd 0.686, 0.686, 0.686, 0.686
coef_gb:  dd 0.168, 0.168, 0.168, 0.168

coef_br:  dd 0.272, 0.272, 0.272, 0.272
coef_bg:  dd 0.534, 0.534, 0.534, 0.534
coef_bb:  dd 0.131, 0.131, 0.131, 0.131

zero_ps:  dd 0.0,   0.0,   0.0,   0.0
max_ps:   dd 255.0, 255.0, 255.0, 255.0
half_ps:  dd 0.5,   0.5,   0.5,   0.5

section .text

; void sepia_sse_apply(const struct pixel *src, struct pixel *dst, uint64_t pixel_count)
; System V ABI:
; rdi = src
; rsi = dst
; rdx = pixel_count   ; guaranteed multiple of 4

sepia_sse_apply:
    test rdx, rdx
    jz .done

    push rbp
    mov rbp, rsp
    sub rsp, 96

.loop4:
    test rdx, rdx
    jz .finish

    ; local layout:
    ; [rsp+00]  b[4] int32
    ; [rsp+16]  g[4] int32
    ; [rsp+32]  r[4] int32
    ; [rsp+48]  out_b[4] int32
    ; [rsp+64]  out_g[4] int32
    ; [rsp+80]  out_r[4] int32

    ; ---- unpack 4 BGR pixels to int32 arrays ----

    ; pixel 0
    movzx eax, byte [rdi]
    mov [rsp+0], eax
    movzx eax, byte [rdi+1]
    mov [rsp+16], eax
    movzx eax, byte [rdi+2]
    mov [rsp+32], eax

    ; pixel 1
    movzx eax, byte [rdi+3]
    mov [rsp+4], eax
    movzx eax, byte [rdi+4]
    mov [rsp+20], eax
    movzx eax, byte [rdi+5]
    mov [rsp+36], eax

    ; pixel 2
    movzx eax, byte [rdi+6]
    mov [rsp+8], eax
    movzx eax, byte [rdi+7]
    mov [rsp+24], eax
    movzx eax, byte [rdi+8]
    mov [rsp+40], eax

    ; pixel 3
    movzx eax, byte [rdi+9]
    mov [rsp+12], eax
    movzx eax, byte [rdi+10]
    mov [rsp+28], eax
    movzx eax, byte [rdi+11]
    mov [rsp+44], eax

    ; ---- load as vectors and convert int -> float ----
    movdqa xmm0, [rsp+0]      ; b
    movdqa xmm1, [rsp+16]     ; g
    movdqa xmm2, [rsp+32]     ; r

    cvtdq2ps xmm0, xmm0
    cvtdq2ps xmm1, xmm1
    cvtdq2ps xmm2, xmm2

    ; ---- new red ----
    movaps xmm3, xmm2
    mulps xmm3, [rel coef_rr]
    movaps xmm4, xmm1
    mulps xmm4, [rel coef_rg]
    addps xmm3, xmm4
    movaps xmm4, xmm0
    mulps xmm4, [rel coef_rb]
    addps xmm3, xmm4

    ; ---- new green ----
    movaps xmm4, xmm2
    mulps xmm4, [rel coef_gr]
    movaps xmm5, xmm1
    mulps xmm5, [rel coef_gg]
    addps xmm4, xmm5
    movaps xmm5, xmm0
    mulps xmm5, [rel coef_gb]
    addps xmm4, xmm5

    ; ---- new blue ----
    movaps xmm5, xmm2
    mulps xmm5, [rel coef_br]
    movaps xmm6, xmm1
    mulps xmm6, [rel coef_bg]
    addps xmm5, xmm6
    movaps xmm6, xmm0
    mulps xmm6, [rel coef_bb]
    addps xmm5, xmm6

    ; ---- clamp to [0,255] ----
    maxps xmm3, [rel zero_ps]
    minps xmm3, [rel max_ps]

    maxps xmm4, [rel zero_ps]
    minps xmm4, [rel max_ps]

    maxps xmm5, [rel zero_ps]
    minps xmm5, [rel max_ps]

    ; ---- round by adding 0.5 and truncating ----
    addps xmm3, [rel half_ps]
    addps xmm4, [rel half_ps]
    addps xmm5, [rel half_ps]

    cvttps2dq xmm3, xmm3
    cvttps2dq xmm4, xmm4
    cvttps2dq xmm5, xmm5

    movdqa [rsp+80], xmm3     ; out_r
    movdqa [rsp+64], xmm4     ; out_g
    movdqa [rsp+48], xmm5     ; out_b

    ; ---- repack to dst as BGR ----

    ; pixel 0
    mov eax, [rsp+48]
    mov [rsi], al
    mov eax, [rsp+64]
    mov [rsi+1], al
    mov eax, [rsp+80]
    mov [rsi+2], al

    ; pixel 1
    mov eax, [rsp+52]
    mov [rsi+3], al
    mov eax, [rsp+68]
    mov [rsi+4], al
    mov eax, [rsp+84]
    mov [rsi+5], al

    ; pixel 2
    mov eax, [rsp+56]
    mov [rsi+6], al
    mov eax, [rsp+72]
    mov [rsi+7], al
    mov eax, [rsp+88]
    mov [rsi+8], al

    ; pixel 3
    mov eax, [rsp+60]
    mov [rsi+9], al
    mov eax, [rsp+76]
    mov [rsi+10], al
    mov eax, [rsp+92]
    mov [rsi+11], al

    add rdi, 12
    add rsi, 12
    sub rdx, 4
    jmp .loop4

.finish:
    add rsp, 96
    pop rbp

.done:
    ret