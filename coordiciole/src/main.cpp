#ifdef ENERGIA
  #include "Energia.h"
#else
  #include "Arduino.h"
#endif

#include <AltSoftSerial.h>
#include <XBee.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

#define R_LED 10
#define G_LED  9

#define SONDE A0

#define SERIAL_SPEED 9600

#define DATA 6
#define LATCH 7
#define CLOCK 10

#define LED_NONE 0x00
#define LED_G 0x01
#define LED_R 0x02
#define LED_1 0x04
#define LED_2 0x08
#define LED_3 0x10
#define LED_4 0x20
#define LED_5 0x40
#define LED_6 0x80

#define XBEE_RX_PIN 13
#define XBEE_TX_PIN 5

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();

Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

AltSoftSerial altSerial;

int leds;

uint16_t last_value;

void led_flash(int, int, int);
void led_breathe(int, uint32_t, uint32_t);

void process_rest(YunClient);

void xbee_configure();
void xbee_transmit();
void xbee_receive();
void xbee_decode_message(RxResponse&);

void shift_led(int value) {
	digitalWrite(LATCH, LOW);
	shiftOut(DATA, CLOCK, MSBFIRST, value);
	digitalWrite(LATCH, HIGH);
	
	leds = value;
}

void setup() {
	Serial.begin(SERIAL_SPEED);
	
	pinMode(XBEE_RX_PIN, INPUT);
	pinMode(XBEE_TX_PIN, OUTPUT);
	altSerial.begin(9600);
	xbee_configure();
	xbee.begin(altSerial);
	
	pinMode(LATCH, OUTPUT);
	pinMode(CLOCK, OUTPUT);
	pinMode(DATA,  OUTPUT);
	
	shift_led(LED_G | LED_R);
	Bridge.begin();
	shift_led(LED_NONE);
	led_flash(LED_G | LED_R, 250, 10);
}

void loop() {
	xbee_receive();
	delay(100);
}

void process_rest(YunClient client) {
	String command = client.readStringUntil('/');
	command.trim();
	
	if (command == "get") {
		client.print(F("value = "));
		client.println(last_value);	
	}
	else {
		client.print(F("unknown command \""));
		client.print(command);
		client.println(F("\""));
	}
}

void led_flash(int led, int length, int count) {
	int step = length / ( 2 * count );

	for (int i=0; i<count; i++) {
		shift_led(leds | led);
		delay(length / 2);
		shift_led(leds & (0xFF ^ led));
		delay(length / 2);
	}
}

void xbee_configure() {
	char thisByte = 0;

	altSerial.print("+++");

	while (thisByte != '\r' && thisByte != '\n') {
		if (altSerial.available() > 0)
			thisByte = altSerial.read(); 
	}

	altSerial.print("ATRE\r");
	altSerial.print("ATAP2\r");
	altSerial.print("ATCE1\r");
	altSerial.print("ATMY");
	altSerial.print(BEE_ADDRESS);
	altSerial.print("\r"); 
	altSerial.print("ATID1111\r");
	altSerial.print("ATCH0C\r");
	altSerial.print("ATCN\r");
}

void xbee_decode_message(Rx16Response& r) {
	uint8_t* data = r.getData();
	uint16_t value = data[0] << 8 | data[1];
	uint8_t label_length = data[1];
	uint8_t level = 6 - map(value, 0, 1023, 0, 6);
	uint8_t level_led = leds & (LED_G | LED_R);
		
	char message[label_length];

	for (int i=0; i<label_length; i++)
		message[i] = static_cast<char>(data[i+2]);

	switch(level) {
	case 6: level_led |= LED_6;
	case 5: level_led |= LED_5;
	case 4: level_led |= LED_4;
	case 3: level_led |= LED_3;
	case 2: level_led |= LED_2;
	case 1: level_led |= LED_1;
	default: break;
	}
	
	Bridge.put("light", String(value));
	last_value = value;
	
	shift_led(level_led);
	led_flash(LED_G, 100, 5);
}

void xbee_transmit() {
	uint8_t label[] = "LIGHT";
	uint8_t payload[sizeof(label)+2];
	uint16_t sonde = analogRead(SONDE);
	
	payload[0] = sonde >> 8 & 0xFF;
	payload[1] = sonde & 0xFF;
	payload[2] = sizeof(label);
	
	for (int i=0; i<sizeof(label); i++)
		payload[i+2] = label[i];
	
	Tx16Request tx;
	TxStatusResponse txStatus = TxStatusResponse();

	tx = Tx16Request(0x1874, payload, sizeof(payload));
	xbee.send(tx);

	if (xbee.readPacket(500)) {
		if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);
			//
			// Get the delivery status, the fifth byte
			//
			if (txStatus.getStatus() == SUCCESS) {
				//
				// Success. time to celebrate
				//
				led_flash(LED_G, 100, 50);
			} else {
				//
				// The remote XBee did not receive our packet.
				// Is it powered on?
				//

				led_flash(LED_R, 100, 50);
			}
		}
	} else if (xbee.getResponse().isError()) {
		//
		// nss.print("Error reading packet. Error code: ");
		// nss.println(xbee.getResponse().getErrorCode());
		// or flash error led
		//
		led_flash(LED_R, 100, 50);
	} else {
		//
		// Local XBee did not provide a timely TX Status Response.
		// Radio is not configured properly or connected
		//		
		led_flash(LED_R, 100, 50);
	}
}

void xbee_receive() {
	led_flash(LED_G, 500, 1);
	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		//		
		// Got something
		//
		if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
			//
			// Got a rx packet
			//
			xbee.getResponse().getRx16Response(rx16);
			xbee_decode_message(rx16);
		} else {
			// not something we were expecting
			led_flash(LED_R | LED_1, 100, 50);
		}
	} else if (xbee.getResponse().isError()) {
		led_flash(LED_R | LED_2, 100, 50);
	}
}
