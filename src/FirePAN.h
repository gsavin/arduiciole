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
#ifndef _FIREPAN_H_
#define _FIREPAN_H_

#ifdef ENERGIA
  #include "Energia.h"
#else
  #include "Arduino.h"
#endif

#include "FirePANSensorType.h"

#define MAX_DATA_SIZE 100

#define TRANSMIT_SUCCESS 			 0
#define TRANSMIT_ERROR_NOT_RECEIVED		-1
#define TRANSMIT_ERROR_RESPONSE 		-2
#define TRANSMIT_ERROR_NO_STATUS_RESPONSE	-3
#define TRANSMIT_ERROR_NO_DEVICE		-4

#define FIREPAN_DB_CMD "firepan_db.py"

typedef enum { ANALOG, DIGITAL } sensor_source_type_t;

typedef uint32_t actor_id_t;

struct pack_header_t {
	actor_id_t	id;
	uint8_t  	chunk_count;
};

struct pack_chunk_t {
	uint8_t  sensor_id;
	uint8_t  sensor_type;
	uint32_t sensor_data;
};

struct pack_t {
	pack_header_t header;
	pack_chunk_t *chunks;
};

struct sensor_source_t {
	sensor_source_type_t 	type;
	int 			pin;
};

class FireActor {
public:
	FireActor();
	FireActor(uint8_t);
	~FireActor();
	
	/**
	 * The number of sensors on this arduiciole.
	 */
	uint8_t getSensorCount();
	
	/**
	 * Define the type of the sensor. All types are defined in ArduicioleSensorType.h.
	 */
	void setSensorType(uint8_t, uint8_t);
	
	/**
	 * Define where the data from this sensor has to be read.
	 */
	void setSensorSource(uint8_t, sensor_source_type_t, int);
	
	void transmit();
	void receive();
	
	void init();
	
	void error(int);
	void success(int);
private:
	uint16_t	 pack_size;
	uint8_t  	 sensor_count;
	uint8_t* 	 sensor_data;
	sensor_source_t* sensor_sources;
	
	pack_chunk_t* getPackChunk(uint8_t);
	
	int __transmit();
	void __check_data();
	void __handle_remote_pack(uint8_t*);
	void __handle_remote_chunk(actor_id_t, pack_chunk_t&);
};

#endif /* _FIREPAN_H_ */

