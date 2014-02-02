#include "dataAck.h"

#include "uart.h"

//------------ These values are used externally to access power data ------------
uint32_t sampleCount;
int64_t powerSum[2];

#ifdef EXTENDED_DATA_PACKET
int64_t voltageSum[2];
int64_t currentSum[2];
uint16_t linePeriod;
#endif
//-------------------------------------------------------------------------------

uint16_t newSampleCount;
int64_t newPowerSum[2];

#ifdef EXTENDED_DATA_PACKET
int64_t newVoltageSum[2];
int64_t newCurrentSum[2];
#endif

int16_t adcSampleData[4];

ISR(PCINT0_vect) { // Data ready triggered
	readData(adcSampleData,4);
	adcSampleData[0] -= deviceCalibration.channel1VoltageOffset;
	adcSampleData[1] -= deviceCalibration.channel2VoltageOffset;
	adcSampleData[2] -= deviceCalibration.channel1CurrentOffset;
	adcSampleData[3] -= deviceCalibration.channel2CurrentOffset;
	
	newSampleCount++;
	newPowerSum[0] += (int64_t)(((int32_t)adcSampleData[0]) * ((int32_t)adcSampleData[2]));
	newPowerSum[1] += (int64_t)(((int32_t)adcSampleData[1]) * ((int32_t)adcSampleData[3]));

	#ifdef EXTENDED_DATA_PACKET
	newVoltageSum[0] += (int64_t)(((int32_t)adcSampleData[0]) * ((int32_t)adcSampleData[0]));
	newVoltageSum[1] += (int64_t)(((int32_t)adcSampleData[1]) * ((int32_t)adcSampleData[1]));
	newCurrentSum[0] += (int64_t)(((int32_t)adcSampleData[2]) * ((int32_t)adcSampleData[2]));
	newCurrentSum[1] += (int64_t)(((int32_t)adcSampleData[3]) * ((int32_t)adcSampleData[3]));
	#endif
	
	PCIFR = (1<<PCIF0); // Need to clear the pin change interrupt flag as we leave to clear out the rising edge interrupt on drdy.
}

ISR(INT0_vect) {
	if (TIFR1 & (1<<TOV1)){ // Coutner rolled over, throw out the data
		linePeriod = 0xFFFF;
		TIFR1 &= (1<<TOV1);
	}
	else {
		linePeriod = TCNT1;
		TCNT1 = 0;
	}

	sampleCount += newSampleCount;
	powerSum[0] += newPowerSum[0];
	powerSum[1] += newPowerSum[1];
	newSampleCount = 0;
	newPowerSum[0] = 0;
	newPowerSum[1] = 0;

	#ifdef EXTENDED_DATA_PACKET
	voltageSum[0] += newVoltageSum[0];
	voltageSum[1] += newVoltageSum[1];
	newVoltageSum[0] = 0;
	newVoltageSum[1] = 0;
	currentSum[0] += newCurrentSum[0];
	currentSum[1] += newCurrentSum[1];
	newCurrentSum[0] = 0;
	newCurrentSum[1] = 0;
	#endif
}

void initDataAck() {
	// Configure the ADC
	initADC();
	_delay_us(100);
	sendCommand(CMD_STOP);
	_delay_us(100);
	sendCommand(CMD_SDATAC);
	_delay_us(100);
	uint8_t registers[] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0x90, 0x90};
	writeRegisters(0x05, registers, 8);
	_delay_us(100);

	// Configure data ready interrupt
	DDRB &= ~(1<<4);
	PCMSK0 = 1<<4;

	// Configure zero cross interrupt
	EICRA |= (1<<ISC01) | (1<<ISC00);
	DDRE |= 1<<2 | 1<<3;

	// Configure period counter
	TCCR1B = (1<<CS11);
		
}

void stopDataAck() {
	EIMSK &= ~(1<<0);
	PCICR &= ~1;
	_delay_us(100);
	sendCommand(CMD_STOP);
	_delay_us(100);
	sendCommand(CMD_SDATAC);
	_delay_us(100);
}


void startDataAck() {
	_delay_us(100);
	sendCommand(CMD_START);
	_delay_us(100);
	sendCommand(CMD_RDATAC);
	_delay_us(100);
	EIMSK |= 1<<0;
	PCICR |= 1;
}

