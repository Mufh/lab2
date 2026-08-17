#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_POSICOES 10
typedef struct {
    int pontos, onda, tiros, arma, escudos, restantes, noite, acabou;
    int posicoes;
    double intervalo;
    char ataques[MAX_POSICOES];
} Estado;

char le_tecla(void)
{
    char tecla;

    fflush(stdout);
    return fread(&tecla, 1, 1, stdin) == 1 ? tecla : 0;
}

const char *armas(Estado *jogo)
{
    return jogo->noite ? "02468n" : "0123456789n";
}

void limpa(Estado *jogo)
{
    int i;

    for (i = 0; i < MAX_POSICOES; i++) {
        jogo->ataques[i] = ' ';
    }
}

int ha_ativos(Estado *jogo)
{
    int i;

    for (i = 0; i < jogo->posicoes; i++) {
        if (jogo->ataques[i] != ' ') {
            return 1;
        }
    }
    return 0;
}

void inicia_onda(Estado *jogo)
{
    int i, chance;

    jogo->onda++;
    chance = jogo->onda < 5 ? 120 - 20 * jogo->onda : 20;
    jogo->noite = rand() % 100 >= chance;
    jogo->posicoes = jogo->noite ? 5 : 10;
    jogo->restantes = jogo->noite ? 15 : 20;
    jogo->tiros = 30;
    jogo->escudos = 3;
    jogo->arma = 0;
    jogo->intervalo = 2.0;
    for (i = 1; i < jogo->onda; i++) {
        jogo->intervalo *= .9;
    }
    if (jogo->noite) {
        jogo->intervalo *= 3;
    }
    limpa(jogo);
}

void desloca(Estado *jogo)
{
    int i, quantidade;

    if (jogo->ataques[0] != ' ') {
        if (jogo->escudos > 0) {
            jogo->escudos--;
        } else {
            jogo->acabou = 1;
        }
    }
    for (i = 0; i < jogo->posicoes - 1; i++) {
        jogo->ataques[i] = jogo->ataques[i + 1];
    }
    jogo->ataques[jogo->posicoes - 1] = ' ';
    if (jogo->restantes > 0) {
        quantidade = jogo->noite ? 6 : 11;
        jogo->ataques[jogo->posicoes - 1] = armas(jogo)[rand() % quantidade];
        if (jogo->ataques[jogo->posicoes - 1] == 'n') {
            jogo->ataques[jogo->posicoes - 1] = 'N';
        }
        jogo->restantes--;
    }
}

void atira(Estado *jogo)
{
    int i, valor;
    char arma = armas(jogo)[jogo->arma];

    if (jogo->tiros <= 0) {
        return;
    }
    jogo->tiros--;
    for (i = 0; i < jogo->posicoes; i++) {
        if (jogo->ataques[i] == arma ||
            (arma == 'n' && jogo->ataques[i] == 'N')) {
            if (jogo->ataques[i] == 'N') {
                jogo->ataques[i] = 'n';
            } else {
                valor = jogo->posicoes - i;
                jogo->pontos += arma == 'n' ? 2 * valor : valor;
                if (jogo->noite) {
                    jogo->pontos += arma == 'n' ? 2 * valor : valor;
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

    if (jogo->noite) {
        printf("Pontos: %d                         \r", jogo->pontos);
        return;
    }
    printf("%3d %2d %c", jogo->pontos, jogo->tiros,
           armas(jogo)[jogo->arma]);
    for (i = 0; i < 3; i++) {
        printf("%c", i < jogo->escudos ? ')' : ' ');
    }
    for (i = 0; i < jogo->posicoes; i++) {
        printf("%c", jogo->ataques[i]);
    }
    printf("   \r");
}

void executa_onda(Estado *jogo)
{
    struct timespec inicio, agora;
    char tecla;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    while (!jogo->acabou && (jogo->restantes || ha_ativos(jogo))) {
        desenha(jogo);
        tecla = le_tecla();
        if (tecla == 27) {
            jogo->acabou = 1;
        } else if (tecla == '\t') {
            jogo->arma = armas(jogo)[jogo->arma + 1] ? jogo->arma + 1 : 0;
        } else if (tecla == '\r' || tecla == '\n') {
            atira(jogo);
        }
        clock_gettime(CLOCK_MONOTONIC, &agora);
        if (agora.tv_sec - inicio.tv_sec +
            1e-9 * (agora.tv_nsec - inicio.tv_nsec) >= jogo->intervalo) {
            desloca(jogo);
            inicio = agora;
        }
    }
}

int main(void)
{
    Estado jogo = {0};

    srand(time(NULL));
    system("stty raw -echo min 0 time 1 opost");
    setvbuf(stdin, NULL, _IONBF, 0);
    while (!jogo.acabou) {
        inicia_onda(&jogo);
        executa_onda(&jogo);
        if (!jogo.acabou) {
            jogo.pontos += 2 * jogo.tiros + 10 * jogo.escudos;
            printf("\nOnda %d concluida. Tecle r.\n", jogo.onda);
            while (le_tecla() != 'r') {
            }
        }
    }
    printf("\nPontuacao final: %d\n", jogo.pontos);
    system("stty sane");
    return 0;
}
