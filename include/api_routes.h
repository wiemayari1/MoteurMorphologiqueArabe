#ifndef API_ROUTES_H
#define API_ROUTES_H

#include "AVL.h"
#include "hash_table.h"
#include "http_server.h"


namespace morpho {

void registerRoutes(HttpServer &server, AVLTree &roots, HashTable &schemes);

} // namespace morpho

#endif