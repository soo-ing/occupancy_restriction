#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include <wiringPi.h>
#include<softTone.h>

#define mqtt_host "localhost"
#define mqtt_port 1883
#define MQTT_TOPIC "space/people"

#define TRIG 23
#define ECHO 24

static int run = 1;

int i = 0;

int Flag = false;
int str[100] = { 0, };
int k;

int j;

const char* Check = "입장";
const char* Reset = "초기화";


void handle_signal(int s)
{
	run = 0;
}
void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	if (strcmp(Check, (char*)message->payload) == 0)
	{
		Flag = true;
	}
	else if (strcmp(Reset, (char*)message->payload) == 0) {
		Flag = true;

		str[j] = i;
		i = 0;

		for (k = 0; k < j + 1; k++) {
			printf("%d번째의 고객 수는 %d명입니다\n", k + 1, str[k]);
		}
		j++;
		printf("\n\n\n");
	}
	else
	{
		printf("오류\n");
	}
}

int main(int argc, char *argv[])
{
	int distance = 0;
	int pulse = 0;

	uint8_t reconnect = true;

	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	mosquitto_lib_init();

	wiringPiSetupGpio();

	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);

	mosq = mosquitto_new(NULL, true, 0);
	if (mosq) {
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);
		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
		mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);

		while (run)
		{
			rc = mosquitto_loop(mosq, -1, 1);
			if (run && rc) {
				printf("connection error!\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}
			if (Flag == true)
			{
				digitalWrite(TRIG, LOW);
				delay(100);
				digitalWrite(TRIG, HIGH);
				delay(100);
				digitalWrite(TRIG, LOW);

				while (digitalRead(ECHO) == LOW);
				long startTime = micros();
				while (digitalRead(ECHO) == HIGH);
				long travelTime = micros() - startTime;
				int distance = travelTime / 58;

				if (distance < 15) {
					i++;
				}

				Flag = false;
			}
		}
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();
	return rc;
}