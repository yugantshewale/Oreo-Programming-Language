section .text
    global main
    extern ExitProcess

main:
    mov rax, 69
    push rax
    push QWORD [rsp + 0]
    pop rax
    test rax, rax
    jz label0
    mov rax, 45
    push rax
    pop rcx
    call ExitProcess
    add rsp, 0
label0:
    mov rax, 3
    push rax
    push QWORD [rsp + 8]
    pop rcx
    call ExitProcess
    mov rcx, 0
    call ExitProcess
