#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 65536  // 64 KB de memória
#define STK_SIZE 16
#define STK_START 0x8200

// Registradores
uint16_t R[8] = {0};   // R0-R7
uint16_t PC = 0;       // Contador de programa
uint16_t IR = 0;       // Registrador de instrução
uint16_t SP = 0x8200;  // Stack Pointer inicial
uint16_t FLAGS = 0;    // Flags (C, Ov, Z, S)

// Memória simulada
uint16_t memoriaDePrograma[MEM_SIZE] = {0}; // Memória de programa
uint16_t memoriaDeDados[MEM_SIZE] = {0};           // Memória de dados
uint8_t pilha[STK_SIZE] = {0};                    // Pilha

// Flags
#define FLAG_C 0x1  // Carry
#define FLAG_Ov 0x2 // Overflow
#define FLAG_Z 0x4  // Zero
#define FLAG_S 0x8  // Signal

//setar flags
void setFlags(int resultado, int carry, int overflow) {
    FLAGS = 0;
    if (resultado == 0) FLAGS |= FLAG_Z;
    if (resultado < 0) FLAGS |= FLAG_S;
    if (carry) FLAGS |= FLAG_C;
    if (overflow) FLAGS |= FLAG_Ov;

}

//Função pra printar o estado dos registradores e etc da Cpu
void printEstado(){
    printf("Registradores:\n");
    for (int i = 0; i < 8; i++) {
        printf("R%d: 0x%04X\n", i, R[i]);
    }
    printf("PC: 0x%04X\n", PC);
    printf("IR: 0x%04X\n", IR);
    printf("SP: 0x%08X\n", SP);

    printf("\nFlags:\n");
    printf("C: %d, Ov: %d, Z: %d, S: %d\n",
           (FLAGS & FLAG_C) ? 1 : 0,
           (FLAGS & FLAG_Ov) ? 1 : 0,
           (FLAGS & FLAG_Z) ? 1 : 0,
           (FLAGS & FLAG_S) ? 1 : 0);

    printf("\nMemória de dados acessada:\n");
    for (int i = 0; i < MEM_SIZE; i++) {
        if (memoriaDeDados[i] != 0) {
            printf("0x%04X: 0x%04X\n", i, memoriaDeDados[i]);
        }
    }

    printf("\nPilha:\n");
    for (int i = 0; i < STK_SIZE; i++) {
        printf("0x%08X: 0x%02X\n", STK_START + i, pilha[i]);
    }
}

void carregarPrograma(const char *nomeArquivo){
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }

    char linha[100];
    
    while (fgets(linha, sizeof(linha), arquivo)) {
        unsigned int endereco, conteudo;
        if (sscanf(linha, "%4hx: 0x%4hx", &endereco, &conteudo) == 2){
            memoriaDePrograma[endereco] = conteudo & 0xFF;
            memoriaDePrograma[endereco + 1] = (conteudo >> 8) & 0xFF;
        }
    }
    fclose(arquivo);
}

void executarInstrucao(){
    while(1){
        // Busca a instrução (16 bits)
        uint16_t instrucao = (memoriaDePrograma[PC] << 8) | memoriaDePrograma[PC + 1];
        PC += 2;

        // Decodificação da instrução
        uint8_t opcode = (instrucao >> 12) & 0x0F;  // OpCode (bits 15-12)
        uint8_t is_imediato = (instrucao >> 11) & 0x01;  // Bit 11 (0 = registrador, 1 = imediato)
        uint8_t rd = (instrucao >> 8) & 0x07;       // Registrador destino (bits 10-8)
        uint8_t rn = (instrucao >> 5) & 0x07;       // Registrador origem (bits 7-5)
        uint8_t rm = instrucao & 0x07;              // Outro registrador (bits 2-0)
        uint8_t lstBits = instrucao & 0x03;         // Últimos 2 bits
        uint16_t imediato = instrucao & 0xFF;       // Valor imediato (8 bits)
        //mov
        if (opcode == 0b0001){
            if (is_imediato == 0){
                R[rd] = R[rm];
            }
            else if (is_imediato == 1){
                R[rd] = imediato;
            }
        }
        //str
        if (opcode == 0b0010){
            if (is_imediato == 0){
                memoriaDeDados[R[rn]] = R[rd];
            }
            else if(is_imediato == 1){
                R[rn] = imediato;
            }
        }
        //ldr
        if (opcode == 0b0011){
            R[rd] = memoriaDeDados[R[rm]];
        }
        //add
        if (opcode == 0b0100){
            uint16_t resultado = R[rn] + R[rm];  
            int carry = R[rd] < R[rn]; 
            int overflow = (R[rn] > 0 && R[rm] > 0 && resultado < 0) || (R[rn] < 0 && R[rm] < 0 && resultado >= 0);
            R[rd] = resultado;
            setFlags(resultado, carry, overflow); 
        }
        //sub
        if (opcode == 0b0101){
            uint16_t resultado = R[rn] - R[rm];
            int carry = (R[rn] < R[rm]); 
            int overflow = (R[rn] >= 0 && R[rm] < 0 && resultado < 0) || (R[rn] < 0 && R[rm] >= 0 && resultado >= 0);
            R[rd] = resultado;
            setFlags(resultado, carry, overflow);
        }
        //mul
        if (opcode == 0b0110){
            uint16_t resultado = R[rn] * R[rm];
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        // and
        if (opcode == 0b0111){
            uint16_t resultado = R[rn] & R[rm];
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        // or
        if (opcode == 0b1000){
            uint16_t resultado = R[rn] | R[rm];
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        // not
        if (opcode == 0b1001){
            uint16_t resultado = ~R[rm];
            R[rd] = resultado;
            setFlags(resultado, 0, 0);

        }
        // xor
        if (opcode == 0b1010){
            uint16_t resultado = R[rn] ^ R[rm];
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        //psh
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b01){
            if (SP > STK_START) {
                pilha[--SP] = R[rm];
            } else {
                printf("Erro: Estouro de pilha!\n");
                exit(1);
            }
        }
        //pop
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b10){
            if (SP < STK_START + STK_SIZE) {
                R[rm] = pilha[SP++];
            } else {
                printf("Erro: Underflow de pilha!\n");
                exit(1);
            }
        }
        //cmp
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b11){
            int resultado = R[rn] - R[rm];
            int carry = (R[rn] < R[rm]);
            setFlags(resultado, carry, 0);
        }
        // jmp
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b00){
            PC += imediato;
        }
        // jeq
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b01){
            if ((FLAGS & FLAG_Z) && !(FLAGS & FLAG_C)) {
                PC += imediato;
            }
        }
        // jlt
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b10){
            if (!(FLAGS & FLAG_Z) && (FLAGS & FLAG_C)) {
                PC += (instrucao & 0xFF);
            }
        }
        // jgt
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b11){
            if ((FLAGS & FLAG_Z) && !(FLAGS & FLAG_C)) {
                PC += (instrucao & 0xFF);
            }
        }
        // shr
        if (opcode == 0b1011){
            uint16_t resultado = R[rn] >> 1; 
            R[rd] = resultado;
            setFlags(resultado, 0, 0); 
        }
        // shl
        if (opcode == 0b1100){
            uint16_t resultado = R[rn] << 1;
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        // ror
        if (opcode == 0b1101){
            uint16_t resultado = R[rn] >> 1 | R[rn] << 15;
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        // rol
        if (opcode == 0b1110){
            uint16_t resultado = R[rn] << 1 | R[rn] >> 15;
            R[rd] = resultado;
            setFlags(resultado, 0, 0);
        }
        //nop
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b00){
            printEstado();
        }
        //halt
        if (opcode == 0b1111 && is_imediato == 1 && lstBits == 0b11){
            //printar e encerrar
            printf("Execução terminada (HALT)\n");
            printEstado();
            return;
        }
    }
}
    int main() {
        carregarPrograma("programa.txt");
        printf("Iniciando execução...\n");
        executarInstrucao();
        return 0;
    }

