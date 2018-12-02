BITS 32
    org 0x7c00
start:
    mov edx, 0x03f8
mainloop:
    mov al, '>'     ; プロンプト表示
    out dx, al
input:
    in al, dx       ; 1文字入力
    cmp al, 'h'     ; h なら hello
    je puthello
    cmp al, 'w'     ; w なら world
    je putworld
    cmp al, 'q'     ; a なら終了
    je fin
    jmp input       ; 再入力
puthello:
    mov esi, msghello
    call puts
    jmp mainloop
putworld:
    mov esi, msgworld
    call puts
    jmp mainloop
fin:
    jmp 0

; esi に設定された文字列を表示するサブルーチン
puts:
    mov al, [esi]
    inc esi
    cmp al, 0
    je putsend
    out dx, al
    jmp puts
putsend:
    ret

msghello:
    db "hello", 0x0d, 0x0a, 0
msgworld:
    db "wolrd", 0x0d, 0x0a, 0
