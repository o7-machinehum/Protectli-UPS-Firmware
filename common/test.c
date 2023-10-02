#include "stdint.h"
#include "msg.h"

int main() {
    char buff[32] = {};
    uint16_t len  = 0;

    struct Msg m1 = {
        .v1 = 0,
        .v2 = 1,
        // .v3 = 0,
        // .v4 = 1
    };

    struct Msg m2 = {0};

    len = msg_encode(m1, buff);
    len = msg_decode(buff, m2);
}
