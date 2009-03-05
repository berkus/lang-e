mov eax, ebx
mov dword ptr [ ebp + imm8 11110000 ], ebp
mov dword ptr [ ebp + imm8 11110000 ], imm32 11111111111111111111111111111111
mov eax, dword ptr [ ebp + imm8 11110000 ]
mov ecx, dword ptr [ ebp + imm8 11110000 ]
mov dword ptr [ ebx + ecx ], eax
mov ebx, imm32 11111111111111111111111111111111
add dword ptr [ ebp + imm8 11110000 ], imm32 11111111111111111111111111111111
