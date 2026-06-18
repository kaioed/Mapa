#include "../include/via.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIA_TAM_LINHA 1024
#define VIA_TAM_CAMPO 128

static bool linha_ignorada(const char* linha) {
    const unsigned char* p = (const unsigned char*)linha;

    while (*p != '\0' && isspace(*p)) {
        p++;
    }

    return *p == '\0' || *p == '#';
}

static void remover_quebra_linha(char* linha) {
    size_t tamanho = strlen(linha);

    while (tamanho > 0 && (linha[tamanho - 1] == '\n' || linha[tamanho - 1] == '\r')) {
        linha[tamanho - 1] = '\0';
        tamanho--;
    }
}

static bool ler_numero_vertices(FILE* arquivo, int* numero_vertices) {
    char linha[VIA_TAM_LINHA];

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (linha_ignorada(linha)) {
            continue;
        }

        if (sscanf(linha, "%d", numero_vertices) == 1 && *numero_vertices >= 0) {
            return true;
        }

        return false;
    }

    return false;
}

static void processar_linha_vertice(Grafo grafo, const char* linha) {
    char comando;
    char id[VIA_TAM_CAMPO];
    double x;
    double y;

    if (sscanf(linha, " %c %127s %lf %lf", &comando, id, &x, &y) == 4) {
        grafo_inserir_vertice(grafo, id, x, y);
    }
}

static void processar_linha_aresta(Grafo grafo, char* linha) {
    char comando;
    char origem[VIA_TAM_CAMPO];
    char destino[VIA_TAM_CAMPO];
    char ldir[VIA_TAM_CAMPO];
    char lesq[VIA_TAM_CAMPO];
    char nome[VIA_TAM_LINHA];
    double comp;
    double vm;

    nome[0] = '\0';

    if (sscanf(linha, " %c %127s %127s %127s %127s %lf %lf %1023[^\n]",
               &comando, origem, destino, ldir, lesq, &comp, &vm, nome) >= 8) {
        char* inicio_nome = nome;

        while (*inicio_nome != '\0' && isspace((unsigned char)*inicio_nome)) {
            inicio_nome++;
        }

        grafo_inserir_aresta(grafo, origem, destino, inicio_nome, ldir, lesq, comp, vm);
    }
}

static bool processar_comandos(FILE* arquivo, Grafo grafo) {
    char linha[VIA_TAM_LINHA];

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        char comando;

        if (linha_ignorada(linha)) {
            continue;
        }

        remover_quebra_linha(linha);
        if (sscanf(linha, " %c", &comando) != 1) {
            continue;
        }

        if (comando == 'v') {
            processar_linha_vertice(grafo, linha);
        } else if (comando == 'e') {
            processar_linha_aresta(grafo, linha);
        }
    }

    return true;
}

Grafo via_ler_arquivo(const char* caminho_arquivo) {
    if (caminho_arquivo == NULL) {
        return NULL;
    }

    FILE* arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        return NULL;
    }

    int numero_vertices = 0;
    if (!ler_numero_vertices(arquivo, &numero_vertices)) {
        fclose(arquivo);
        return NULL;
    }

    Grafo grafo = grafo_criar(numero_vertices);
    if (grafo == NULL) {
        fclose(arquivo);
        return NULL;
    }

    if (!processar_comandos(arquivo, grafo)) {
        grafo_destruir(grafo);
        fclose(arquivo);
        return NULL;
    }

    fclose(arquivo);
    return grafo;
}

bool via_processar_arquivo(const char* caminho_arquivo, Grafo grafo) {
    if (caminho_arquivo == NULL || grafo == NULL) {
        return false;
    }

    FILE* arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        return false;
    }

    int numero_vertices = 0;
    if (!ler_numero_vertices(arquivo, &numero_vertices)) {
        fclose(arquivo);
        return false;
    }

    (void)numero_vertices;
    bool ok = processar_comandos(arquivo, grafo);
    fclose(arquivo);
    return ok;
}
