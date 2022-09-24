#include "otd_CorePeri.h"

#ifdef __cplusplus
extern "C"{
#endif 

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <avr/wdt.h>



/*
 * DECLARATION OF STATIC FUNCTIONS
 */
static void init_timer_adc();
static int uart_init();
void uart_delay();
static char uart_getch();
//#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}
#define Reset_AVR() while(1) {}

/*
 * TIMER (ADC) DEFINITIONS
 */
#define ADC_CH_AMP0					11
/*
 * ::: NOTE :::	I/O PLL input clock can be maximum 8MHz. So, by using Fuse bits internal RC OSC was used.
 * 				The ADC prescaler value "128" was used. Hence, in free running mode ADC conversion takes
 * 				"16" ADC clock cycles (8Mhz/128) results in 4KHz ADC completed interrupt.
 */
#define ADC_SAMPLING_PERIOD_US		128
unsigned long uptime_tick	= 0;




/*
 * UART DEFINITIONS
 */
#define UART_BAUD					19200
#define UART_DDR					DDRD
#define UART_PORT					PORTD
#define UART_PIN					PIND
#define UART_RX						2
#define UART_TX						5
//
static uint8_t sDelayCount = 0;
static uint8_t sTxMask = 0;
static uint8_t sTxUnmask = 0;
static uint8_t sRxMask = 0;
//
#define UART_BUF_SIZE	64
#define WRAP(x, y) if (++(x) == (y)) x = 0
struct uart_buffer {
	unsigned char buffer[UART_BUF_SIZE];
	volatile  unsigned char size;
	unsigned char in;
	unsigned char out;
};
static struct uart_buffer sRx_stream;




/*
 * CORE PERIPHERAL FUNCTIONS
 */

void otd_InitCorePeri(){

	// Disbale watchdog
	wdt_disable();

	// Used for timing
	init_timer_adc();
	// Used for communication
	uart_init();

	// Enable interrupts
	sei();
	return;
}



#include <avr/io.h>
void otd_SoftReset(){

	// Enable soft reset
	wdt_enable(WDTO_30MS);

	// Enter the watchdog trigger loop
	Reset_AVR();
	return;
}





/*
 * TIMER (ADC) FUNCTIONS
 */
unsigned long getUptime_ms(){
	/*
	 * ::: NOTE :::	ADC sampling period for 8MHz inter OSC Rc clock with prescaler 2 is ~4us.
	 * 				4 = (1000×1000÷(8×1000×1000÷2÷16))
	 */
	/*
	 * ::: NOTE :::	ADC runs a little bit faster so "1028" is for calibration
	 */
	return (uptime_tick*ADC_SAMPLING_PERIOD_US/1000);
}

static void init_timer_adc(){

	/*
	 * ::: NOTE :::	We are using ADC as a timer. Becuase the AT90PWM161 does not have a usable timer
	 */
	// First configure ADC "Digital Input Disable Register"
	DIDR0 = 0;			// :::No channel for ADC sampling
	DIDR1 = 0;



	// As DIDR of the adc input is not disabled. Any value should be fine. Set ADC Multiplexer Register as regular
	ADMUX  = _BV(REFS1) | _BV(REFS0)	// Internal 2.56V reference voltage with  PE3 pin free as port
				| _BV(ADLAR)   			// left adjust result
				| ADC_CH_AMP0;       	// initial input channel = AMP0

	//
	ADCSRB = 0 | _BV(ADHSM);	// high speed mode and auto trigger source = self


	ADCSRA = _BV(ADEN)    		// enable ADC
				| _BV(ADATE)   	// enable auto trigger
				| _BV(ADIE)    	// enable interrupt
				| _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);	// prescaler = 128
	ADCSRA &= ~(1<<ADIF);		// clear interrupt flag


	/*
	 * ::: NOTE :::	If we do not enable Amplifier, ADC conversion does not start.
	 */
	AMP0CSR = _BV(AMP0EN)		// Enable Amplifier
				| _BV(AMP0GS);	// Use ground instead of AMP0-

	_delay_us(9);         		// let the conversion start

	sei();

	ADCSRA |= (1<<ADSC);		// Start conversion

	return;
}
// ADC interrupt service routine.
ISR(ADC_vect)
{
	// Increment tick counter
	uptime_tick = uptime_tick +1;

}






/*
 * UART FUCNTIONS
 */
void otd_UartPrintByte(uint8_t inData){

	uint8_t oldSREG = SREG;
	cli(); //Prevent interrupts from breaking the transmission. Note: TinySoftwareSerial is half duplex.
	//it can either recieve or send, not both (because recieving requires an interrupt and would stall transmission
	__asm__ __volatile__ (
		"   com %[ch]\n" // ones complement, carry set
		"   sec\n"
		"1: brcc 2f\n"					// Branch if Carry Cleared
		"   in r23,%[uartPort] \n"
		"   and r23,%[uartUnmask]\n"
		"   out %[uartPort],r23 \n"
		"   rjmp 3f\n"
		"2: in r23,%[uartPort] \n"
		"   or r23,%[uartMask]\n"
		"   out %[uartPort],r23 \n"
		"   nop\n"
		"3: rcall uart_delay\n"
		"   rcall uart_delay\n"
		"   rcall uart_delay\n"
		"   rcall uart_delay\n"
		"   lsr %[ch]\n"
		"   dec %[count]\n"
		"   brne 1b\n"
		:
		:
		  [ch] "r" (inData),
		  [count] "r" ((uint8_t)10),
		  [uartPort] "I" (_SFR_IO_ADDR(UART_PORT)),
		  [uartMask] "r" (sTxMask),
		  [uartUnmask] "r" (sTxUnmask)
		: "r23",
		  "r24",
		  "r25"
	);
	SREG = oldSREG;
	sei();
}

void otd_UartPrint(char *inStr){
	while(*inStr) otd_UartPrintByte(*inStr++);
}


void otd_UartPrintInt(int inData){
	char tmpStr[16];
	memset(tmpStr, 0, sizeof(char)*16);
	itoa(inData, tmpStr, 10);
	otd_UartPrint(tmpStr);
}

void otd_UartPrintFloat(float inData){

	char tmpStr[32];
	memset(tmpStr, 0, sizeof(char)*32);
	dtostrf(inData, 10, 5, tmpStr);
	otd_UartPrint(tmpStr);
}

void otd_UartPrintln(char *inStr){
	otd_UartPrint(inStr);
	otd_UartPrintByte('\n');
}

int16_t otd_UartRead(uint8_t inCanBlock){
	if (inCanBlock == 1){
		while (sRx_stream.size <= 0);
		char c = *(sRx_stream.buffer + sRx_stream.out);
		WRAP(sRx_stream.out, UART_BUF_SIZE);
		--sRx_stream.size;
		return c;
	}else{
		if(sRx_stream.size <= 0){
			return -1;
		}
		char c = *(sRx_stream.buffer + sRx_stream.out);
		WRAP(sRx_stream.out, UART_BUF_SIZE);
		--sRx_stream.size;
		return c;
	}
}


void otd_UartWrite(uint8_t inData){
	otd_UartPrintByte(inData);
	return;
}




static int uart_init(){


	// Reset software uart buffer
	memset(&sRx_stream, 0, sizeof(struct uart_buffer));

	// Calculate delay count
	long tempDelay = (((F_CPU/UART_BAUD)-39)/12);
	if ((tempDelay > 255) || (tempDelay <= 0)){
		return -1; //Cannot start as it would screw up uartDelay().
	}
	sDelayCount = (uint8_t)tempDelay;


	// Configure TX pin
	UART_DDR  |= (1<<UART_TX);
	UART_PORT |= (1<<UART_TX);
	// Set mask unmask byte
	sTxMask = _BV(UART_TX);
	sTxUnmask = ~sTxMask;

	// Configure RX pin
	UART_DDR &= ~(1 << UART_RX);
	//
	sRxMask = _BV(UART_RX);


	/* ANALOG COMPARATOR 1 INIT */
	AC1CON = 0;
	AC1ECON = 0;
	// ::: NOTE ::: In initADC() REFS0 and REFS1 bits are cleared. Internal 2.56V reference voltage with  PE3 pin free as port
	// Vbg Bandgap reference voltage is 1.1V
	AC1CON |= (1 << AC1M2);	//enable the internal bandgap reference - used instead of AIN0 to allow it to be used for TX.
	//
	AC1CON |= (1 << AC1IS1);  //interrupt on falling edge (this means RX has gone from Mark state to Start bit state).
	//
	AC1CON |= (1 << AC1IE);  //turn on the comparator interrupt to allow us to use it for RX
	AC1CON |= (1 << AC1EN);	//turn on the comparator for RX

	// Enable digital input register
	DIDR0 &= ~(1 << ADC0D);

	return 0;
}

void uart_delay() {
	__asm__ __volatile__ (
	  "mov r25,%[count]\n"
	  "1:dec r25\n"
      "brne 1b\n"
      "ret\n"
	  ::[count] "r" ((uint8_t)sDelayCount)
	);
}



ISR(ANALOG_COMP_1_vect){
	// Check wrong detection. Note that uart has start bit value "0"
	if ( (PIND & sRxMask) == sRxMask){
		return;
	}

	char ch = uart_getch();

	sRx_stream.buffer[sRx_stream.in] = ch;
	WRAP(sRx_stream.in, UART_BUF_SIZE);
	++sRx_stream.size;

}


static char uart_getch() {
  uint8_t ch = 0;
    __asm__ __volatile__ (
		"   rcall uart_delay\n"          // Get to 0.25 of start bit (our baud is too fast, so give room to correct)
		"1: rcall uart_delay\n"              // Wait 0.25 bit period
		"   rcall uart_delay\n"              // Wait 0.25 bit period
		"   rcall uart_delay\n"              // Wait 0.25 bit period
		"   rcall uart_delay\n"              // Wait 0.25 bit period
		"   clc\n"
		"   in r23,%[pin]\n"
		"   and r23, %[mask]\n"
		"   breq 2f\n"
		"   sec\n"
		"2: ror   %0\n"
		"   dec   %[count]\n"
		"   breq  3f\n"
		"   rjmp  1b\n"
		"3: rcall uart_delay\n"              // Wait 0.25 bit period
		"   rcall uart_delay\n"              // Wait 0.25 bit period
		:
		  "=r" (ch)
		:
		  "0" ((uint8_t)0),
		  [count] "r" ((uint8_t)8),
		  [pin] "I" (_SFR_IO_ADDR(UART_PIN)),
		  [mask] "r" (sRxMask)
		:
		  "r23",
		  "r24",
		  "r25"
    );
	return ch;
}



enum LAST_RESET_TYPE getLastResetCause(){

	enum LAST_RESET_TYPE tmpResetType = LAST_RESET_NONE;
	if (MCUSR & (1 << PORF)) {        // Power-on Reset
		tmpResetType = LAST_RESET_POWERON;
	}
	else if (MCUSR & (1 << EXTRF)) {  // External Reset
		tmpResetType = LAST_RESET_EXT;
	}
	else if (MCUSR & (1 << BORF)) {   // Brown-Out Reset
		tmpResetType = LAST_RESET_BROWNOUT;
	}
	else if (MCUSR & (1 << WDRF)) {   // Watchdog Reset
		tmpResetType = LAST_RESET_WATCHDOG;
	}

	// Reset MCU status register. To avoid further watchdog resets
	MCUSR = 0;

	return tmpResetType;
}

#ifdef __cplusplus
}
#endif

