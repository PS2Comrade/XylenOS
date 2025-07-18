[org 0x7c00]
KERNEL_OFFSET EQU 0x1000

BOOT_DRIVE db 0x0

MSG_REAL_MODE: db "started in 16-bit real mode", 0xa, 0xd, 0x0
MSG_LOAD_KERNEL: db "loading kernel into memory...", 0x0
MSG_PROT_MODE: db "successfully landed in 32-bit protected mode.", 0x0

mov [BOOT_DRIVE], dl
mov bp, 0x9000
mov sp, bp
mov bx, MSG_REAL_MODE
call print_string
call load_kernel
call switch_to_pm
jmp $

print_string:
  pusha
  mov ah, 0xe
  jmp read_character
read_character:
  mov al, [bx]
  cmp al, 0
  jne print_character
  popa
  ret
print_character:
  int 0x10
  add bx, 1
  jmp read_character

disk_load:
  pusha
  push dx
  mov ah, 0x02
  mov al, dh
  mov ch, 0
  mov dh, 0
  mov cl, 2
  int 0x13
  pop dx
  jc disk_error
  cmp al, dh
  jne disk_error
  popa
  ret
disk_error:
  mov bx, ERROR_MSG
  call print_string
  jmp $
ERROR_MSG: db 'disk error', 0

gdt_start:
gdt_null:
  dd 0x0
  dd 0x0
gdt_code:
  dw 0xffff
  dw 0x0
  db 0x0
  db 10011010b
  db 11001111b
  db 0x0
gdt_data:
  dw 0xffff
  dw 0x0
  db 0x0
  db 10010010b
  db 11001111b
  db 0x0

gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start

CODE_SEG EQU gdt_code - gdt_start
DATA_SEG EQU gdt_data - gdt_start

[bits 16]
switch_to_pm:
  cli
  lgdt [gdt_descriptor]
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  jmp CODE_SEG:init_pm

[bits 32]
init_pm:
  mov ax, DATA_SEG
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ebp, 0x90000
  mov esp, ebp
  call BEGIN_PM

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f
print_string_pm:
  pusha
  mov edx, VIDEO_MEMORY
print_string_pm_loop:
  mov al, [ebx]
  mov ah, WHITE_ON_BLACK
  cmp al, 0
  je print_string_pm_end
  mov [edx], ax
  add ebx, 1
  add edx, 2
  jmp print_string_pm_loop
print_string_pm_end:
  popa
  ret

[bits 16]
load_kernel:
  mov bx, MSG_LOAD_KERNEL
  call print_string
  mov bx, KERNEL_OFFSET
  mov dh, 47
  mov dl, [BOOT_DRIVE]
  call disk_load
  ret

[bits 32]
BEGIN_PM:
  mov ebx, MSG_PROT_MODE
  call print_string_pm
  call KERNEL_OFFSET
  jmp $

times 510-($-$$) db 0x0
dw 0xaa55
