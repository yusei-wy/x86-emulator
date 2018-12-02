BITS 32
    org 0x7c00
start:              ; プログラムの開始
    mov esi, msg
    call puts       ; サブルーチンを呼び出す
    jmp 0

puts:
    mov al, [esi]   ; 1文字読み込む
    inc esi
    cmp al, 0       ; 文字列の終端
    je  puts_end    ; にきたら終了
    mov ah, 0x0e
    mov ebx, 10
    int 0x10        ; BIOS 呼び出し
    jmp puts
puts_end:
    ret             ; サブルーチンから抜ける

msg:
    db "hello, world", 0x0d, 0x0a, 0
