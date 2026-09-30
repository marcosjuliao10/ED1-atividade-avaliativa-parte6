#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#define MAX_ENTIDADES 20
#define RAIO_JOGADOR 20.0f
#define ARQUIVO_PLACAR "placar.txt"
#define ARQUIVO_SAVE "save.bin"

typedef enum {
    ENTIDADE_JOGADOR,
    ENTIDADE_INIMIGO,
    ENTIDADE_ITEM
} TipoEntidade;

typedef union {
    int dano;
    int valor;
} ExtraEntidade;

typedef struct {
    TipoEntidade tipo;
    Vector2 pos;
    float raio;
    int vida;
    Color cor;
    ExtraEntidade extra;
} Entidade;

Entidade *vetorEntidades[MAX_ENTIDADES];
int totalEntidades = 0;
int pontuacao = 0;
char mensagem[100] = "";
float tempoMensagem = 0;

Entidade *criarEntidade(TipoEntidade tipo, Vector2 pos) {
    Entidade *e = (Entidade *)malloc(sizeof(Entidade));
    if (e == NULL) return NULL;

    e->tipo = tipo;
    e->pos = pos;
    e->raio = (tipo == ENTIDADE_JOGADOR) ? RAIO_JOGADOR
            : (tipo == ENTIDADE_INIMIGO) ? 15.0f : 8.0f;

    switch (tipo) {
        case ENTIDADE_JOGADOR:
            e->vida = 100;
            e->cor = BLUE;
            break;
        case ENTIDADE_INIMIGO:
            e->vida = 40;
            e->cor = MAROON;
            e->extra.dano = GetRandomValue(5, 15);
            break;
        case ENTIDADE_ITEM:
            e->vida = 1;
            e->cor = GOLD;
            e->extra.valor = GetRandomValue(5, 20);
            break;
    }
    return e;
}

void adicionarEntidade(Entidade *e) {
    if (totalEntidades < MAX_ENTIDADES) {
        vetorEntidades[totalEntidades] = e;
        totalEntidades++;
    }
}

void removerEntidade(int indice) {
    if (indice < 0 || indice >= totalEntidades) return;

    free(vetorEntidades[indice]);
    vetorEntidades[indice] = vetorEntidades[totalEntidades - 1];
    totalEntidades--;
}

void liberarTodasEntidades(void) {
    for (int i = 0; i < totalEntidades; i++) {
        free(vetorEntidades[i]);
    }
    totalEntidades = 0;
}

void salvarPlacarTexto(int pontuacao) {
    FILE *arquivo = fopen(ARQUIVO_PLACAR, "a");
    if (arquivo == NULL) return;

    fprintf(arquivo, "%d\n", pontuacao);
    fclose(arquivo);
}

int lerMelhorPontuacao(void) {
    FILE *arquivo = fopen(ARQUIVO_PLACAR, "r");
    if (arquivo == NULL) return 0;

    int melhor = 0, valor = 0;
    while (fscanf(arquivo, "%d", &valor) == 1) {
        if (valor > melhor) melhor = valor;
    }
    fclose(arquivo);
    return melhor;
}

bool salvarJogoBinario(void) {
    FILE *arquivo = fopen(ARQUIVO_SAVE, "wb");
    if (arquivo == NULL) return false;

    fwrite(&totalEntidades, sizeof(int), 1, arquivo);
    for (int i = 0; i < totalEntidades; i++) {
        fwrite(vetorEntidades[i], sizeof(Entidade), 1, arquivo);
    }

    fclose(arquivo);
    return true;
}

bool carregarJogoBinario(void) {
    FILE *arquivo = fopen(ARQUIVO_SAVE, "rb");
    if (arquivo == NULL) return false;

    int totalSalvo = 0;
    if (fread(&totalSalvo, sizeof(int), 1, arquivo) != 1) {
        fclose(arquivo);
        return false;
    }

    liberarTodasEntidades();
    for (int i = 0; i < totalSalvo; i++) {
        Entidade *e = (Entidade *)malloc(sizeof(Entidade));
        if (fread(e, sizeof(Entidade), 1, arquivo) != 1) {
            free(e);
            break;
        }
        adicionarEntidade(e);
    }

    fclose(arquivo);
    return true;
}

int main(void) {
    int larguraTela = 800;
    int alturaTela = 450;

    InitWindow(larguraTela, alturaTela, "Atividade 6 - Exercicio 2");
    SetTargetFPS(60);

    Entidade *jogador = criarEntidade(ENTIDADE_JOGADOR, (Vector2){400, 225});
    adicionarEntidade(jogador);

    for (int i = 0; i < 5; i++) {
        Vector2 pos;
        pos.x = GetRandomValue(0, larguraTela);
        pos.y = GetRandomValue(0, alturaTela);
        adicionarEntidade(criarEntidade(ENTIDADE_INIMIGO, pos));
    }

    for (int i = 0; i < 5; i++) {
        Vector2 pos;
        pos.x = GetRandomValue(0, larguraTela);
        pos.y = GetRandomValue(0, alturaTela);
        adicionarEntidade(criarEntidade(ENTIDADE_ITEM, pos));
    }

    int recorde = lerMelhorPontuacao();

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) jogador->pos.x += 3;
        if (IsKeyDown(KEY_LEFT)) jogador->pos.x -= 3;
        if (IsKeyDown(KEY_UP)) jogador->pos.y -= 3;
        if (IsKeyDown(KEY_DOWN)) jogador->pos.y += 3;

        for (int i = 1; i < totalEntidades; i++) {
            if (vetorEntidades[i]->tipo == ENTIDADE_ITEM) {
                if (CheckCollisionCircles(jogador->pos, jogador->raio, vetorEntidades[i]->pos, vetorEntidades[i]->raio)) {
                    pontuacao += vetorEntidades[i]->extra.valor;
                    removerEntidade(i);
                    break;
                }
            }
        }

        if (IsKeyPressed(KEY_F5)) {
            salvarPlacarTexto(pontuacao);
            strcpy(mensagem, "Pontuacao salva!");
            tempoMensagem = 2.0f;
        }

        if (IsKeyPressed(KEY_F6)) {
            salvarJogoBinario();
            strcpy(mensagem, "Jogo salvo!");
            tempoMensagem = 2.0f;
        }

        if (IsKeyPressed(KEY_F9)) {
            carregarJogoBinario();
            jogador = vetorEntidades[0];
            strcpy(mensagem, "Jogo carregado!");
            tempoMensagem = 2.0f;
        }

        if (IsKeyPressed(KEY_DELETE)) {
            if (remove(ARQUIVO_SAVE) == 0) {
                strcpy(mensagem, "Save apagado!");
            } else {
                strcpy(mensagem, "Nenhum save encontrado");
            }
            tempoMensagem = 2.0f;
        }

        if (tempoMensagem > 0) {
            tempoMensagem -= GetFrameTime();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < totalEntidades; i++) {
            DrawCircleV(vetorEntidades[i]->pos, vetorEntidades[i]->raio, vetorEntidades[i]->cor);
        }

        DrawText(TextFormat("Pontuacao: %d", pontuacao), 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Recorde: %d", recorde), 10, 35, 20, DARKGRAY);
        DrawText("F5 salva placar, F6 salva jogo, F9 carrega, DELETE apaga save", 10, 420, 15, DARKGRAY);

        if (tempoMensagem > 0) {
            DrawText(mensagem, 10, 60, 20, RED);
        }

        EndDrawing();
    }

    liberarTodasEntidades();
    CloseWindow();
    return 0;
}
