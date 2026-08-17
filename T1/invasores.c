#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pontos;
    int tiros;
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

void desenha_estado(Estado *jogo)
{
    printf("Pontos: %d  tiros: %d  Esc para sair   \r",
           jogo->pontos, jogo->tiros);
}

int main(void)
{
    Estado jogo = {0, 30, 0};

    configura_terminal();
    while (!jogo.acabou) {
        desenha_estado(&jogo);
        if (le_tecla() == 27) {
            jogo.acabou = 1;
        }
    }
    normaliza_terminal();
    return 0;
}
