#include "stdint.h"
#include "msg.h"

int main()
{
	char buff[256] = {};
	uint16_t len = 0;

	struct Msg m1 = {
		.v1 = 0,
		.v2 = 200,
		.v3 = 19209,
		.v4 = 38021,
		.v5 = 8903829,
	};

	struct Msg m2 = {0};

	len = msg_encode(m1, buff);
	len = msg_decode(buff, m2);

	printf("%d, %d, %d, %d, %d\n", m2.v1, m2.v2, m2.v3, m2.v4, m2.v5);
}
