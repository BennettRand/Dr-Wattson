#include "dataAck.h"

#include "uart.h"

uint32_t sampleCount;
int64_t powerSum[2];

#ifdef EXTENDED_DATA_PACKET
int64_t voltageSum[2];
int64_t currentSum[2];
uint16_t linePeriod;
#endif

uint16_t newSampleCount;
int64_t newPowerSum[2];

#ifdef EXTENDED_DATA_PACKET
int64_t newVoltageSum[2];
int64_t newCurrentSum[2];
#endif

int16_t adcSampleData[4];

uint8_t cycle_cnt = 0;

#if (BOARD_REV == 1)
ISR(PCINT0_vect) { // Data ready triggered
#elif (BOARD_REV == 2)
ISR(INT1_vect) {
#endif
	readData(adcSampleData,3);
	#if (BOARD_REV == 1)
	adcSampleData[0] -= deviceCalibration.channel1VoltageOffset;
	adcSampleData[2] -= deviceCalibration.channel1CurrentOffset;
	adcSampleData[3] -= deviceCalibration.channel2CurrentOffset;
	#elif (BOARD_REV == 2)
	adcSampleData[0] -= deviceCalibration.channel1CurrentOffset;
	adcSampleData[1] -= deviceCalibration.channel2CurrentOffset;
	adcSampleData[2] -= deviceCalibration.channel2VoltageOffset;
	#endif
	
	newSampleCount++;
	#if (BOARD_REV == 1)
	newPowerSum[0] += (int64_t)(((int32_t)adcSampleData[0]) * ((int32_t)adcSampleData[2]));
	newPowerSum[1] += (int64_t)(((int32_t)adcSampleData[0]) * ((int32_t)adcSampleData[3]));

	#ifdef EXTENDED_DATA_PACKET
	newVoltageSum[0] += (int64_t)(((int32_t)adcSampleData[0]) * ((int32_t)adcSampleData[0]));
	newVoltageSum[1] = newVoltageSum[0];
	newCurrentSum[0] += (int64_t)(((int32_t)adcSampleData[2]) * ((int32_t)adcSampleData[2]));
	newCurrentSum[1] += (int64_t)(((int32_t)adcSampleData[3]) * ((int32_t)adcSampleData[3]));
	#endif
	#elif (BOARD_REV == 2)
	newPowerSum[0] += (int64_t)(((int32_t)adcSampleData[2]) * ((int32_t)adcSampleData[0]));
	newPowerSum[1] += (int64_t)(((int32_t)adcSampleData[2]) * ((int32_t)adcSampleData[1]));

	#ifdef EXTENDED_DATA_PACKET
	newVoltageSum[0] += (int64_t)(((int32_t)adcSampleData[2]) * ((int32_t)adcSampleData[2]));
	newVoltageSum[1] = newVoltageSum[0];
	newCurrentSum[0] += (int64_t)(((int32_t)adcSampleData[0]) * ((int32_t)adcSampleData[0]));
	newCurrentSum[1] += (int64_t)(((int32_t)adcSampleData[1]) * ((int32_t)adcSampleData[1]));
	#endif
	#endif
	
	#if (BOARD_REV == 1)
	PCIFR = (1<<PCIF0); // Need to clear the pin change interrupt flag as we leave to clear out the rising edge interrupt on drdy.
	#endif
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

	if (cycle_cnt < 3) {
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

	cycle_cnt++;
	if (cycle_cnt == 2) { // Skip a cycle
		#if (BOARD_REV == 1)
		PCICR &= ~1;
		PCIFR = (1<<PCIF0);
		#elif (BOARD_REV == 2)
		EIMSK &= ~(1<<INT1);
		EIFR = (1<<INTF1);
		#endif
	}
	else if (cycle_cnt == 3) { // Re-enable adc
		#if (BOARD_REV == 1)
		PCICR |= 1;
		PCIFR = (1<<PCIF0);
		#elif (BOARD_REV == 2)
		EIMSK |= (1<<INT1);
		EIFR = (1<<INTF1);
		#endif
		cycle_cnt = 0;
	}
}

void initDataAck() {
	// Configure the ADC
	initADC();
	_delay_us(100);
	sendCommand(CMD_STOP);
	_delay_us(100);
	sendCommand(CMD_SDATAC);
	_delay_us(100);
	uint8_t registers[] = {0x20, 0x20, 0x20, 0x90, 0x90, 0x90, 0x90, 0x90};
	writeRegisters(0x05, registers, 8);
	_delay_us(100);

	// Configure data ready interrupt
	#if (BOARD_REV == 1)
	DDRB &= ~(1<<4);
	PCMSK0 = 1<<4;
	#elif (BOARD_REV == 2)
	DDRD &= ~(1<<1);
	EICRA |= (1<<ISC11);
	#endif

	// Configure zero cross interrupt
	EICRA |= (1<<ISC01) | (1<<ISC00);
	DDRE |= 1<<2 | 1<<3;

	// Configure period counter
	TCCR1B = (1<<CS11);
}

void stopDataAck() {
	EIMSK &= ~(1<<INT0);
	#if (BOARD_REV == 1)
	PCICR &= ~1;
	#elif (BOARD_REV == 2)
	EIMSK &= ~(1<<INT1);
	#endif
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
	#if (BOARD_REV == 1)
	PCICR |= 1;
	#elif (BOARD_REV == 2)
	EIMSK |= (1<<INT1);
	#endif
}

bool dataReady() {
	return sampleCount >= 3;
}

void getData(dataPacket_t *pkt) {
	pkt->sampleCount = sampleCount;
	pkt->powerData1 = powerSum[0];
	pkt->powerData2 = powerSum[1];
	#ifdef EXTENDED_DATA_PACKET
	pkt->linePeriod = linePeriod;
	pkt->squaredVoltage1 = voltageSum[0];
	pkt->squaredVoltage2 = voltageSum[1];
	pkt->squaredCurrent1 = currentSum[0];
	pkt->squaredCurrent2 = currentSum[1];
	#endif	
}

void removeSamples(dataPacket_t *pkt) {
	sampleCount -= pkt->sampleCount;
	powerSum[0] -= pkt->powerData1;
	powerSum[1] -= pkt->powerData2;
	#ifdef EXTENDED_DATA_PACKET
	linePeriod = 0;
	voltageSum[0] -= pkt->squaredVoltage1;
	voltageSum[1] -= pkt->squaredVoltage2;
	currentSum[0] -= pkt->squaredCurrent1;
	currentSum[1] -= pkt->squaredCurrent2;
	#endif
	pkt->sampleCount = 0;
}

