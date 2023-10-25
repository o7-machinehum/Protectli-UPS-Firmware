#include <stdio.h>
#include "stdint.h"
#include "msg.h"

int main()
{
	char buff[256] = {};
	uint16_t len = 0;

	struct Msg m1 = {.voltage = 1120, .current = 1230};

	struct Msg m2 = {0};

	len = msg_cobs_encode(m1, buff);
	len = msg_cobs_decode(buff, &m2);

	printf("%d, %d", m2.voltage, m2.current);
}
