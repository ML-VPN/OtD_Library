#include "otd_DigitalIO.h"

#ifdef __cplusplus
extern "C"{
#endif 


#include <avr/io.h>
#include <util/delay.h>
#include "otd_CorePeri.h"


#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
// Output enable pin
#define DIGOUT_EN_PORT		PORTD
#define DIGOUT_EN_PIN		4


/*
 * DECLARATIONS
 */
static uint8_t reverse_bit8(uint8_t x);
static void I2C_Init();
static void I2C_Start();
static void I2C_Stop();
static uint8_t I2C_Write_Byte(unsigned char IIC_Byte);
static void I2C_Read_Byte( uint8_t *);
//
static void init_digital_output();


void otd_InitDigitalIO(){
	/* Initialize Digital Input */
	I2C_Init();

	/* Initialize Digital Output */
	init_digital_output();

	return;
}

uint8_t otd_DigitalRead(enum DIGITAL_INPUT_PINS inInputPin){

	uint8_t tmpByte = otd_DigitalReadAll();
	if (tmpByte & (1 << inInputPin)){
		return 1;
	}

	return 0;
};

uint8_t otd_DigitalReadAll(){

	uint8_t addrByte;
	uint8_t valByte;
	uint8_t retVal;

	/*
	 * ::: NOTE :::	We do not know the current state of the IC so, we send a "STOP" mark
	 */
	I2C_Stop();
	I2C_Start();
	addrByte = 0x40;
	BIT_SET(addrByte,0);
	retVal = I2C_Write_Byte(addrByte);
	if (retVal == 0){
		otd_UartPrintln("NO ACK !!!");
		I2C_Stop();
		return 0;
	}
	// Read port of the device
	I2C_Read_Byte(&valByte);
	I2C_Stop();

	// Due to the input interface IC, digital input LSB is sent first. So, reverse input byte
	valByte = reverse_bit8(valByte);
	// Note that the input values are inverted due to the digital input circuitry
	return ~valByte;
};


void otd_DigitalWrite(enum DIGITAL_OUTPUT_PINS inOutputPin, uint8_t inValue){

	volatile uint8_t *curPort;
	uint8_t curPin;

	switch(inOutputPin){
	case DIGITAL_OUTPUT_1:
		curPort = &PORTB;
		curPin = 7;
		break;

	case DIGITAL_OUTPUT_2:
		curPort = &PORTB;
		curPin = 0;
		break;

	case DIGITAL_OUTPUT_3:
		curPort = &PORTB;
		curPin = 2;
		break;

	case DIGITAL_OUTPUT_4:
		curPort = &PORTB;
		curPin = 3;
		break;

	case DIGITAL_OUTPUT_5:
		curPort = &PORTD;
		curPin = 6;
		break;

	case DIGITAL_OUTPUT_6:
		curPort = &PORTD;
		curPin = 3;
		break;

	default:
		return;
	}

	// Set pin value
	if (inValue == 0){
		*curPort &= ~(1 << curPin);
	}else{
		*curPort |= (1 << curPin);
	}

	return;
}



uint8_t otd_GetDigitalWriteState(enum DIGITAL_OUTPUT_PINS inOutputPin){

	uint8_t curPort;
	uint8_t curPin;

	switch(inOutputPin){
	case DIGITAL_OUTPUT_1:
		curPort = PORTB;
		curPin = 7;
		break;

	case DIGITAL_OUTPUT_2:
		curPort = PORTB;
		curPin = 0;
		break;

	case DIGITAL_OUTPUT_3:
		curPort = PORTB;
		curPin = 2;
		break;

	case DIGITAL_OUTPUT_4:
		curPort = PORTB;
		curPin = 3;
		break;

	case DIGITAL_OUTPUT_5:
		curPort = PORTD;
		curPin = 6;
		break;

	case DIGITAL_OUTPUT_6:
		curPort = PORTD;
		curPin = 3;
		break;

	default:
		return 0;
	}

	// Get output pin state
	if (curPort & (1 << curPin)){
		return 1;
	}

	return 0;
}


void otd_OutputEnable(){
	PORTD |= (1 << DIGOUT_EN_PIN);
	return;
}


void otd_OutputDisable(){
	PORTD &= ~(1 << DIGOUT_EN_PIN);
	return;
}


static uint8_t reverse_bit8(uint8_t x)
{
	x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1);
	x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2);
	return (x << 4) | (x >> 4);
}




/*
 * I2C Functions
 */
/* The connection of the 2-wire interface is related to the actual circuit */
#define I2C_SCL_PORT_DIR		DDRE
#define I2C_SCL_PORT_OUT		PORTE
//#define I2C_SCL_PORT_IN			PIND
#define I2C_SCL_PIN				3
//
#define I2C_SDA_PORT_DIR		DDRD
#define I2C_SDA_PORT_OUT		PORTD
#define I2C_SDA_PORT_IN			PIND
#define I2C_SDA_PIN				7
/* The bit operation of the 2-wire interface is related to the microcontroller */
#define I2C_SCL_SET				(I2C_SCL_PORT_OUT |= (1 << I2C_SCL_PIN))
#define I2C_SCL_CLR				(I2C_SCL_PORT_OUT &= ~(1 << I2C_SCL_PIN))
#define I2C_SCL_D_OUT			(I2C_SCL_PORT_DIR |= (1 << I2C_SCL_PIN))
//
#define I2C_SDA_SET				(I2C_SDA_PORT_OUT |= (1 << I2C_SDA_PIN))
#define I2C_SDA_CLR				(I2C_SDA_PORT_OUT &= ~(1 << I2C_SDA_PIN))
#define I2C_SDA_IN				(I2C_SDA_PORT_IN & (1 << I2C_SDA_PIN))
#define I2C_SDA_D_OUT			(I2C_SDA_PORT_DIR |= (1 << I2C_SDA_PIN))\
// ::: NOTE :::	Setting SDA while it is input, sets the internal pull up resistor
#define I2C_SDA_D_IN			{(I2C_SDA_PORT_DIR &= ~(1 << I2C_SDA_PIN));(I2C_SDA_SET);}
//
#define I2C_DELAY 5   // uS


static void I2C_Init(){

	I2C_SCL_SET;		// Set SCL high
	I2C_SCL_D_OUT;		// Set SCL as output
	I2C_SDA_SET;		// Set SDA high
	I2C_SDA_D_OUT;		// Set SDA as output

	return;
}

static void I2C_Start()
{
   I2C_SDA_D_OUT;
   I2C_SCL_SET;      			//SCL  high;
   _delay_us(I2C_DELAY);
   I2C_SDA_SET;      			//SDA  high;
   _delay_us(I2C_DELAY);
   I2C_SDA_CLR;      			//SDA  low;
   _delay_us(I2C_DELAY);
   I2C_SCL_CLR;      			//SCL  low;
   _delay_us(I2C_DELAY);
   return;
}

static void I2C_Stop()
{
	I2C_SDA_D_OUT;
	I2C_SCL_CLR;
	_delay_us(I2C_DELAY);
   I2C_SDA_CLR;
   _delay_us(I2C_DELAY);
   I2C_SCL_SET;
   _delay_us(I2C_DELAY);
   I2C_SDA_SET;
   _delay_us(I2C_DELAY);

   return;
}

static uint8_t I2C_Write_Byte(unsigned char inByte)
{
	// We reset hasAck if the slave does not send ACK
	unsigned char hasACK = 1;

	unsigned char i;
	I2C_SDA_D_OUT;
	for(i = 0; i < 8; i++) {
		if(inByte & 0x80){
			I2C_SDA_SET;
		}else{
			I2C_SDA_CLR;
		}
		_delay_us(I2C_DELAY);
		I2C_SCL_SET;
		_delay_us(I2C_DELAY);
		I2C_SCL_CLR;
		_delay_us(I2C_DELAY);
		inByte<<=1;
	}
	_delay_us(I2C_DELAY);

	I2C_SDA_D_IN;
	_delay_us(I2C_DELAY);
	I2C_SCL_SET;
	_delay_us(I2C_DELAY);

	// Read the Acknowledge bit
	if (I2C_SDA_IN != 0){
		// Could not get acknowledge
		hasACK = 0;
	}

	I2C_SCL_CLR;
	_delay_us(I2C_DELAY);

	return hasACK;
}

static void I2C_Read_Byte(uint8_t *outByte){

	// Reset output byte
	*outByte = 0;
	unsigned char i, bytedata=0;

	//Set the data to input mode
	I2C_SDA_D_IN;
	_delay_us(I2C_DELAY);

	//The data line is pulled high
	I2C_SDA_SET;
	_delay_us(I2C_DELAY);
	I2C_SCL_CLR;
	_delay_us(I2C_DELAY);

	//Read 8-bit data
	for(i = 0; i <8; i++){
		I2C_SCL_SET;
		_delay_us(I2C_DELAY);
		_delay_us(I2C_DELAY);
		if (I2C_SDA_IN){
			bytedata |= (1 << i);
		}else{
			bytedata &= ~(1 << i);
		}

		_delay_us(I2C_DELAY);
		I2C_SCL_CLR;
		_delay_us(I2C_DELAY);
	}

	//Set the data line back to output mode
	I2C_SDA_D_OUT;

	I2C_SDA_SET;
	_delay_us(I2C_DELAY);
	I2C_SCL_SET;
	_delay_us(I2C_DELAY);
	I2C_SCL_CLR;
	_delay_us(I2C_DELAY);

	*outByte = bytedata;
	return;
}

/*
 * DIGITAL OUTPUT Functions
 */
static void init_digital_output(){

	// Set output enable to zero and make the pin output
	PORTD &= ~(1 << DIGOUT_EN_PIN);
	DDRD |= (1 << DIGOUT_EN_PIN);
	// Reset output
	PORTB &= ~(1 << 7);		// OUT 0
	PORTB &= ~(1 << 0);		// OUT 1
	PORTB &= ~(1 << 2);		// OUT 2
	PORTB &= ~(1 << 3);		// OUT 3
	PORTD &= ~(1 << 6);		// OUT 4
	PORTD &= ~(1 << 3);		// OUT 5

	// Set as output
	DDRB |= (1 << 7);
	DDRB |= (1 << 0);
	DDRB |= (1 << 2);
	DDRB |= (1 << 3);
	DDRD |= (1 << 6);
	DDRD |= (1 << 3);

	return;
}



#ifdef __cplusplus
}
#endif