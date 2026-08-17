#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pontos;
    int tiros;
    int arma;
    int acabou;
} Estado;

void configura_terminal(void)
{
    system("stty raw -echo min 0 time 1 opost");
    setvbuf(stdin, NULL, _IONBF, 0);
}

void normaliza_terminal(void)
{
    system("stty sane");
}

char le_tecla(void)
{
    char tecla;

    fflush(stdout);
    if (fread(&tecla, 1, 1, stdin) == 1) {
        return tecla;
    }
    return 0;
}

char arma_atual(Estado *jogo)
{
    return "0123456789n"[jogo->arma];
}

void processa_tecla(Estado *jogo, char tecla)
{
    if (tecla == 27) {
        jogo->acabou = 1;
    } else if (tecla == '\t') {
        jogo->arma = (jogo->arma + 1) % 11;
    } else if ((tecla == '\r' || tecla == '\n') && jogo->tiros > 0) {
        jogo->tiros--;
    }
}

void desenha_estado(Estado *jogo)
{
    printf("%3d %2d %c)))          \r", jogo->pontos,
           jogo->tiros, arma_atual(jogo));
}

int main(void)
{
    Estado jogo = {0, 30, 0, 0};

    configura_terminal();
    while (!jogo.acabou) {
        desenha_estado(&jogo);
        processa_tecla(&jogo, le_tecla());
    }
    normaliza_terminal();
    return 0;
}
