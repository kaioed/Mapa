#include "geo.h"
#include "grafo.h"
#include "hash_extensivel.h"
#include "via.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define CRIAR_DIRETORIO(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define CRIAR_DIRETORIO(path) mkdir(path, 0777)
#endif

#define CAMINHO_MAX 1024

static void imprimir_uso(const char* programa) {
    fprintf(stderr, "Uso: %s [-e path] -f arq.geo [-q consulta.qry] [-v arqvias.via] -o dir\n",
            programa);
}

static bool caminho_absoluto(const char* caminho) {
    if (caminho == NULL || caminho[0] == '\0') {
        return false;
    }

    if (caminho[0] == '/' || caminho[0] == '\\') {
        return true;
    }

    return strlen(caminho) > 2 && caminho[1] == ':';
}

static bool juntar_caminho(const char* base, const char* nome, char* saida, size_t tamanho) {
    if (nome == NULL || saida == NULL || tamanho == 0) {
        return false;
    }

    if (caminho_absoluto(nome) || base == NULL || base[0] == '\0') {
        return snprintf(saida, tamanho, "%s", nome) < (int)tamanho;
    }

    size_t tamanho_base = strlen(base);
    const char separador = (tamanho_base > 0 && (base[tamanho_base - 1] == '/' ||
                                                 base[tamanho_base - 1] == '\\')) ? '\0' : '/';

    if (separador == '\0') {
        return snprintf(saida, tamanho, "%s%s", base, nome) < (int)tamanho;
    }

    return snprintf(saida, tamanho, "%s/%s", base, nome) < (int)tamanho;
}

static const char* obter_nome_arquivo(const char* caminho) {
    const char* barra = strrchr(caminho, '/');
    const char* barra_invertida = strrchr(caminho, '\\');
    const char* ultimo = barra > barra_invertida ? barra : barra_invertida;

    return ultimo != NULL ? ultimo + 1 : caminho;
}

static bool obter_nome_base_sem_extensao(const char* caminho, char* saida, size_t tamanho) {
    const char* nome_arquivo = obter_nome_arquivo(caminho);

    if (snprintf(saida, tamanho, "%s", nome_arquivo) >= (int)tamanho) {
        return false;
    }

    char* ponto = strrchr(saida, '.');
    if (ponto != NULL) {
        *ponto = '\0';
    }

    return saida[0] != '\0';
}

static bool criar_diretorio_saida(const char* caminho) {
    if (caminho == NULL || caminho[0] == '\0' || strcmp(caminho, ".") == 0) {
        return true;
    }

    if (CRIAR_DIRETORIO(caminho) == 0) {
        return true;
    }

    return errno == EEXIST;
}

static bool montar_saida(const char* dir_saida, const char* base, const char* extensao,
                         char* saida, size_t tamanho) {
    char nome[CAMINHO_MAX];

    if (snprintf(nome, sizeof(nome), "%s%s", base, extensao) >= (int)sizeof(nome)) {
        return false;
    }

    return juntar_caminho(dir_saida, nome, saida, tamanho);
}

static bool parse_argumentos(int argc, char** argv, const char** dir_entrada,
                             const char** arquivo_geo, const char** arquivo_qry,
                             const char** arquivo_via, const char** dir_saida) {
    *dir_entrada = ".";
    *arquivo_geo = NULL;
    *arquivo_qry = NULL;
    *arquivo_via = NULL;
    *dir_saida = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            *dir_entrada = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            *arquivo_geo = argv[++i];
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            *arquivo_qry = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
            *arquivo_via = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            *dir_saida = argv[++i];
        } else {
            return false;
        }
    }

    return *arquivo_geo != NULL && *dir_saida != NULL;
}

static bool escrever_relatorio(const char* caminho_txt, const char* caminho_geo,
                               const char* caminho_via, const char* caminho_qry,
                               Grafo grafo) {
    FILE* txt = fopen(caminho_txt, "w");

    if (txt == NULL) {
        return false;
    }

    fprintf(txt, "Arquivo GEO: %s\n", caminho_geo);
    fprintf(txt, "SVG base gerado a partir das quadras do arquivo .geo.\n");

    if (grafo != NULL) {
        fprintf(txt, "Arquivo VIA: %s\n", caminho_via);
        fprintf(txt, "Vertices do grafo viario: %d\n", grafo_quantidade_vertices(grafo));
        fprintf(txt, "Arestas do grafo viario: %d\n", grafo_quantidade_arestas(grafo));
    } else {
        fprintf(txt, "Arquivo VIA: nao informado.\n");
    }

    if (caminho_qry != NULL) {
        fprintf(txt, "Arquivo QRY: %s\n", caminho_qry);
        fprintf(txt, "Consultas .qry ainda nao processadas nesta etapa.\n");
    } else {
        fprintf(txt, "Arquivo QRY: nao informado.\n");
    }

    fclose(txt);
    return true;
}

int main(int argc, char** argv) {
    const char* dir_entrada;
    const char* arquivo_geo;
    const char* arquivo_qry;
    const char* arquivo_via;
    const char* dir_saida;
    char caminho_geo[CAMINHO_MAX];
    char caminho_qry[CAMINHO_MAX];
    char caminho_via[CAMINHO_MAX];
    char nome_base[CAMINHO_MAX];
    char caminho_svg[CAMINHO_MAX];
    char caminho_txt[CAMINHO_MAX];
    char caminho_hash[CAMINHO_MAX];
    Grafo grafo = NULL;

    if (!parse_argumentos(argc, argv, &dir_entrada, &arquivo_geo, &arquivo_qry,
                          &arquivo_via, &dir_saida)) {
        imprimir_uso(argv[0]);
        return 1;
    }

    if (!criar_diretorio_saida(dir_saida) ||
        !juntar_caminho(dir_entrada, arquivo_geo, caminho_geo, sizeof(caminho_geo)) ||
        !obter_nome_base_sem_extensao(arquivo_geo, nome_base, sizeof(nome_base)) ||
        !montar_saida(dir_saida, nome_base, ".svg", caminho_svg, sizeof(caminho_svg)) ||
        !montar_saida(dir_saida, nome_base, ".txt", caminho_txt, sizeof(caminho_txt)) ||
        !montar_saida(dir_saida, nome_base, ".hash.tmp", caminho_hash, sizeof(caminho_hash))) {
        fprintf(stderr, "Erro ao montar caminhos de entrada ou saida.\n");
        return 1;
    }

    HashExtensivel* hash_quadras = hash_criar(2, caminho_hash);
    if (hash_quadras == NULL) {
        fprintf(stderr, "Erro ao criar estrutura de quadras.\n");
        return 1;
    }

    if (!geo_processar_arquivo(caminho_geo, caminho_svg, hash_quadras)) {
        fprintf(stderr, "Erro ao processar arquivo .geo: %s\n", caminho_geo);
        hash_destruir(hash_quadras);
        remove(caminho_hash);
        return 1;
    }

    hash_destruir(hash_quadras);
    remove(caminho_hash);

    if (arquivo_via != NULL) {
        if (!juntar_caminho(dir_entrada, arquivo_via, caminho_via, sizeof(caminho_via))) {
            fprintf(stderr, "Erro ao montar caminho do arquivo .via.\n");
            return 1;
        }

        grafo = via_ler_arquivo(caminho_via);
        if (grafo == NULL) {
            fprintf(stderr, "Erro ao processar arquivo .via: %s\n", caminho_via);
            return 1;
        }
    }

    const char* caminho_qry_relatorio = NULL;
    if (arquivo_qry != NULL) {
        if (!juntar_caminho(dir_entrada, arquivo_qry, caminho_qry, sizeof(caminho_qry))) {
            fprintf(stderr, "Erro ao montar caminho do arquivo .qry.\n");
            grafo_destruir(grafo);
            return 1;
        }
        caminho_qry_relatorio = caminho_qry;
    }

    if (!escrever_relatorio(caminho_txt, caminho_geo, arquivo_via != NULL ? caminho_via : NULL,
                            caminho_qry_relatorio, grafo)) {
        fprintf(stderr, "Erro ao escrever arquivo .txt: %s\n", caminho_txt);
        grafo_destruir(grafo);
        return 1;
    }

    grafo_destruir(grafo);
    return 0;
}
