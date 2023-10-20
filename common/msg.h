#ifndef MSG_H
#define MSG_H
struct Msg {
	uint16_t voltage;
	uint16_t current;
	uint8_t pad[2];
};

uint16_t msg_cobs_encode(struct Msg msg, char *buf);
uint16_t msg_cobs_decode(char *buf, struct Msg *msg);
#endif /* MSG_H */
