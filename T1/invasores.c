#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ATAQUES 10
#define MAX_SOM 256

typedef struct {
    int pontos;
    int onda;
    int tiros;
    int escudos;
    int restantes;
    int posicoes;
    int noite;
    int acabou;
    int desistiu;
    int arma;
    double intervalo;
    char ataques[MAX_ATAQUES];
} Estado;

typedef struct timespec Cronometro;

// Configura o terminal para leitura imediata de teclas.
void configura_terminal(void)
{
    if (system("stty raw -echo min 0 time 1 opost") != 0) {
        perror("erro ao configurar terminal");
        exit(1);
    }
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("erro ao configurar entrada");
        exit(1);
    }
}

// Devolve o terminal ao modo normal antes de encerrar o programa.
void normaliza_terminal(void)
{
    system("stty sane");
}

// Lê uma tecla ou retorna zero se nenhuma tecla foi pressionada.
char le_tecla(void)
{
    char c;

    fflush(stdout);
    if (fread(&c, 1, 1, stdin) == 1) {
        return c;
    }
    return 0;
}

// Inicia um cronometro usando o relogio monotônico do sistema.
void inicia_cronometro(Cronometro *inicio)
{
    clock_gettime(CLOCK_MONOTONIC, inicio);
}

// Retorna os segundos transcorridos desde o início do cronometro.
double tempo_decorrido(Cronometro *inicio)
{
    Cronometro agora;
    double segundos;
    double nanos;

    clock_gettime(CLOCK_MONOTONIC, &agora);
    segundos = agora.tv_sec - inicio->tv_sec;
    nanos = agora.tv_nsec - inicio->tv_nsec;
    return segundos + 1e-9 * nanos;
}

// Informa o conjunto de armas permitido para o tipo atual de onda.
const char *armas_disponiveis(Estado *jogo)
{
    if (jogo->noite) {
        return "02468n";
    }
    return "0123456789n";
}

// Retorna a arma atualmente selecionada pelo jogador.
char arma_atual(Estado *jogo)
{
    return armas_disponiveis(jogo)[jogo->arma];
}

// Converte um objeto para o nome do seu arquivo de som.
void nome_som(char tipo, char *nome)
{
    if (tipo == 'N' || tipo == 'n') {
        sprintf(nome, "11.3.wav");
    } else if (tipo == ')') {
        sprintf(nome, "12.3.wav");
    } else if (tipo == ' ') {
        sprintf(nome, "x.3.wav");
    } else {
        sprintf(nome, "%c.3.wav", tipo);
    }
}

// Toca um som em segundo plano usando o programa aplay.
void toca_som(char tipo)
{
    char arquivo[16];
    char comando[MAX_SOM];

    nome_som(tipo, arquivo);
    sprintf(comando, "aplay -q Sons/%s &", arquivo);
    system(comando);
}

// Retorna a probabilidade de uma onda ser diurna.
int chance_dia(int onda)
{
    if (onda == 1) {
        return 100;
    }
    if (onda < 5) {
        return 100 - 20 * (onda - 1);
    }
    return 20;
}

// Sorteia um tipo de ataque permitido na onda atual.
char sorteia_ataque(Estado *jogo)
{
    const char *tipos;
    int quantidade;
    int posicao;

    tipos = armas_disponiveis(jogo);
    quantidade = jogo->noite ? 6 : 11;
    posicao = rand() % quantidade;
    if (tipos[posicao] == 'n') {
        return 'N';
    }
    return tipos[posicao];
}

// Limpa todas as posições usadas para os ataques inimigos.
void limpa_ataques(Estado *jogo)
{
    int i;

    for (i = 0; i < MAX_ATAQUES; i++) {
        jogo->ataques[i] = ' ';
    }
}

// Calcula o intervalo de movimentação da onda atual.
double calcula_intervalo(int onda)
{
    double intervalo;
    int i;

    intervalo = 2.0;
    for (i = 1; i < onda; i++) {
        intervalo *= 0.9;
    }
    return intervalo;
}

// Inicializa os valores que mudam a cada onda de ataques.
void inicia_onda(Estado *jogo)
{
    jogo->onda++;
    jogo->noite = rand() % 100 >= chance_dia(jogo->onda);
    jogo->posicoes = jogo->noite ? 5 : 10;
    jogo->restantes = jogo->noite ? 15 : 20;
    jogo->tiros = 30;
    jogo->escudos = 3;
    jogo->arma = 0;
    jogo->intervalo = calcula_intervalo(jogo->onda);
    if (jogo->noite) {
        jogo->intervalo *= 3.0;
    }
    limpa_ataques(jogo);
}

// Desenha a linha de estado visível durante uma onda diurna.
void desenha_dia(Estado *jogo)
{
    int i;

    printf("%3d %2d %c", jogo->pontos, jogo->tiros, arma_atual(jogo));
    for (i = 0; i < 3; i++) {
        printf("%c", i < jogo->escudos ? ')' : ' ');
    }
    printf(" ");
    for (i = 0; i < jogo->posicoes; i++) {
        printf("%c", jogo->ataques[i]);
    }
    printf("   \r");
}

// Desenha a linha mínima usada durante uma onda noturna.
void desenha_noite(Estado *jogo)
{
    printf("Pontos: %d                         \r", jogo->pontos);
}

// Desenha a tela apropriada para o período atual da onda.
void desenha_estado(Estado *jogo)
{
    if (jogo->noite) {
        desenha_noite(jogo);
    } else {
        desenha_dia(jogo);
    }
}

// Retorna se há algum ataque ativo nas posições da tela.
int ha_ataque_ativo(Estado *jogo)
{
    int i;

    for (i = 0; i < jogo->posicoes; i++) {
        if (jogo->ataques[i] != ' ') {
            return 1;
        }
    }
    return 0;
}

// Move os ataques uma posição para a esquerda após tratar a colisão.
void desloca_ataques(Estado *jogo)
{
    int i;

    if (jogo->ataques[0] != ' ') {
        if (jogo->escudos > 0) {
            jogo->escudos--;
            jogo->ataques[0] = ' ';
            toca_som(')');
        } else {
            jogo->acabou = 1;
        }
    }
    for (i = 0; i < jogo->posicoes - 1; i++) {
        jogo->ataques[i] = jogo->ataques[i + 1];
    }
    jogo->ataques[jogo->posicoes - 1] = ' ';
}

// Ativa um novo ataque na última posição, se a onda ainda tiver ataques.
void ativa_ataque(Estado *jogo)
{
    int ultimo;

    if (jogo->restantes == 0) {
        return;
    }
    ultimo = jogo->posicoes - 1;
    jogo->ataques[ultimo] = sorteia_ataque(jogo);
    jogo->restantes--;
    toca_som(jogo->ataques[ultimo]);
}

// Processa uma passagem de tempo durante a onda.
void processa_tempo(Estado *jogo)
{
    desloca_ataques(jogo);
    if (!jogo->acabou) {
        ativa_ataque(jogo);
    }
}

// Avança a seleção para a próxima arma disponível.
void troca_arma(Estado *jogo)
{
    const char *armas;

    armas = armas_disponiveis(jogo);
    jogo->arma++;
    if (armas[jogo->arma] == '\0') {
        jogo->arma = 0;
    }
    toca_som(arma_atual(jogo));
}

// Localiza o ataque mais próximo da base atingível pela arma escolhida.
int encontra_alvo(Estado *jogo, char arma)
{
    int i;

    for (i = 0; i < jogo->posicoes; i++) {
        if (jogo->ataques[i] == arma) {
            return i;
        }
        if (arma == 'n' && jogo->ataques[i] == 'N') {
            return i;
        }
    }
    return -1;
}

// Calcula os pontos de destruir um inimigo em uma determinada posição.
int valor_ataque(Estado *jogo, int posicao, char tipo)
{
    int valor;

    valor = jogo->posicoes - posicao;
    if (tipo == 'n') {
        valor *= 2;
    }
    if (jogo->noite) {
        valor *= 2;
    }
    return valor;
}

// Dispara a arma selecionada e atualiza o placar quando houver acerto.
void atira(Estado *jogo)
{
    char arma;
    int alvo;

    if (jogo->tiros == 0) {
        return;
    }
    jogo->tiros--;
    arma = arma_atual(jogo);
    alvo = encontra_alvo(jogo, arma);
    if (alvo < 0) {
        toca_som(' ');
    } else if (arma == 'n' && jogo->ataques[alvo] == 'N') {
        jogo->ataques[alvo] = 'n';
        toca_som('n');
    } else {
        jogo->pontos += valor_ataque(jogo, alvo, arma);
        jogo->ataques[alvo] = ' ';
        toca_som(arma);
    }
}

// Acrescenta o arquivo de som de um objeto ao comando do sonar.
void adiciona_som(char *comando, int *tamanho, char tipo)
{
    char arquivo[16];

    nome_som(tipo, arquivo);
    *tamanho += sprintf(comando + *tamanho, "Sons/%s ", arquivo);
}

// Toca em sequência os sons de todos os objetos e espaços do jogo.
void usa_sonar(Estado *jogo)
{
    char comando[MAX_SOM];
    int i;
    int tamanho;

    tamanho = sprintf(comando, "aplay -q ");
    for (i = 0; i < 3; i++) {
        adiciona_som(comando, &tamanho, i < jogo->escudos ? ')' : ' ');
    }
    for (i = 0; i < jogo->posicoes; i++) {
        adiciona_som(comando, &tamanho, jogo->ataques[i]);
    }
    sprintf(comando + tamanho, "&");
    system(comando);
}

// Processa uma tecla válida recebida durante a execução da onda.
void processa_tecla(Estado *jogo, char tecla)
{
    if (tecla == 27) {
        jogo->acabou = 1;
        jogo->desistiu = 1;
    } else if (tecla == '\t') {
        troca_arma(jogo);
    } else if (tecla == '\r' || tecla == '\n') {
        atira(jogo);
    } else if (tecla == ' ') {
        usa_sonar(jogo);
    }
}

// Retorna se a onda terminou sem ataques ativos ou pendentes.
int onda_terminou(Estado *jogo)
{
    return jogo->restantes == 0 && !ha_ataque_ativo(jogo);
}

// Concede os pontos extras obtidos ao sobreviver ao final de uma onda.
void bonifica_onda(Estado *jogo)
{
    jogo->pontos += 2 * jogo->tiros + 10 * jogo->escudos;
}

// Aguarda a tecla r entre ondas, mantendo o terminal no modo cru.
void espera_proxima_onda(Estado *jogo)
{
    char tecla;

    printf("\nOnda %d concluida. Pontos: %d. Tecle r.\n",
           jogo->onda, jogo->pontos);
    do {
        tecla = le_tecla();
        if (tecla == 27) {
            jogo->acabou = 1;
            jogo->desistiu = 1;
        }
    } while (tecla != 'r' && !jogo->acabou);
}

// Executa uma onda até que seus ataques acabem ou a partida termine.
void executa_onda(Estado *jogo)
{
    Cronometro inicio;
    char tecla;

    inicia_cronometro(&inicio);
    while (!jogo->acabou && !onda_terminou(jogo)) {
        desenha_estado(jogo);
        tecla = le_tecla();
        processa_tecla(jogo, tecla);
        if (tempo_decorrido(&inicio) >= jogo->intervalo) {
            processa_tempo(jogo);
            inicia_cronometro(&inicio);
        }
    }
}

// Lê as três maiores pontuações salvas, usando zero quando não há arquivo.
void le_pontuacoes(int pontos[])
{
    FILE *arquivo;
    int i;

    arquivo = fopen("pontuacoes.txt", "r");
    for (i = 0; i < 3; i++) {
        pontos[i] = 0;
    }
    if (arquivo == NULL) {
        return;
    }
    for (i = 0; i < 3; i++) {
        fscanf(arquivo, "%d", &pontos[i]);
    }
    fclose(arquivo);
}

// Insere a pontuação nas três maiores, se ela se qualificar.
int inclui_pontuacao(int pontos[], int pontuacao)
{
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        if (pontuacao > pontos[i]) {
            for (j = 2; j > i; j--) {
                pontos[j] = pontos[j - 1];
            }
            pontos[i] = pontuacao;
            return 1;
        }
    }
    return 0;
}

// Tenta salvar as três maiores pontuações sem encerrar em caso de erro.
void grava_pontuacoes(int pontos[])
{
    FILE *arquivo;
    int i;

    arquivo = fopen("pontuacoes.txt", "w");
    if (arquivo == NULL) {
        return;
    }
    for (i = 0; i < 3; i++) {
        fprintf(arquivo, "%d\n", pontos[i]);
    }
    fclose(arquivo);
}

// Mostra o resultado final e atualiza o arquivo de recordes.
void mostra_resultado(Estado *jogo)
{
    int pontos[3];
    int entrou;

    le_pontuacoes(pontos);
    entrou = inclui_pontuacao(pontos, jogo->pontos);
    grava_pontuacoes(pontos);
    printf("\n%s. Pontuacao final: %d\n",
           jogo->desistiu ? "Partida encerrada" : "Invasao bem sucedida",
           jogo->pontos);
    if (entrou) {
        printf("Sua pontuacao ficou entre as tres maiores.\n");
    }
    printf("Maiores pontuacoes: %d, %d, %d\n",
           pontos[0], pontos[1], pontos[2]);
    toca_som('N');
}

// Inicializa uma nova partida antes de iniciar a primeira onda.
void inicia_partida(Estado *jogo)
{
    jogo->pontos = 0;
    jogo->onda = 0;
    jogo->arma = 0;
    jogo->acabou = 0;
    jogo->desistiu = 0;
}

// Executa as ondas da partida até ocorrer uma derrota ou desistência.
void executa_partida(Estado *jogo)
{
    inicia_partida(jogo);
    while (!jogo->acabou) {
        inicia_onda(jogo);
        executa_onda(jogo);
        if (!jogo->acabou) {
            bonifica_onda(jogo);
            toca_som(')');
            espera_proxima_onda(jogo);
        }
    }
    mostra_resultado(jogo);
}

// Pergunta se o jogador quer iniciar outra partida.
int quer_repetir(void)
{
    char tecla;

    printf("Jogar novamente? (s/n) ");
    do {
        tecla = le_tecla();
    } while (tecla != 's' && tecla != 'S' && tecla != 'n' && tecla != 'N');
    printf("\n");
    return tecla == 's' || tecla == 'S';
}

// Configura o programa e executa partidas enquanto o jogador desejar.
int main(void)
{
    Estado jogo;

    srand(time(NULL));
    configura_terminal();
    do {
        executa_partida(&jogo);
    } while (quer_repetir());
    normaliza_terminal();
    return 0;
}
