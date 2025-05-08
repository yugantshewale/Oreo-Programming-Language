section .text
    global main
    extern ExitProcess

main:
    mov rax, 69
    push rax
    mov rax, 0
    push rax
    pop rax
    test rax, rax
    jz label0
    mov rax, 45
    push rax
    pop rcx
    call ExitProcess
    add rsp, 0
label0:
    mov rax, 0
    push rax
    pop rax
    test rax, rax
    jz label2
    mov rax, 46
    push rax
    pop rcx
    call ExitProcess
    add rsp, 0
    jmp label1
label2:
    mov rax, 3
    push rax
    pop rcx
    call ExitProcess
    add rsp, 0
label1:
    mov rax, 3
    push rax
    push QWORD [rsp + 8]
    pop rcx
    call ExitProcess
    mov rcx, 0
    call ExitProcess
