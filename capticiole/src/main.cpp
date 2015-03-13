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

#include <XBee.h>

#define R_LED 10
#define G_LED  9

#define SONDE A0

XBee xbee = XBee();

void led_flash(int, uint32_t, uint32_t);

void xbee_configure();
void xbee_transmit();

void setup() {
	Serial.begin(9600);
	xbee_configure();
	xbee.begin(Serial);

	pinMode(G_LED, OUTPUT);
	pinMode(R_LED, OUTPUT);
}

void loop() {
	led_flash(G_LED, 500, 1);
	xbee_transmit();
}

void led_flash(int led, uint32_t step, uint32_t count) {
	step /= 2;

	for (int i=0; i<count; i++) {
		digitalWrite(led, HIGH);
		delay(step);
		digitalWrite(led, LOW);
		delay(step);
	}
}

void xbee_configure() {
	char thisByte = 0;

	Serial.print("+++");

	while (thisByte != '\r' && thisByte != '\n') {
		if (Serial.available() > 0)
			thisByte = Serial.read(); 
	}

	Serial.print("ATRE\r");
	Serial.print("ATAP2\r");
	Serial.print("ATCE0\r");
	Serial.print("ATMY");
	Serial.print(BEE_ADDRESS);
	Serial.print("\r"); 
	Serial.print("ATID1111\r");
	Serial.print("ATCH0C\r");
	Serial.print("ATCN\r");
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
				led_flash(G_LED, 500, 10);
			} else {
				//
				// The remote XBee did not receive our packet.
				// Is it powered on?
				//
				led_flash(R_LED, 2000, 50);
			}
		}
	} else if (xbee.getResponse().isError()) {
		//
		// nss.print("Error reading packet. Error code: ");
		// nss.println(xbee.getResponse().getErrorCode());
		// or flash error led
		//
		led_flash(R_LED, 2000, 50);
	} else {
		//
		// Local XBee did not provide a timely TX Status Response.
		// Radio is not configured properly or connected
		//
		led_flash(R_LED, 1000, 50);
	}
}

