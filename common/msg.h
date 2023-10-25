#ifndef MSG_H
#define MSG_H
struct Msg {
	uint16_t vout;
	uint16_t vbat;
	uint16_t iout;
	uint16_t ibat;
};

uint16_t msg_cobs_encode(struct Msg msg, char *buf);
uint16_t msg_cobs_decode(char *buf, struct Msg *msg);
#endif /* MSG_H */
