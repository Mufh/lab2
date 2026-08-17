#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 10
typedef struct {
    int pontos, onda, tiros, escudos, restantes, arma, noite, posicoes;
    int acabou, desistiu;
    double intervalo;
    char ataques[MAX];
} Estado;

char le_tecla(void)
{
    char c;

    fflush(stdout);
    return fread(&c, 1, 1, stdin) == 1 ? c : 0;
}

const char *armas(Estado *j)
{
    return j->noite ? "02468n" : "0123456789n";
}

void nome_som(char t, char *n)
{
    if (t == 'N' || t == 'n') sprintf(n, "11.3.wav");
    else if (t == ')') sprintf(n, "12.3.wav");
    else if (t == ' ') sprintf(n, "x.3.wav");
    else sprintf(n, "%c.3.wav", t);
}

void som(char t)
{
    char n[16], comando[64];

    nome_som(t, n);
    sprintf(comando, "aplay -q Sons/%s &", n);
    system(comando);
}

void limpa(Estado *j)
{
    int i;

    for (i = 0; i < MAX; i++) j->ataques[i] = ' ';
}

int ativos(Estado *j)
{
    int i;

    for (i = 0; i < j->posicoes; i++) if (j->ataques[i] != ' ') return 1;
    return 0;
}

void inicia_onda(Estado *j)
{
    int i, chance;

    j->onda++;
    chance = j->onda < 5 ? 120 - 20 * j->onda : 20;
    j->noite = rand() % 100 >= chance;
    j->posicoes = j->noite ? 5 : 10;
    j->restantes = j->noite ? 15 : 20;
    j->tiros = 30;
    j->escudos = 3;
    j->arma = 0;
    j->intervalo = 2;
    for (i = 1; i < j->onda; i++) j->intervalo *= .9;
    if (j->noite) j->intervalo *= 3;
    limpa(j);
}

void passa_tempo(Estado *j)
{
    int i, n;

    if (j->ataques[0] != ' ') {
        if (j->escudos) { j->escudos--; som(')'); }
        else j->acabou = 1;
    }
    for (i = 0; i < j->posicoes - 1; i++) j->ataques[i] = j->ataques[i + 1];
    j->ataques[j->posicoes - 1] = ' ';
    if (j->restantes) {
        n = j->noite ? 6 : 11;
        j->ataques[j->posicoes - 1] = armas(j)[rand() % n];
        if (j->ataques[j->posicoes - 1] == 'n') j->ataques[j->posicoes - 1] = 'N';
        j->restantes--;
        som(j->ataques[j->posicoes - 1]);
    }
}

void atira(Estado *j)
{
    int i, pontos;
    char arma = armas(j)[j->arma];

    if (!j->tiros) return;
    j->tiros--;
    for (i = 0; i < j->posicoes; i++) {
        if (j->ataques[i] == arma || (arma == 'n' && j->ataques[i] == 'N')) {
            if (j->ataques[i] == 'N') j->ataques[i] = 'n';
            else {
                pontos = j->posicoes - i;
                if (arma == 'n') pontos *= 2;
                if (j->noite) pontos *= 2;
                j->pontos += pontos;
                j->ataques[i] = ' ';
            }
            som(arma);
            return;
        }
    }
    som(' ');
}

void sonar(Estado *j)
{
    char comando[256], n[16];
    int i, usados = sprintf(comando, "aplay -q ");

    for (i = 0; i < 3; i++) { nome_som(i < j->escudos ? ')' : ' ', n); usados += sprintf(comando + usados, "Sons/%s ", n); }
    for (i = 0; i < j->posicoes; i++) { nome_som(j->ataques[i], n); usados += sprintf(comando + usados, "Sons/%s ", n); }
    sprintf(comando + usados, "&");
    system(comando);
}

void desenha(Estado *j)
{
    int i;

    if (j->noite) { printf("Pontos: %d                         \r", j->pontos); return; }
    printf("%3d %2d %c", j->pontos, j->tiros, armas(j)[j->arma]);
    for (i = 0; i < 3; i++) printf("%c", i < j->escudos ? ')' : ' ');
    for (i = 0; i < j->posicoes; i++) printf("%c", j->ataques[i]);
    printf("   \r");
}

void executa_onda(Estado *j)
{
    struct timespec inicio, agora;
    char tecla;

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    while (!j->acabou && (j->restantes || ativos(j))) {
        desenha(j);
        tecla = le_tecla();
        if (tecla == 27) { j->acabou = 1; j->desistiu = 1; }
        else if (tecla == '\t') { j->arma = armas(j)[j->arma + 1] ? j->arma + 1 : 0; som(armas(j)[j->arma]); }
        else if (tecla == '\r' || tecla == '\n') atira(j);
        else if (tecla == ' ') sonar(j);
        clock_gettime(CLOCK_MONOTONIC, &agora);
        if (agora.tv_sec - inicio.tv_sec + 1e-9 * (agora.tv_nsec - inicio.tv_nsec) >= j->intervalo) { passa_tempo(j); inicio = agora; }
    }
}

void le_recordes(int p[])
{
    FILE *f = fopen("pontuacoes.txt", "r");
    int i;

    for (i = 0; i < 3; i++) p[i] = 0;
    if (f) { for (i = 0; i < 3; i++) fscanf(f, "%d", &p[i]); fclose(f); }
}

int salva_recorde(int p[], int valor)
{
    FILE *f;
    int i, k, entrou = 0;

    for (i = 0; i < 3; i++) if (valor > p[i]) { for (k = 2; k > i; k--) p[k] = p[k - 1]; p[i] = valor; entrou = 1; break; }
    f = fopen("pontuacoes.txt", "w");
    if (f) { for (i = 0; i < 3; i++) fprintf(f, "%d\n", p[i]); fclose(f); }
    return entrou;
}

void partida(Estado *j)
{
    j->pontos = j->onda = j->acabou = j->desistiu = 0;
    while (!j->acabou) {
        inicia_onda(j);
        executa_onda(j);
        if (!j->acabou) { j->pontos += 2 * j->tiros + 10 * j->escudos; som(')'); printf("\nOnda %d concluida. Tecle r.\n", j->onda); while (le_tecla() != 'r') {} }
    }
}

int repetir(void)
{
    char c;

    printf("Jogar novamente? (s/n) ");
    do c = le_tecla(); while (c != 's' && c != 'S' && c != 'n' && c != 'N');
    printf("\n");
    return c == 's' || c == 'S';
}

int main(void)
{
    Estado jogo;
    int pontos[3], entrou;

    srand(time(NULL));
    system("stty raw -echo min 0 time 1 opost");
    setvbuf(stdin, NULL, _IONBF, 0);
    do {
        partida(&jogo);
        le_recordes(pontos);
        entrou = salva_recorde(pontos, jogo.pontos);
        printf("\n%s. Pontuacao final: %d\n", jogo.desistiu ? "Partida encerrada" : "Invasao bem sucedida", jogo.pontos);
        if (entrou) printf("Sua pontuacao ficou entre as tres maiores.\n");
        printf("Maiores pontuacoes: %d, %d, %d\n", pontos[0], pontos[1], pontos[2]);
        som('N');
    } while (repetir());
    system("stty sane");
    return 0;
}
