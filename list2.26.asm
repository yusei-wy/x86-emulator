start:
    mov si, msg
    call puts
fin:
    hlt
    jmp fin

puts:
    mov al, [si]
    inc si
    cmp al, 0
    je puts_end
    mov ah, 0x0e
    mov bx, 15
    int 0x10
    jmp puts
puts_end:
    ret

msg:
    db "hello, wolrd", 0x0d, 0x0a, 0
