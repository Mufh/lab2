#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POSICOES 10
typedef struct {
    int pontos, tiros, arma, escudos, restantes, acabou;
    char ataques[POSICOES];
} Estado;

char le_tecla(void)
{
    char tecla;

    fflush(stdout);
    return fread(&tecla, 1, 1, stdin) == 1 ? tecla : 0;
}

void limpa(Estado *jogo)
{
    int i;

    for (i = 0; i < POSICOES; i++) {
        jogo->ataques[i] = ' ';
    }
}

void desloca(Estado *jogo)
{
    int i;

    if (jogo->ataques[0] != ' ') {
        if (jogo->escudos > 0) {
            jogo->escudos--;
        } else {
            jogo->acabou = 1;
        }
    }
    for (i = 0; i < POSICOES - 1; i++) {
        jogo->ataques[i] = jogo->ataques[i + 1];
    }
    jogo->ataques[POSICOES - 1] = ' ';
    if (jogo->restantes-- > 0) {
        jogo->ataques[POSICOES - 1] = rand() % 11 == 10 ? 'N' :
                                           "0123456789"[rand() % 10];
    }
}

void atira(Estado *jogo)
{
    int i;
    char arma = "0123456789n"[jogo->arma];

    if (jogo->tiros <= 0) {
        return;
    }
    jogo->tiros--;
    for (i = 0; i < POSICOES; i++) {
        if (jogo->ataques[i] == arma ||
            (arma == 'n' && jogo->ataques[i] == 'N')) {
            if (jogo->ataques[i] == 'N') {
                jogo->ataques[i] = 'n';
            } else {
                jogo->pontos += POSICOES - i;
                if (arma == 'n') {
                    jogo->pontos += POSICOES - i;
                }
                jogo->ataques[i] = ' ';
            }
            return;
        }
    }
}

void desenha(Estado *jogo)
{
    int i;

    printf("%3d %2d %c", jogo->pontos, jogo->tiros,
           "0123456789n"[jogo->arma]);
    for (i = 0; i < 3; i++) {
        printf("%c", i < jogo->escudos ? ')' : ' ');
    }
    for (i = 0; i < POSICOES; i++) {
        printf("%c", jogo->ataques[i]);
    }
    printf("   \r");
}

int main(void)
{
    Estado jogo = {0, 30, 0, 3, 20, 0};
    struct timespec inicio, agora;
    char tecla;

    limpa(&jogo);
    srand(time(NULL));
    system("stty raw -echo min 0 time 1 opost");
    setvbuf(stdin, NULL, _IONBF, 0);
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    while (!jogo.acabou) {
        tecla = le_tecla();
        if (tecla == 27) {
            jogo.acabou = 1;
        } else if (tecla == '\t') {
            jogo.arma = (jogo.arma + 1) % 11;
        } else if (tecla == '\r' || tecla == '\n') {
            atira(&jogo);
        }
        clock_gettime(CLOCK_MONOTONIC, &agora);
        if (agora.tv_sec - inicio.tv_sec >= 2) {
            desloca(&jogo);
            inicio = agora;
        }
        desenha(&jogo);
    }
    printf("\nInvasao bem sucedida. Pontos: %d\n", jogo.pontos);
    system("stty sane");
    return 0;
}
