#ifndef __MAX471_H
#define __MAX471_H

void TEST_ADC_Init(void);
void TEST_ADC_ADCGet(void);
int TEST_ADC_GetVoltage_mv(void);

extern uint16_t adc_DATA;

#endif
