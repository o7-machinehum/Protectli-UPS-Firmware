#include <stdio.h>
#include <string.h>

struct Msg {
	uint8_t v1;
	uint8_t v2;
	uint8_t pad[2];
	uint16_t v3;
	uint16_t v4;
	uint32_t v5;
};

// https://blog.mbedded.ninja/programming/serialization-formats/consistent-overhead-byte-stuffing-cobs/
uint16_t msg_encode(struct Msg msg, char *buf)
{
	uint8_t *ptr = (uint8_t *)(&msg);
	int k = 0;
	uint8_t len = sizeof(struct Msg);

	buf[len + 2] = 0; // End Byte
	int i = len;
	while (i--) {
		k++;
		if (ptr[i] == 0) {
			buf[i + 1] = k;
			k = 0;
		} else {
			buf[i + 1] = ptr[i];
		}
	}
	k++;
	buf[0] = k; // First byte

	return len + 2;
}

uint16_t msg_decode(char *buf, struct Msg msg)
{
	uint8_t *ptr = (uint8_t *)(&msg);
	uint8_t len = sizeof(struct Msg);

	int i = 0;
	while (buf[i]) {
		size_t temp = buf[i];
		buf[i] = 0;
		i += temp;
	}

	memcpy(buf, (uint8_t *)(&msg), len);
	return len;
}
