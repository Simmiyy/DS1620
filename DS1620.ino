
/* PIN setup */
/*    SPI   -     Arduino     */
/*                            */
/*  RST     <-     digital 7  */
/*  CLK     <-     digital 6  */
/*  DQ      <->    digital 5  */
/*                            */

#define RST 7
#define CLK 6
#define SDO 5

/* DS1620 commands */
#define SETTH 0x01
#define SETTL 0x02
#define SETCF 0x0C
#define GETTH 0xA1
#define GETTL 0xA2
#define GETCF 0xAC
#define GETTP 0xAA

/* functions prototype */
byte  nibbleToByte( byte nibble );
void  createBuffer( byte *buffer, byte *nibbles, int nlen );
int   getInput( byte *buff );
float byteToFloat( byte *buffer );
void  clearBuffer( byte *buf, int len );
void  writeByte( byte *input, int len );
void  readByte( byte cmd, byte *output, int len );

/* global variables */
static byte  buffer[5];
static byte  support[5];
static int   incomingByte = 0;
static float temperature;

/*********************************
 **********  FUNCTIONS  **********
 *********************************/

void setup(){
 
  Serial.begin( 9600 );
  
  /* reset and clock pins are output only */
  pinMode( RST, OUTPUT );
  pinMode( CLK, OUTPUT );
  
  /* set the clock and reset pins default state */
  digitalWrite( RST, LOW );
  digitalWrite( CLK, LOW );
  
  /* wait 100 ms */
  delay( 100 );

}

void loop(){
  
  if( Serial.available() > 0 ){
    
    // read the incoming byte:
    incomingByte = Serial.read();
    
    switch( incomingByte ){
     
      case 'h':
        /* print the help menu */
        Serial.println( "******* DS1621 Help menu *******" );
        Serial.println( "" );
        Serial.println( "h  print this help menu" );
        Serial.println( "t  read the temperature regiser" );
        Serial.println( "g  read the high temperature register" );
        Serial.println( "l  read the low temperature register" );
        Serial.println( "c  read configuration register" );
        Serial.println( "s  write configuration register" );
        Serial.println( "k  write the high temperature register" );
        Serial.println( "p  write the low temperature register" );
        break;
      
      case 't':
        clearBuffer( buffer, 5 );
        readByte( GETTP, buffer, 9 );
        Serial.print( "Temperature: " );
        Serial.print( byteToFloat( buffer ) );
        Serial.println( " °C" );
        break;
        
      case 'g':
        clearBuffer( buffer, 5 );
        readByte( GETTH, buffer, 9 );
        Serial.print( "Temperature high: " );
        Serial.print( byteToFloat( buffer ) );
        Serial.println( "" );
        break;
        
      case 'l':
        clearBuffer( buffer, 5 );
        readByte( GETTL, buffer, 9 );
        Serial.print( "Temperature low: " );
        Serial.print( byteToFloat( buffer ) );
        Serial.println( "" );
        break;

      case 'c':
        clearBuffer( buffer, 5 );
        readByte( GETCF, buffer, 8 );
        Serial.print( "The configuration byte is: " );
        Serial.print( buffer[0], HEX );
        Serial.println( "" );
        break;

      case 's':
        /* write the configuration register */
        Serial.println( "Enter the new configuration register value in HEX format" );
        
        clearBuffer( buffer, 5 );
        buffer[0] = SETCF;
        createBuffer( &buffer[1], support, getInput( support ) );
        Serial.print( "new register value is: " );
        Serial.println( buffer[1], HEX );
        
        writeByte( buffer, 16 );
        break;
        
      case 'k':
        /* write the temperature high register */
        Serial.println( "Enter the new temperature high register value in HEX format" );
        
        clearBuffer( buffer, 5 );
        buffer[0] = SETTH;
        createBuffer( &buffer[1], support, getInput( support ) );
        Serial.print( "new register value is: " );
        Serial.print( buffer[2], HEX );
        Serial.println( buffer[1], HEX );
        
        writeByte( buffer, 17 );
        break;
        
      case 'p':
        /* write the temperature low register */
        Serial.println( "Enter the new temperature low register value in HEX format" );
        
        clearBuffer( buffer, 5 );
        buffer[0] = SETTL;
        createBuffer( &buffer[1], support, getInput( support ) );
        Serial.print( "new register value is: " );
        Serial.print( buffer[2], HEX );
        Serial.println( buffer[1], HEX );
        
        writeByte( buffer, 17 );
        break;
    }
  }
}



/*
 * convert an ASCII character to the equivalent hex value
 */

byte nibbleToByte( byte nibble ){
  
  /* digit */
  if( nibble >= '0' && nibble <= '9' ){
    
    nibble -= '0';
  
  /* lowe case character */
  } else if( nibble >= 'a' && nibble <= 'f' ) {
    
    nibble -= 87;
    
  /* upper case character */
  } else if( nibble >= 'A' && nibble <= 'F' ) {
    
    nibble -= 55;
    
  } else {
    
    nibble = 0;
  }
  
  return nibble;
}



void createBuffer( byte *buffer, byte *nibbles, int nlen ){
 
 int i, j, flag = 0;
 
 /* remove not valid character from the tail */
 while( (nibbles[nlen-1] < '0'                          ) ||
        (nibbles[nlen-1] > '9' && nibbles[nlen-1] < 'A' ) ||
        (nibbles[nlen-1] > 'F' && nibbles[nlen-1] < 'a' ) ||
        (nibbles[nlen-1] > 'f'                          )
       ){
   nlen--;
 }

 /* from LSB to MSB */
 for( i = nlen - 1, j = 0; i >= 0; i-- ){
  
   if( flag == 0 ){
     
     buffer[j] = nibbleToByte( nibbles[i] );
     flag = 1;
     
   } else {
     
     buffer[j] |= nibbleToByte( nibbles[i] ) << 4;
     j++;
     flag = 0;
   }
 } 
}



void clearInput(){
  
  while( Serial.read() != -1 ){
    delay( 1 );
  }
}


/* 
 * reads from serial console a byte
 */

int getInput( byte *buff ){
  
  int i = 0;
  
  /* clear input queue */
  clearInput();
  
  /* wait the user input */
  while( Serial.available() <= 0 ){
    
    delay( 10 );
  }
  
  /* read incoming data up to 5 bytes */
  do{
    
    buff[i++] = (byte)Serial.read();
    delay( 5 );
    
  } while( Serial.available() && i < 5 );
  
  return i;
}


/* 
 * translate the 9bit precision value into float
 */
 
float byteToFloat( byte *buffer ){
 
  float retval = 0.0;
 
  if( buffer[0] & 0x01 ){
    retval += 0.5;
  }
  
  /* remove the 0.5 bit */
  buffer[0] >>= 1;
  
  /* if negative temperature */
  if( buffer[1] & 0x01 ){
    retval += (float)(int)(0xFF80 | (unsigned int)buffer[0]);
  } else {
    retval += (float)buffer[0];
  }
          
  return retval;
}


/*
 * Empty the buffer provided
 * buf    the buffer to empty
 * len    how many byte should be empty in the buffer
 */

void clearBuffer( byte *buf, int len ){
  
  int i;
  
  for( i = 0; i < len; i++ ){
    
    buf[i] = 0x00;
  }
}



/*
 * Write byte to the SPI interface bit by bit
 * input  input buffer where the byte to sent are read
 * len    how many bit should be sent
 */
 
void writeByte( byte *input, int len ){

  int i;
  
  /* set the SDO pin as output */  
  pinMode( SDO, OUTPUT );
  
  /* start a new transfer */
  digitalWrite( CLK, HIGH );
  digitalWrite( RST, HIGH );

  for( i = 0; i < len ; i++ ){
   
    digitalWrite( CLK, LOW );
    delayMicroseconds( 10 );
    
    /* if the bit is HIGH, LSB to MSB, bit 0 to 7 */
    if( (input[i/8] >> (i % 8)) & 0x01 ){
      
      /* send 1 */ 
      digitalWrite( SDO, HIGH );
      
    } else {
      
      /* send 0 */
      digitalWrite( SDO, LOW );
    }
    
    digitalWrite( CLK, HIGH );
    delayMicroseconds( 10 );
  }

  /* end the transfer */
  digitalWrite( RST, LOW );
  digitalWrite( CLK, LOW ); 
}



/*
 * Read bytes from SPI interface after sending a command byte bit by bit
 * cmd     the command byte to send before reading
 * output  output buffer where the read bytes are stored
 * len     how many bit should be read
 */

void readByte( byte cmd, byte *output, int len ){
  
  int i;
  
  /* set the SDO pin as output */  
  pinMode( SDO, OUTPUT );
  
  /* start a new transfer */
  digitalWrite( CLK, HIGH );
  digitalWrite( RST, HIGH );
  
  for( i = 0; i < 8 ; i++ ){
   
    digitalWrite( CLK, LOW );
    delayMicroseconds( 10 );
   
    /* if the bit is HIGH, MSB to LSB */
    if( (cmd >> i) & 0x01 ){
      
      /* send 1 */ 
      digitalWrite( SDO, HIGH );
      
    } else {
      
      /* send 0 */
      digitalWrite( SDO, LOW );
    }
    
    digitalWrite( CLK, HIGH );
    delayMicroseconds( 10 );
  }
  
  /* set the SDO pin as output */  
  pinMode( SDO, INPUT );
  /* disable pull-up resistor */
  digitalWrite( SDO, LOW );
  
  /* read incoming data */
  for( i = 0; i < len; i++ ){
  
    digitalWrite( CLK, LOW );
    delayMicroseconds( 10 );
    
    digitalWrite( CLK, HIGH );
    delayMicroseconds( 10 );
    
    if( digitalRead( SDO ) == HIGH ){
      
      output[i/8] |= 0x01 << (i % 8);
    }
  }
  
  /* end the transfer */
  digitalWrite( RST, LOW );
  digitalWrite( CLK, LOW );
}


