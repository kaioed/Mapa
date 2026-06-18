#ifndef VIA_H
#define VIA_H

#include "grafo.h"

#include <stdbool.h>

Grafo via_ler_arquivo(const char* caminho_arquivo);
bool via_processar_arquivo(const char* caminho_arquivo, Grafo grafo);

#endif
