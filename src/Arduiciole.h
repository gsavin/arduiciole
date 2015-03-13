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
#ifndef _ARDUICIOLE_H_
#define _ARDUICIOLE_H_

#include "ArduicioleSensorType.h"

#define MAX_DATA_SIZE 100

#define ASTREAM_SUCCESS 			 0
#define ASTREAM_ERROR_NOT_RECEIVED		-1
#define ASTREAM_ERROR_RESPONSE 			-2
#define ASTREAM_ERROR_NO_STATUS_RESPONSE	-3

typedef enum { ANALOG, DIGITAL } sensor_source_type_t;

typedef struct {
	uint8_t  arduiciole_chunk_count
} pack_header_t;

typedef struct {
	uint8_t  sensor_id,
	uint8_t  sensor_type,
	uint32_t sensor_data
} pack_chunk_t;

typedef struct {
	sensor_source_type_t 	source;
	int 			pin;
} sensor_source_t;

class Arduiciole {
public:
	Arduiciole();
	Arduiciole(uint8_t);
	~Arduiciole();
	
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
	
	void init();
	
	void error(int);
	void success(int);
private:
	uint16_t	 pack_size;
	uint8_t  	 sensor_count;
	uint8_t* 	 sensor_data;
	sensor_source_t* sensor_sources;
	
	pack_chunk_t* getSensorChunk(uint8_t);
	
	int __transmit();
	void __check_data();
};

#endif /* _ARDUICIOLE_H_ */

