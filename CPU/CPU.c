#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 65536   // 64 KB
#define STK_SIZE   16
#define STK_START  0x8200

// Estrutura da CPU (registradores e flags)
typedef struct {
    uint16_t registradores[8];  // R0-R7
    uint16_t pc;                // Contador de Programa
    uint16_t sp;                // Ponteiro de Pilha
    uint16_t ir;                // Registrador de Instrução
    uint8_t flags;             // Flags (C, Ov, Z, S)
} CPU;

// Definições das Flags
#define FLAG_C   0x1  // Carry
#define FLAG_OV  0x2  // Overflow
#define FLAG_Z   0x4  // Zero
#define FLAG_S   0x8  // Sinal


// Memórias
uint8_t memoriaDePrograma[MEM_SIZE] = {0};  // Instruções
uint8_t memoriaDeDados[MEM_SIZE] = {0};     // Dados
uint8_t pilha[STK_SIZE] = {0};               // Pilha (full descending)
CPU cpu = {{0}, 0, STK_START, 0, 0};         // Inicialização da CPU


// ========== FUNÇÕES PRINCIPAIS ========== //

// Carrega o programa na memória
void carregar_programa(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo!\n");
        exit(1);
    }

    char linha[100];
    uint16_t endereco, instrucao;

    while (fgets(linha, sizeof(linha), arquivo)) {
        if (sscanf(linha, "%4hx: 0x%4hx", &endereco, &instrucao) == 2) {
            memoriaDePrograma[endereco] = instrucao & 0xFF;      // Byte baixo
            memoriaDePrograma[endereco + 1] = instrucao >> 8;    // Byte alto
        }
    }

    fclose(arquivo);
}


void definir_flags(int resultado, int carry, int overflow) {
    cpu.flags = 0;
    if (resultado == 0) cpu.flags |= FLAG_Z;
    if (resultado < 0) cpu.flags |= FLAG_S;
    if (carry) cpu.flags |= FLAG_C;
    if (overflow) cpu.flags |= FLAG_OV;
}

void imprimir_estado() {
    printf("\n=== Estado da CPU ===\n");
    for (int i = 0; i < 8; i++) {
        printf("R[%d]: 0x%04X\n", i, cpu.registradores[i]);
    }
    printf("PC: 0x%04X | SP: 0x%04X | IR: 0x%04X\n", cpu.pc, cpu.sp, cpu.ir);
    printf("Flags: [C:%d Ov:%d Z:%d S:%d]\n",
           (cpu.flags & FLAG_C) ? 1 : 0,
           (cpu.flags & FLAG_OV) ? 1 : 0,
           (cpu.flags & FLAG_Z) ? 1 : 0,
           (cpu.flags & FLAG_S) ? 1 : 0);


    printf("\n=== Pilha ===\n");
    for (int i = 0; i < STK_SIZE; i += 2) {
        uint16_t endereco = cpu.sp - i;
        uint16_t valor = pilha[i] | (pilha[i + 1] << 8);
        printf("0x%04X: 0x%04X\n", endereco, valor);
    }
    

}

// Executa uma instrução
void executar_instrucao() {
    while (1) {
        // Busca a instrução (16 bits)
        cpu.ir = memoriaDePrograma[cpu.pc] | (memoriaDePrograma[cpu.pc + 1] << 8);
        cpu.pc += 2;

        uint8_t opcode = (cpu.ir >> 12) & 0x0F;  // OpCode (bits 15-12)
        uint8_t is_imediato = (cpu.ir >> 11) & 0x01;  // Bit 11 (0 = registrador, 1 = imediato)
        uint8_t rd = (cpu.ir >> 8) & 0x07;       // Registrador destino (bits 10-8)
        uint8_t rn = (cpu.ir >> 5) & 0x07;       // Registrador origem (bits 7-5)
        uint8_t rm = cpu.ir & 0x07;              // Outro registrador (bits 2-0)
        uint8_t bits1_0 = cpu.ir & 0x03;         // Últimos 2 bits
        uint16_t valor_imediato = cpu.ir & 0xFF;       // Valor imediato (8 bits)

        //halt
        if (cpu.ir == 0xFFFF){
            break;
        }
        //nop
        if (cpu.ir == 0x0000){
            imprimir_estado();
        }

        switch (opcode){
            //mov
            case 0b0001: {
                if (is_imediato == 0){
                    cpu.registradores[rd] = cpu.registradores[rm];
                }
                else{
                    cpu.registradores[rd] = valor_imediato;
                }
                break;
            }
            //str
            case 0b0010: {
                if (is_imediato == 0){
                    memoriaDeDados[cpu.registradores[rn]] = cpu.registradores[rd];
                }
                else {
                    cpu.registradores[rn] = valor_imediato;
                }
                
                break;
            }
            //ldr
            case 0b0011: {
                cpu.registradores[rd] = memoriaDeDados[cpu.registradores[rm]];
                break;
            }
            //add
            case 0b0100: {
                uint16_t resultado = cpu.registradores[rn] + cpu.registradores[rm];  
                int carry = cpu.registradores[rd] < cpu.registradores[rn]; 
                int overflow = (cpu.registradores[rn] > 0 && cpu.registradores[rm] > 0 && resultado < 0) || (cpu.registradores[rn] < 0 && cpu.registradores[rm] < 0 && resultado >= 0);
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, carry, overflow); 
                break;
            }
            //sub
            case 0b0101: {
                uint16_t resultado = cpu.registradores[rn] - cpu.registradores[rm];  
                int carry = cpu.registradores[rn] < cpu.registradores[rm]; 
                int overflow = (cpu.registradores[rn] >= 0 && cpu.registradores[rm] < 0 && resultado < 0) || (cpu.registradores[rn] < 0 && cpu.registradores[rm] >= 0 && resultado >= 0);
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, carry, overflow); 
                break;
            }
            //mul
            case 0b0110: {
                uint16_t resultado = cpu.registradores[rn] * cpu.registradores[rm];
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0);
                break;
            }
            //and
            case 0b0111: {
                uint16_t resultado = cpu.registradores[rn] & cpu.registradores[rm];
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0);
                break;
            }
            //orr
            case 0b1000: {
                uint16_t resultado = cpu.registradores[rn] | cpu.registradores[rm];
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0);
                break;
            }
            //not
            case 0b1001: {
                uint16_t resultado = ~cpu.registradores[rn];
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0);
                break;
            }
            //xor
            case 0b1010: {
                uint16_t resultado = cpu.registradores[rn] ^ cpu.registradores[rm];
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0);
                break;
            }
            //shr
            case 0b1011: {
                uint16_t resultado = cpu.registradores[rn] >> 1; 
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0); 
                break;
            }
            //shl
            case 0b1100: {
                uint16_t resultado = cpu.registradores[rn] << 1; 
                cpu.registradores[rd] = resultado;
                definir_flags(resultado, 0, 0);
                break;
            }
            //ror
            case 0b1101: {
                uint16_t resultado = (cpu.registradores[rn] << 1) | (cpu.registradores[rn] >> 15); // mudar isso dps
                cpu.registradores[rd] = resultado;

                break;
            }
            //rol
            case 0b1110: {
                uint16_t resultado = (cpu.registradores[rn] << 1) | (cpu.registradores[rn] >> 15);
                cpu.registradores[rd] = resultado;
                break;
            }
            default:
                break;
        }
        //cmp
        if (opcode == 0x0000 && bits1_0 == 0x0003){
            int resultado = cpu.registradores[rn] - cpu.registradores[rm];
            int carry = (cpu.registradores[rn] < cpu.registradores[rm]);
            definir_flags(resultado, carry, 0);
        }
        //desvio (jumps)
        if (opcode == 0b0000 && is_imediato == 1){
            uint16_t imediato = (cpu.ir & 0x01FF) >> 2;
            uint8_t tipoJump = (cpu.ir & 0x0003);

            //jmp
            if (tipoJump == 0x0){
                cpu.pc += imediato;
            }

            else if (tipoJump == 0x1){
               if ((cpu.flags & FLAG_Z) && !(cpu.flags & FLAG_S)){
                   cpu.pc += imediato;
               }
            }

            else if (tipoJump == 0x2){
                if (!(cpu.flags & FLAG_Z) && (cpu.flags & FLAG_S)){
                    cpu.pc += imediato;
                }
            }

            else if (tipoJump == 0x3){
                if (!(cpu.flags & FLAG_Z) && !(cpu.flags & FLAG_S)){
                    cpu.pc += imediato;
                }
            }
        }
        //pilha
        // psh (push)
        if (opcode == 0b0000 && is_imediato == 0 && (cpu.ir & 0x03) == 0b01) {
            uint16_t valor = cpu.registradores[rn];
            pilha[cpu.sp - STK_START] = valor & 0xFF;         
            pilha[cpu.sp - STK_START + 1] = (valor >> 8) & 0xFF; 
            cpu.sp -= 2;  
        }
        // pop
        else if (opcode == 0b0000 && is_imediato == 0 && (cpu.ir & 0x03) == 0b10) {
            cpu.sp += 2;
            cpu.registradores[rd] = pilha[cpu.sp - STK_START] | (pilha[cpu.sp - STK_START + 1] << 8);
        }

    }
}


int main() {
    carregar_programa("instrucoes.txt");
    printf("Iniciando execução...\n");
    executar_instrucao();
    imprimir_estado();
    return 0;
}