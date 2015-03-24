/**
 * Copyright (c) 2015 Guilhelm Savin. All rights reserved.
 *
 * This file is part of Arduiciole.
 *
 * Arduiciole is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arduiciole is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef ENERGIA
  #include "Energia.h"
#else
  #include "Arduino.h"
#endif

#include <AltSoftSerial.h>
#include <XBee.h>
#include <Bridge.h>

#define DATA 6
#define LATCH 7
#define CLOCK 10

#define XBEE_RX_PIN 13
#define XBEE_TX_PIN 5

#define LED_NONE 0x00
#define LED_G 0x01
#define LED_R 0x02
#define LED_1 0x04
#define LED_2 0x08
#define LED_3 0x10
#define LED_4 0x20
#define LED_5 0x40
#define LED_6 0x80

XBee xbee = XBee();
Rx16Response rx16 = Rx16Response();

AltSoftSerial altSerial;

int leds;

void shift_led(int);
void led_flash(int, int, int);

void xbee_configure();
void xbee_receive();
void xbee_decode_message(RxResponse&);

void setup() {
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

void shift_led(int value) {
	digitalWrite(LATCH, LOW);
	shiftOut(DATA, CLOCK, MSBFIRST, value);
	digitalWrite(LATCH, HIGH);
	
	leds = value;
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
	
	Bridge.put(message, String(value));
	
	shift_led(level_led);
	led_flash(LED_G, 100, 5);
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
