#ifdef ENERGIA
  #include "Energia.h"
#else
  #include "Arduino.h"
#endif

#include <XBee.h>

#define R_LED 10
#define G_LED  9

#define SONDE A0

#define SERIAL_SPEED 9600

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();

Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

void led_flash(int, int, int);
void led_breathe(int, uint32_t, uint32_t);

void xbee_configure();
void xbee_transmit();
void xbee_receive();
void xbee_decode_message(RxResponse&);

void setup() {
	Serial.begin(SERIAL_SPEED);
	xbee_configure();
	xbee.begin(Serial);

	pinMode(G_LED, OUTPUT);
	pinMode(R_LED, OUTPUT);
}

void loop() {
#ifdef RECEIVER
	led_flash(G_LED, 500, 1);
	xbee_receive();
#else
	led_flash(G_LED, 500, 1);
	xbee_transmit();
#endif	
}

void led_breathe(int led, uint32_t length, uint32_t count) {
	int a, i;
	int step = length / (count * 512);

	for (i=0; i<count; i++) {
		for (a=0; a<256; a++) {
			analogWrite(led, a);
			delay(step);
		}

		for (a=255; a>=0; a--) {
			analogWrite(led, a);
			delay(step);
		}
	}
}

void led_flash(int led, int length, int count) {
	int step = length / ( 2 * count );

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

void xbee_decode_message(Rx16Response& r) {
	uint8_t* data = r.getData();
	uint16_t value = data[0] << 8 | data[1];
	uint8_t label_length = data[1];
	
	char message[label_length];

	for (int i=0; i<label_length; i++)
		message[i] = static_cast<char>(data[i+2]);

	led_breathe(G_LED, 5000, 4);
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

void xbee_receive() {
	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		//		
		// Got something
		//
		if (xbee.getResponse().getApiId() == TX_16_REQUEST) {
			//
			// Got a rx packet
			//
			xbee.getResponse().getRx16Response(rx16);
			xbee_decode_message(rx16);
		} else {
			// not something we were expecting
			led_flash(R_LED, 2000, 50);
		}
	} else if (xbee.getResponse().isError()) {
		led_flash(R_LED, 2000, 50);
	}
}
