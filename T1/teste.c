#include <stdio.h>
#include <time.h>

// implementação de um cronômetro
typedef struct timespec crono;

// inicializa um cronômetro com a hora atual
void crono_inicia(crono *c)
{
    clock_gettime(CLOCK_MONOTONIC, c);
}

// retorna o tempo passado desde que o cronômetro *c foi iniciado, em segundos
double crono_parcial(crono *c)
{
    crono agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);

    double segundos = agora.tv_sec - c->tv_sec;
    double nanosegundos = agora.tv_nsec - c->tv_nsec;
    return segundos + 1e-9 * nanosegundos;
}

int f(int x)
{
    if (x <= 0) return 0;
    return f(x - 1);
}

int main()
{
    crono c1;
    crono_inicia(&c1);
    f(1000);
    double t1 = crono_parcial(&c1);
    f(2000);
    double t2 = crono_parcial(&c1);
    printf("f(1000) demorou %f segundos.\n", t1);
    printf("f(2000) demorou %f segundos.\n", t2 - t1);
}