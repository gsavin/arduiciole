/**
 * Copyright (c) 2015 Guilhelm Savin. All rights reserved.
 *
 * This file is part of FireActor.
 *
 * FireActor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FireActor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "FirePAN.h"

#ifdef __altsoftserial__
# include <AltSoftSerial.h>
AltSoftSerial altSerial;
#endif

#ifdef __xbee__
# include <XBee.h>
XBee xbee;
#  ifdef __altsoftserial__
#    define __xbee_serial__ altSerial
#  else
#    define __xbee_serial__ Serial
#  endif
#endif

#ifdef __yun__
# include <Bridge.h>
#endif

#ifndef XBEE_STATUS_RESPONSE_DELAY
# define XBEE_STATUS_RESPONSE_DELAY 500
#endif

//
// Class FireActor
//

FireActor::FireActor(){
	FireActor(1);
}

FireActor::FireActor(uint8_t sensor_count) {
	this->sensor_count   = sensor_count;
	this->pack_size      = sizeof(pack_header_t) + sensor_count * sizeof(pack_chunk_t);
	this->sensor_data    = (uint8_t*) malloc(this->pack_size);
	this->sensor_sources = new sensor_source_t[sensor_count];
}

FireActor::~FireActor() {
	free(this->sensor_data);
	this->sensor_data = NULL;
	this->sensor_data = 0;
	delete this->sensor_sources;
}

uint8_t FireActor::getSensorCount() {
	return this->sensor_count;
}

void FireActor::setSensorType(uint8_t sensor_id, uint8_t sensor_type) {
	pack_chunk_t *chunk = getPackChunk(sensor_id);
	chunk->sensor_type  = sensor_type;
}

void FireActor::setSensorSource(uint8_t sensor_id, sensor_source_type_t type, int pin) {
	if (sensor_id < sensor_count) {
		sensor_sources [sensor_id].type = type;
		sensor_sources [sensor_id].pin  = pin;
	}
}

pack_chunk_t* FireActor::getPackChunk(uint8_t sensor_id) {
	return &(((pack_t*) this->sensor_data)->chunks[sensor_id]);
	//return (pack_chunk_t*) (this->sensor_data + 1 + sensor_id * sizeof(pack_chunk_t));
}



void FireActor::init() {
#ifdef __xbee__
	xbee = XBee();		
	__xbee_serial__.begin(9600);
	xbee.begin(__xbee_serial__);

	char thisByte = 0;

	__xbee_serial__.print("+++");

	while (thisByte != '\r' && thisByte != '\n') {
		if (__xbee_serial__.available() > 0)
			thisByte = __xbee_serial__.read(); 
	}
	
#  ifdef __xbee_serie_1__
	__xbee_serial__.print("ATRE\r");
	__xbee_serial__.print("ATAP2\r");
	__xbee_serial__.print("ATCE");
	__xbee_serial__.print(__xbee_ce__); 
	__xbee_serial__.print("\r"); 
	__xbee_serial__.print("ATMY");
	__xbee_serial__.print(ACTOR_ADDRESS);
	__xbee_serial__.print("\r"); 
	__xbee_serial__.print("ATID");
	__xbee_serial__.print(ACTOR_NETWORK);
	__xbee_serial__.print("\r"); 
	__xbee_serial__.print("ATCH0C\r");
	__xbee_serial__.print("ATCN\r");
#  else
	// TODO XBee Serie 2
#  endif
#endif

#ifdef __yun__
	Bridge.begin();
#endif
}

void FireActor::transmit() {
	__check_data();
	int r = __transmit();
	
	if (r < 0)
		error(r);
	else
		success(r);
}

void FireActor::__check_data() {
	pack_chunk_t *chunk;
	
	for (uint8_t i=0; i<sensor_count; i++) {
		chunk = getPackChunk(i);
		
		if (sensor_sources[i].type == DIGITAL) {
			chunk->sensor_data = digitalRead(sensor_sources[i].pin);
		}
		else {
			chunk->sensor_data = analogRead(sensor_sources[i].pin);
		}
	}
}

void FireActor::__handle_remote_pack(uint8_t *data) {
	pack_t *pack = (pack_t*) data;
	
	for (uint8_t i=0; i < pack->header.chunk_count; i++)
		__handle_remote_chunk(pack->header.id, pack->chunks[i]);
}

void FireActor::__handle_remote_chunk(actor_id_t from, pack_chunk_t& chunk) {
#ifdef __yun__
	Process p;
	p.begin(FIREPAN_DB_CMD);
	p.addParameter("add");
	p.addParameter(from);
	p.addParameter(chunk->sensor_id);
	p.addParameter(chunk->sensor_type);
	p.addParameter(chunk->sensor_data);
	p.run();
#endif
}

#if defined(__xbee__)
int FireActor::__transmit() {
	Tx16Request tx;
	TxStatusResponse txStatus = TxStatusResponse();

	tx = Tx16Request(__xbee_coordinator__, sensor_data, sizeof(pack_header_t) + sensor_count * sizeof(pack_chunk_t));
	xbee.send(tx);

	if (xbee.readPacket(XBEE_STATUS_RESPONSE_DELAY)) {
		if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);
			
			if (txStatus.getStatus() == SUCCESS) {
				return TRANSMIT_SUCCESS;
			} else {
				return TRANSMIT_ERROR_NOT_RECEIVED;
			}
		}
	} else if (xbee.getResponse().isError()) {
		return TRANSMIT_ERROR_RESPONSE;
	}
	
	return TRANSMIT_ERROR_NO_STATUS_RESPONSE;
}

void FireActor::receive() {
	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == RX_16_RESPONSE ||
			xbee.getResponse().getApiId() == RX_64_RESPONSE) {
			if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
				Rx16Response rx = Rx16Response();
				xbee.getResponse().getRx16Response(rx);
				__handle_remote_pack(rx.getData());
			} else {
				Rx64Response rx = Rx64Response();
				xbee.getResponse().getRx64Response(rx);
				__handle_remote_pack(rx.getData());
			}
			
			success(0);
		} else {
			error(-1);
		}
	} else if (xbee.getResponse().isError()) {
		error(-2);
	}
}

#elif defined(__bluetooth__)
int FireActor::__transmit() {
	// TODO
	return TRANSMIT_ERROR_NO_DEVICE;
}

void FireActor::receive() {
	// TODO
}

#elif defined(__wifi__)
int FireActor::__transmit() {
	// TODO
	return TRANSMIT_ERROR_NO_DEVICE;
}

void FireActor:receive() {
	// TODO
}

#elif defined(__rf433__)
int FireActor::__transmit() {
	// TODO
	return TRANSMIT_ERROR_NO_DEVICE;
}

void FireActor::receive() {
	// TODO
}

#else
int FireActor::__transmit() {
	return TRANSMIT_ERROR_NO_DEVICE;
}

void FireActor::receive() {
	// Easy life, nothing to do
}

#endif

