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

void executarInstrucao(){};
int main() {};
