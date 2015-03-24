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
#include <FirePAN.h>

#define R_LED 10
#define G_LED  9

#define LOOP_FREQ 100

FireActor actor = FireActor(1);

void led_flash(int, uint32_t, uint8_t);

void setup() {
	actor.setSensorSource(0x00, ANALOG, A0);
	actor.setSensorType(0x00, SENSOR_LIGHT);
	actor.init();
	
	pinMode(G_LED, OUTPUT);
	pinMode(R_LED, OUTPUT);
}

void loop() {
	unsigned long time = millis();
	actor.transmit();
	
	time = millis() - time;
	delay(time > LOOP_FREQ ? 0 : LOOP_FREQ - time);
}

void FireActor::error(int e) {
	led_flash(R_LED, 200, 5);
}

void FireActor::success(int s) {
	led_flash(G_LED, 50, 1);
}

void led_flash(int led, uint32_t step, uint8_t count) {
	step /= 2;

	for (uint8_t i=0; i<count; i++) {
		digitalWrite(led, HIGH);
		delay(step);
		digitalWrite(led, LOW);
		delay(step);
	}
}

