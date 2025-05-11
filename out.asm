section .text
    global main
    extern ExitProcess

main:
    mov rax, 4
    push rax
    mov rax, 3
    push rax
    push QWORD [rsp + 0]
    push QWORD [rsp + 16]
    pop rax
    pop rbx
    cmp rax, rbx
    sete al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jz label0
    push QWORD [rsp + 0]
    pop rcx
    call ExitProcess
    add rsp, 0
    jmp label1
label0:
    push QWORD [rsp + 8]
    pop rcx
    call ExitProcess
    add rsp, 0
label1:
    mov rcx, 0
    call ExitProcess
