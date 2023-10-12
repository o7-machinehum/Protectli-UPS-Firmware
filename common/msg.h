#include <stdio.h>
#include <string.h>

struct Msg {
	uint8_t v1;
	uint8_t v2;
	uint8_t pad[2];
	uint16_t v3;
	uint16_t v4;
	uint32_t v5;
	float v6;
};

// https://blog.mbedded.ninja/programming/serialization-formats/consistent-overhead-byte-stuffing-cobs/
// Please note this cobs encoding and decoding is quite limited. If the packet is over a certain side
// there will be issues, and it doesn't check buffer lengths.
uint16_t msg_cobs_encode(struct Msg msg, char *buf)
{
	uint8_t *ptr = (uint8_t *)(&msg);
	int k = 1;
	uint8_t len = sizeof(struct Msg);

	buf[len + 1] = 0; // End Byte
	int i = len;
	while (i--) {
		if (ptr[i] == 0) {
			buf[i + 1] = k;
			k = 1;
		} else {
			buf[i + 1] = ptr[i];
			k++;
		}
	}
	buf[0] = k; // First byte

	return len + 2;
}

uint16_t msg_cobs_decode(char *buf, struct Msg *msg)
{
	uint8_t *ptr = (uint8_t *)(&msg);
	uint8_t len = sizeof(struct Msg);

	int offset = 0;
	for (int i = 0; buf[i]; i++) {
		offset = buf[i];
		buf[i] = 0;
		i += offset - 1;
	}

	memcpy((uint8_t *)(msg), buf + 1, len);
	return len;
}
