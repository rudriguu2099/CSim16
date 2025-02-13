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
    //simploficar essa leitura depois!!
    while (fgets(linha, sizeof(linha), arquivo)) {
        unsigned int endereco, conteudo;
        sscanf(linha, "%x:%x", &endereco, &conteudo);
        memoriaDePrograma[endereco] = (uint16_t)conteudo;
    }
    fclose(arquivo);
}

void executarInstrucao(){
    while(1){
        uint16_t instrucao = memoriaDePrograma[PC];  // Buscar instrução
        PC += 2;  // Incrementa PC

        uint8_t opcode = (instrucao >> 12) & 0x0F;  // OpCode (bits 15-12)
        uint8_t is_imediato = (instrucao >> 11) & 0x01;  // Bit 11 (0 = registrador, 1 = imediato)
        uint8_t rd = (instrucao >> 8) & 0x07;       // Registrador destino (bits 10-8)
        uint8_t rn = (instrucao >> 5) & 0x07;       // Registrador origem (bits 7-5)
        uint8_t rm = instrucao & 0x07;              // Outro registrador (bits 2-0)
        uint8_t lstBits = instrucao & 0x03;         // ultimos 2 bits
        //mov
        if (opcode == 0b0001){
            if (is_imediato == 0){
                R[rd] = R[rm];
            }
            else if (is_imediato == 1){
                R[rd] = instrucao & 0xFF;
            }
        }
        //str
        if (opcode == 0b0010){
            if (is_imediato == 0){

            }
            else if(is_imediato == 1){

            }
        }
        //ldr
        if (opcode == 0b0011){
            
        }
        //add
        if (opcode == 0b0100){

        }
        //sub
        if (opcode == 0b0101){

        }
        //mul
        if (opcode == 0b0110){

        }
        // and
        if (opcode == 0b0111){

        }
        // or
        if (opcode == 0b1000){

        }
        // not
        if (opcode == 0b1001){

        }
        // xor
        if (opcode == 0b1010){

        }
        //psh
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b01){

        }
        //pop
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b10){

        }
        //cmp
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b11){

        }
        // jmp
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b00){

        }
        // jeq
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b01){

        }
        // jlt
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b10){

        }
        // jgt
        if (opcode == 0b0000 && is_imediato == 1 && lstBits == 0b11){

        }
        // shr
        if (opcode == 0b1011){

        }
        // shl
        if (opcode == 0b1100){

        }
        // ror
        if (opcode == 0b1101){

        }
        // rol
        if (opcode == 0b1110){

        }
        //nop
        if (opcode == 0b0000 && is_imediato == 0 && lstBits == 0b00){
            //printar
        }
        //halt
        if (opcode == 0b1111 && is_imediato == 1 && lstBits == 0b11){
            //printar e encerrar
            printf("Execução terminada (HALT)\n");
            return;
        }

    }
}
int main() {}
