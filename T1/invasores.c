#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int pontos, tiros, arma, acabou;
    char ataques[10];
} Estado;

char le_tecla(void)
{
    char tecla;

    fflush(stdout);
    return fread(&tecla, 1, 1, stdin) == 1 ? tecla : 0;
}

void move_ataques(Estado *jogo)
{
    int i;

    for (i = 0; i < 9; i++) {
        jogo->ataques[i] = jogo->ataques[i + 1];
    }
    jogo->ataques[9] = "0123456789"[rand() % 10];
}

void desenha(Estado *jogo)
{
    int i;

    printf("%3d %2d %c)))", jogo->pontos, jogo->tiros,
           "0123456789n"[jogo->arma]);
    for (i = 0; i < 10; i++) {
        printf("%c", jogo->ataques[i]);
    }
    printf("\r");
}

int main(void)
{
    Estado jogo = {0, 30, 0, 0, {' ', ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', ' '}};
    struct timespec inicio, agora;

    srand(time(NULL));
    system("stty raw -echo min 0 time 1 opost");
    setvbuf(stdin, NULL, _IONBF, 0);
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    while (!jogo.acabou) {
        char tecla = le_tecla();

        if (tecla == 27) {
            jogo.acabou = 1;
        } else if (tecla == '\t') {
            jogo.arma = (jogo.arma + 1) % 11;
        } else if ((tecla == '\r' || tecla == '\n') && jogo.tiros > 0) {
            jogo.tiros--;
        }
        clock_gettime(CLOCK_MONOTONIC, &agora);
        if (agora.tv_sec - inicio.tv_sec >= 2) {
            move_ataques(&jogo);
            inicio = agora;
        }
        desenha(&jogo);
    }
    system("stty sane");
    return 0;
}
