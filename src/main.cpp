#include "server.hpp"

#ifdef USE_MNEMOSYNE
#include "allocator.h"
#endif

int main() {

#ifdef USE_MNEMOSYNE
    ma::init();
#endif

    {
        Server server(6379);
        server.run();
    }

#ifdef USE_MNEMOSYNE
    ma::shutdown();
#endif

    return 0;
}