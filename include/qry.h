#ifndef QRY_H
#define QRY_H

#include "grafo.h"
#include "hash_extensivel.h"

#include <stdbool.h>

bool qry_processar_arquivo(const char* caminho_qry, const char* caminho_svg,
                           const char* caminho_txt, HashExtensivel* hash_quadras,
                           Grafo grafo);

#endif
