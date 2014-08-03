/*
 * main implementation: use this 'C' sample to create your own application
 *
 */

#include "derivative.h" /* include peripheral declarations */
#include <stdio.h>

#define RED         (18)
#define RED_SHIFT   (1<<RED)

#define BLUE         (1)
#define BLUE_SHIFT   (1<<BLUE)

#define RED_OFF     (GPIOB_PSOR = RED_SHIFT)
#define RED_ON      (GPIOB_PCOR = RED_SHIFT)
#define RED_TOGGLE  (GPIOB_PTOR = RED_SHIFT)

#define BLUE_OFF     (GPIOD_PSOR = BLUE_SHIFT)
#define BLUE_ON      (GPIOD_PCOR = BLUE_SHIFT)
#define BLUE_TOGGLE  (GPIOD_PTOR = BLUE_SHIFT)

static void InitLED(void) {
  /* Turn on clock to PortB module */
  SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
  /* Turn on clock to PortD module */
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;


  /* Set the PTB18 pin multiplexer to GPIO mode */
  PORTB_PCR18 = PORT_PCR_MUX(1);

  // Set PTD1 pin mux to GPIO mode
  PORTD_PCR1 = PORT_PCR_MUX(1);

  /* Set the initial output state to high */
  GPIOB_PSOR |= RED_SHIFT;
  GPIOD_PSOR |= BLUE_SHIFT;
  /* Set the pins direction to output */
  GPIOB_PDDR |= RED_SHIFT;
  GPIOD_PDDR |= BLUE_SHIFT;

}

static void calADC(void){
	int16_t calVal;
	int ii;

	ii=0;
	ADC0_SC3|=0x87; /* 0000 0000 0000 0000 0000 0000 1000 0111 : Start calibration, single conversion, enable hardware averaging to 32 samples averaged */
	do
	{
		ii++;
	}  while ((ADC0_SC1A && (1<<7))==0);
	calVal=ADC0_CLP0 + ADC0_CLP1 + ADC0_CLP2 + ADC0_CLP3 + ADC0_CLP4 + ADC0_CLPS;
	calVal=calVal>>1; // Divide by 2
	calVal|=(1<<15); // Set MSB
	ADC0_PG=calVal;
	calVal=ADC0_CLM0 + ADC0_CLM1 + ADC0_CLM2 + ADC0_CLM3 + ADC0_CLM4 + ADC0_CLMS;
	calVal=calVal>>1; // Divide by 2
	calVal|=(1<<15); // Set MSB
	ADC0_MG=calVal;
}

static void InitADC(void){

	int abc;
	int sizeofabc;

	sizeofabc=sizeof(abc);
	// Turn on clock for Port E module (PTE20 --> J10 01 --> ADC0_SE0/ADC0_DP0)
	SIM_SCGC6|= SIM_SCGC6_ADC0_MASK; // Enable clock for ADC0
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	PORTE_PCR20 = PORT_PCR_MUX(0);

	calADC(); // Calibrate ADC
	ADC0_CFG1  	= 0x1C;  // 0b0,00,1,11,00 : Normal power, /1 clk, long sample time, 16b SE, bus clk
	ADC0_SC1A  	= 0x20;  // 0b0,1,0,00000 : COCO, enable conversion complete interrupt, SE conversion, select ADC channel 0 (which connects to J10 pin 02)
	ADC0_CFG2 	= 0x01; // 0b0,0,0,0,01 : MUX select ADxxa channels, disable asynch clock output, normal conversion speed, long sample time select



	ADC0_SC2 	= 0x00; 	// 0b0,0,0,0,0,0,00 : Conversion active flag, Hardware trigger,  compare disable, compare disable, compare disable, DMA disable, Default voltage ref

	// ADC0_SC3|=0x48; /* 0000 0000 0000 0000 0000 0000 1000 0111 : Start calibration, single conversion, enable hardware averaging to 32 samples averaged */

}

void ADC0_IRQHandler(void)
{
	int xx=0;

	xx++;

}

int singleCapture(void){

	int ii=0;
	ADC0_SC3 =0x48; // 0b0,0,00,0,1,11 : no cal, cal flag, reserved, one conversion, hw avg enabled, avg 32 samples
	ADC0_SC1A  = 0x00;  // 0b0,0,0,00000 : COCO, disable conversion complete interrupt, SE conversion, select ADC channel 0 (which connects to J10 pin 02)
	do
	{
		ii++;
	}  while ((ADC0_SC1A & (1<<7))==0);


	return ADC0_RA;
}

static void NegLED(void) {
  RED_TOGGLE;
  BLUE_TOGGLE;
}

static void wait(void) {
  volatile int i;
  for(i=0;i<8192;i++){}
}

int main(void) {
  int counter = 0;
  int buffer[1024];

  InitLED();
  InitADC();
  for(;;) {	   
    counter++;

    buffer[counter%1024]=singleCapture()-32768;
    // wait();
    if ((counter%1024)==0) { /* blink LED slowly so it is better visible */
      NegLED();
    }
    // buffer[counter%64]=counter;
    /* printf("Enter a string:\r\n");

    if (gets(buffer)!=NULL)
    {
		printf("you entered: %s\r\n", buffer);
    }
    */
  }	
  return 0;
}
