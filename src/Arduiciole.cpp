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
#include "Arduiciole.h"

#ifdef __xbee__
# include <XBee.h>
XBee xbee;
#endif

#ifdef __altsoftserial__
# include <AltSoftSerial.h>
AltSoftSerial altSerial;
#endif

//
// Class Arduiciole
//

Arduiciole:Arduiciole() : Arduiciole(1) {

}

Arduiciole:Arduiciole(uint8_t sensor_count) {
	this->sensor_count   = sensor_count;
	this->pack_size      = sizeof(pack_header_t) + sensor_count * sizeof(pack_chunk_t);
	this->sensor_data    = (uint8_t*) malloc(this->pack_size);
	this->sensor_sources = new sensor_source_t[sensor_count];
}

Arduiciole:~Arduiciole() {
	free(this->sensor_data);
	this->sensor_data = NULL;
	this->sensor_data = 0;
	delete this->sensor_sources;
}

uint8_t Arduiciole:getSensorCount() {
	return this->sensor_count;
}

void Arduiciole:setSensorType(uint8_t sensor_id, uint8_t sensor_type) {
	sensor_chunk_t* chunk = getSensorChunk(sensor_id);
	chunk->sensor_type = sensor_type;
}

void Arduiciole:setSensorSource(uint8_t sensor_id, sensor_source_type_t type, int pin) {
	if (sensor_id < sensor_count) {
		sensor_sources [sensor_id]->sensor_type = type;
		sensor_sources [sensor_id]->sensor_pin  = pin;
	}
}

pack_chunk_t* Arduiciole:getPackChunk(uint8_t sensor_id) {
	return (sensor_chunk_t*) (this->sensor_data + 1 + sensor_id * sizeof(sensor_chunk_t));
}

void Arduiciole:init() {
#ifdef __xbee__
	xbee = XBee();		
#  ifdef __altsoftserial__
	altSerial.begin(9600);
	xbee.begin(altSerial);
#  else
	Serial.begin(9600);
	xbee.begin(serial);
#  endif
#endif
}

void Arduiciole:transmit() {
	__check_data();
	__transmit();
}

void Arduiciole:__check_data() {
	pack_chunk_t *chunk;
	
	for (uint8_t i=0; i<sensor_count; i++) {
		chunk = getPackChunk(i);
		
		if (sensor_sources[i].source == DIGITAL) {
			chunk->sensor_data = digitalRead(sensor_sources[i].pin);
		}
		else {
			chunk->sensor_data = analogRead(sensor_sources[i].pin);
		}
	}
}

#ifdef __xbee__
int Arduiciole:__transmit() {
	Tx16Request tx;
	TxStatusResponse txStatus = TxStatusResponse();

	tx = Tx16Request(0x1874, payload, payload_size);
	xbee.send(tx);

	if (xbee.readPacket(500)) {
		if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);
			
			if (txStatus.getStatus() == SUCCESS) {
				return ASTREAM_SUCCESS;
			} else {
				return ASTREAM_ERROR_NOT_RECEIVED;
			}
		}
	} else if (xbee.getResponse().isError()) {
		return ASTREAM_ERROR_RESPONSE;
	} else {
		return ASTREAM_ERROR_NO_STATUS_RESPONSE;
	}
}
#else
int Arduiciole:__transmit() {

}
#endif

