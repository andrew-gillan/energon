/*
 * ADE7816.h
 *
 *  Created on: Dec 17, 2012
 *      Author: haliax
 */

#ifndef ADE7816_H_
#define ADE7816_H_

typedef struct {
	float awatthr;
	float avarhr;
	float bwatthr;
	float bvarhr;
	float cwatthr;
	float cvarhr;
	float dwatthr;
	float dvarhr;
	float ewatthr;
	float evarhr;
	float fwatthr;
	float fvarhr;
} ADE7816_energyRegisterValues_t;

int ADE7816_readRegister(const unsigned int reg);
int ADE7816_writeRegister(const unsigned int reg, const unsigned int value);
int ADE7816_writeMultipleRegisters(const char *filename);
int ADE7816_runDSP(void);
int ADE7816_stopDSP(void);
int ADE7816_readRmsRegisters(void);
int ADE7816_readEnergyRegisters(void);
int ADE7816_getEnergyRegisters(ADE7816_energyRegisterValues_t *energyRegVal);
int ADE7816_writeCalConstants(const char *filename);
int ADE7816_init(void);
//int ADE7816_init(void);

#endif /* ADE7816_H_ */
