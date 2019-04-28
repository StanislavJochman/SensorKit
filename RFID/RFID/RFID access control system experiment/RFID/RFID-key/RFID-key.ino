/*
 * File name：RFID.pde
 * Modifier：kuman
  *Function description：Mifare1  Find cards→Conflict prevention→Pick a card→Read and  write  Interface
*/
 // the sensor communicates using SPI, so include the library:

#include <SPI.h>
#include <string.h>
#define	uchar	unsigned char
#define	uint	unsigned int

//Maximum array length
#define MAX_LEN 16

/////////////////////////////////////////////////////////////////////
//set the pin
/////////////////////////////////////////////////////////////////////
const int chipSelectPin = 10;//If the control panel is UNO,328,168
//const int chipSelectPin = 53; //If the control panel is mega 2560,1280
const int NRSTPD = 5;
const int LED_PIN=2;
/////////////////Key parameters////////////////////////////////////
unsigned char Key_buf[5];
//unsigned char Key_Test[5]={0x93,0xd3,0x45,0x8c,0x89};//Key password (that is, the card number)
unsigned char Key_Test[5]={0x53,0x08,0x3F,0xAA,0xCE};
unsigned char Key_Flag=0;
///////////////////////////////////////////////////////////////////
//MF522 Command word
#define PCD_IDLE              0x00               //NO action;Cancel current command
#define PCD_AUTHENT           0x0E               //Authentication key
#define PCD_RECEIVE           0x08               //receive data
#define PCD_TRANSMIT          0x04               //send data
#define PCD_TRANSCEIVE        0x0C               //Send and receive data
#define PCD_RESETPHASE        0x0F               //reset
#define PCD_CALCCRC           0x03               //CRC calculation

//Mifare_One Card command word
#define PICC_REQIDL           0x26               //No sleep state was found in the antenna area
#define PICC_REQALL           0x52               //Find all the cards in the antenna area
#define PICC_ANTICOLL         0x93               //anti-collision
#define PICC_SElECTTAG        0x93               //Pick a card
#define PICC_AUTHENT1A        0x60               //Verify A key
#define PICC_AUTHENT1B        0x61               //Verify B key
#define PICC_READ             0x30               //Read block
#define PICC_WRITE            0xA0               //Write block
#define PICC_DECREMENT        0xC0               
#define PICC_INCREMENT        0xC1               
#define PICC_RESTORE          0xC2               //Block data to buffer
#define PICC_TRANSFER         0xB0               //Save buffer data
#define PICC_HALT             0x50               //dormancy

//Error code returned when communicating with MF522
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2


//------------------MFRC522 register---------------
//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg	          0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34			  0x3F
//-----------------------------------------------

//4 byte card serial number, fifth bytes for the check byte
uchar serNum[5];
uchar  writeDate[16] ={'T', 'e', 'n', 'g', ' ', 'B', 'o', 0, 0, 0, 0, 0, 0, 0, 0,0};
//Sector A password, 16 sectors, each sector password 6Byte
 uchar sectorKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                            };
 uchar sectorNewKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                               };
void OPPEN_Door(void)
{
  if(Key_Flag==1)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(5000);
   digitalWrite(LED_PIN, LOW);  
  }else
  {
    digitalWrite(LED_PIN, LOW); 
  }
}

void setup() {                
   Serial.begin(9600);                       // RFID reader SOUT pin connected to Serial RX pin at 2400bps 
 // start the SPI library:
  SPI.begin();
   pinMode(LED_PIN,OUTPUT);
   digitalWrite(LED_PIN, LOW); 
   
  pinMode(chipSelectPin,OUTPUT);             // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin 
    digitalWrite(chipSelectPin, LOW);          // Activate the RFID reader
  pinMode(NRSTPD,OUTPUT);               // Set digital pin 10 , Not Reset and Power-down
    digitalWrite(NRSTPD, HIGH);

  MFRC522_Init();  
}

void loop()
{
  	uchar i,tmp;
	uchar status;
        uchar str[MAX_LEN];
        uchar RC_size;
        uchar blockAddr;	//Select the block address for the operation0～63


		//Find card, return card type	
		status = MFRC522_Request(PICC_REQIDL, str);	
		if (status == MI_OK)
		{
		}

		//Anti collision, return card serial number 4 bytes
		status = MFRC522_Anticoll(str);
		memcpy(serNum, str, 5);
		if (status == MI_OK)
		{
                        for(i=0;i<4;i++)Key_buf[i]=serNum[i];
                        for(i=0;i<4;i++)
                        {
                           if(Key_buf[i]!=Key_Test[i])
                            {
                              Key_Flag = 0;
                              Serial.println("Passworld Error");
                              break;
                            }else
                          {
                            Key_Flag = 1;
                              Serial.println("Passworld    OK");
                          }  
                            
                        }          
                        Serial.println("The card's number is  : ");
			Serial.print(serNum[0],HEX);
                        Serial.println(" ");
			Serial.print(serNum[1],HEX);
                        Serial.println(" ");
			Serial.print(serNum[2],HEX);
                        Serial.println(" ");  
			Serial.print(serNum[3],HEX);
			Serial.println(" ");
                        Serial.print(serNum[4],HEX);
                        Serial.println(" ");
                        OPPEN_Door();       
		}

		//Card selection, return card capacity
		RC_size = MFRC522_SelectTag(serNum);
		if (RC_size != 0)
		{}
                
		//Write data card
		blockAddr = 7;		//data block7		
		status = MFRC522_Auth(PICC_AUTHENT1A, blockAddr, sectorKeyA[blockAddr/4], serNum);	//authentication
		if (status == MI_OK)
		{
			//Write data
			status = MFRC522_Write(blockAddr, sectorNewKeyA[blockAddr/4]);
                        Serial.print("set the new card password, and can modify the data of the Sector: ");
                        Serial.print(blockAddr/4,DEC);
   
                        //Write data
                        blockAddr = blockAddr - 3 ; 
                        status = MFRC522_Write(blockAddr, writeDate);
                        if(status == MI_OK)
                        {
                           Serial.println("OK!");
                        }
		}

		//card reading
		blockAddr = 7;		//Data block 7		
		status = MFRC522_Auth(PICC_AUTHENT1A, blockAddr, sectorNewKeyA[blockAddr/4], serNum);	//authentication
		if (status == MI_OK)
		{
			//Read data
                        blockAddr = blockAddr - 3 ; 
                        status = MFRC522_Read(blockAddr, str);
			if (status == MI_OK)
			{
                                Serial.println("Read from the card ,the data is : ");
				for (i=0; i<16; i++)
				{
              			      Serial.print(str[i]);
				}
                                Serial.println(" \n");
			}
		}
                //Serial.println(" ");
		MFRC522_Halt();			//Command card to enter hibernation           
          
}

/*
 * Function name：Write_MFRC5200
 * Function Description: write a byte of data to a MFRC522 of a certain register.
 * Input parameters: addr-- register address; val-- value to be written
 * Return value: None
 */
  void Write_MFRC522(uchar addr, uchar val)
{
	digitalWrite(chipSelectPin, LOW);

	//Address format：0XXXXXX0
	SPI.transfer((addr<<1)&0x7E);	
	SPI.transfer(val);
	
	digitalWrite(chipSelectPin, HIGH);
}


/*
 * Function name: Read_MFRC522
 * Function Description: read a byte of data from a MFRC522 register
 * Input parameters: addr-- register address
 * Returns a byte data read to
 */
 
uchar Read_MFRC522(uchar addr)
{
	uchar val;

	digitalWrite(chipSelectPin, LOW);
       	//Address format：1XXXXXX0
	SPI.transfer(((addr<<1)&0x7E) | 0x80);	
	val =SPI.transfer(0x00);
	
	digitalWrite(chipSelectPin, HIGH);
	
	return val;	
}

/*
 * Function name: SetBitMask
 * Function Description: set RC522 register bit
 * input parameters: reg-- register address; mask-- set value
 * return value: None
 */
void SetBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);  // set bit mask
}


/*
* function name: ClearBitMask
* function description: RC522 register bits
* input parameters: reg-- register address; mask-- value
* return value: None 
*/
void ClearBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));  // clear bit mask
} 


/*
 * function name: AntennaOn
*Function Description:  open antenna, every time you start or shut down at least 1ms should launch the interval between the natural barrier
* input parameters: None
* return value: None
 */
void AntennaOn(void)
{
	uchar temp;

	temp = Read_MFRC522(TxControlReg);
	if (!(temp & 0x03))
	{
		SetBitMask(TxControlReg, 0x03);
	}
}


/*
 * function name: AntennaOff
 *Function Description:  off the antenna, every time you start or shut down at least 1ms should launch the interval between the natural barrier
 * input parameters: None
 * return value: None
 */
void AntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}


/*
* function name: ResetMFRC522
* function description: reset RC522
* input parameters: None
* return value: None
 */
void MFRC522_Reset(void)
{
    Write_MFRC522(CommandReg, PCD_RESETPHASE);
}


/*
* function name: InitMFRC522
* function description: initialize RC522
* input parameters: None
* return value: None
 */
void MFRC522_Init(void)
{
	digitalWrite(NRSTPD,HIGH);

	MFRC522_Reset();
	 	
	//Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    Write_MFRC522(TModeReg, 0x8D);		//Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    Write_MFRC522(TPrescalerReg, 0x3E);	//TModeReg[3..0] + TPrescalerReg
    Write_MFRC522(TReloadRegL, 30);           
    Write_MFRC522(TReloadRegH, 0);
	
	Write_MFRC522(TxAutoReg, 0x40);		//100%ASK
	Write_MFRC522(ModeReg, 0x3D);		//CRCinitial value 0x6363	???

	//ClearBitMask(Status2Reg, 0x08);		//MFCrypto1On=0
	//Write_MFRC522(RxSelReg, 0x86);		//RxWait = RxSelReg[5..0]
	//Write_MFRC522(RFCfgReg, 0x7F);   		//RxGain = 48dB

	AntennaOn();		//Open antenna
}


/*
 * function name: MFRC522_Request
 * Functional Description: find cards, read card type
 * input parameters: reqMode-- search method,
 * TagType-- return card type
 *			 	0x4400 = Mifare_UltraLight
 *				0x0400 = Mifare_One(S50)
 *				0x0200 = Mifare_One(S70)
 *				0x0800 = Mifare_Pro(X)
 *				0x4403 = Mifare_DESFire
 * return value: return to MI_OK
 */
uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
	uchar status;  
	uint backBits;			//Received data bits

	Write_MFRC522(BitFramingReg, 0x07);		//TxLastBists = BitFramingReg[2..0]	???
	
	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10))
	{    
		status = MI_ERR;
	}
   
	return status;
}


/*
 * function name: MFRC522_ToCard
* function description: RC522 and ISO14443 card communication
* input parameters: command--MF522 command word,
 *			SendData-- sent to the card through the RC522 data,
 *			 Data length sent by sendLen--	 
 *			 BackData-- received card return data,
 *			BackLen-- returns the bit length of the data
 * Return value: return to MI_OK
 */
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
    uchar status = MI_ERR;
    uchar irqEn = 0x00;
    uchar waitIRq = 0x00;
    uchar lastBits;
    uchar n;
    uint i;

    switch (command)
    {
        case PCD_AUTHENT:		//Certification card
		{
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:	//Send data in FIFO
		{
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
    }
   
Write_MFRC522(CommIEnReg, irqEn|0x80);	//Allow interrupt request
    ClearBitMask(CommIrqReg, 0x80);			//Clear all interrupt request bits
    SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO initialization
    
	Write_MFRC522(CommandReg, PCD_IDLE);	//NO action;Cancel current command	???

	//向FIFOWrite data in
    for (i=0; i<sendLen; i++)
    {   
		Write_MFRC522(FIFODataReg, sendData[i]);    
	}

	//Execute command
	Write_MFRC522(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {    
		SetBitMask(BitFramingReg, 0x80);		//StartSend=1,transmission of data starts  
	}   
    
	//Waiting for receiving data to complete
	i = 2000;	//iAccording to the clock frequency adjustment, the operation of the M1 card maximum waiting time 25ms	???
    do 
    {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read_MFRC522(CommIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    ClearBitMask(BitFramingReg, 0x80);			//StartSend=0
	
    if (i != 0)
    {    
        if(!(Read_MFRC522(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   
				status = MI_NOTAGERR;			//??   
			}

            if (command == PCD_TRANSCEIVE)
            {
               	n = Read_MFRC522(FIFOLevelReg);
              	lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits)
                {   
					*backLen = (n-1)*8 + lastBits;   
				}
                else
                {   
					*backLen = n*8;   
				}

                if (n == 0)
                {   
					n = 1;    
				}
                if (n > MAX_LEN)
                {   
					n = MAX_LEN;   
				}
				
				//Read data received in FIFO
                for (i=0; i<n; i++)
                {   
					backData[i] = Read_MFRC522(FIFODataReg);    
				}
            }
        }
        else
        {   
			status = MI_ERR;  
		}
        
    }
	
    //SetBitMask(ControlReg,0x80);           //timer stops
    //Write_MFRC522(CommandReg, PCD_IDLE); 

    return status;
}


/*
 * function name: MFRC522_Anticoll
* Feature Description: anti collision detection, read the card number of the selected card
* input parameters: serNum-- returns the 4 byte card serial number, and the fifth byte is the check byte
* return value: return to MI_OK
 */
uchar MFRC522_Anticoll(uchar *serNum)
{
    uchar status;
    uchar i;
	uchar serNumCheck=0;
    uint unLen;
    

    //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
    //ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
 
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
	{
		//Check card serial number
		for (i=0; i<4; i++)
		{   
		 	serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i])
		{   
			status = MI_ERR;    
		}
    }

    //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

    return status;
} 


/*
 * function name: CalulateCRC
* function description: use MF522 to calculate CRC
* input parameters: pIndata-- to read CRC data, len-- data length, CRC calculation of the pOutData-- results
* return value: None
 */
void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
    uchar i, n;

    ClearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
    SetBitMask(FIFOLevelReg, 0x80);			//Clear FIFO pointer
    //Write_MFRC522(CommandReg, PCD_IDLE);

	//Write data to FIFO	
    for (i=0; i<len; i++)
    {   
		Write_MFRC522(FIFODataReg, *(pIndata+i));   
	}
    Write_MFRC522(CommandReg, PCD_CALCCRC);

	//Waiting for the CRC calculation to complete
    i = 0xFF;
    do 
    {
        n = Read_MFRC522(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation results
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);
}


/*
 * function name: MFRC522_SelectTag
 * Feature Description: select card, memory card memory capacity
 * input parameters: serNum-- incoming card serial number
 * return value: return card capacity
 */
uchar MFRC522_SelectTag(uchar *serNum)
{
    uchar i;
	uchar status;
	uchar size;
    uint recvBits;
    uchar buffer[9]; 

	//ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0

    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i=0; i<5; i++)
    {
    	buffer[i+2] = *(serNum+i);
    }
	CalulateCRC(buffer, 7, &buffer[7]);		//??
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    
    if ((status == MI_OK) && (recvBits == 0x18))
    {   
		size = buffer[0]; 
	}
    else
    {   
		size = 0;    
	}

    return size;
}


/*
 * function name: MFRC522_Auth
* function description: verify card password
* input parameters: authMode-- password authentication mode
            0x60 = verify A key
             0x61 = verify B key
             BlockAddr-- block address
              Sectorkey-- sector password
               SerNum-- card serial number, 4 byte
* return value: return to MI_OK
 */
uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
    uchar status;
    uint recvBits;
    uchar i;
	uchar buff[12]; 

	//Verification instruction + block address + sector password + card serial number
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i=0; i<6; i++)
    {    
		buff[i+2] = *(Sectorkey+i);   
	}
    for (i=0; i<4; i++)
    {    
		buff[i+8] = *(serNum+i);   
	}
    status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

    if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
    {   
		status = MI_ERR;   
	}
    
    return status;
}


/*
* function name: MFRC522_Read
*Function Description: read block data
* input parameters: blockAddr-- block address; block data read by recvData--
* return value: return to MI_OK 
*/
uchar MFRC522_Read(uchar blockAddr, uchar *recvData)
{
    uchar status;
    uint unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    CalulateCRC(recvData,2, &recvData[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

    if ((status != MI_OK) || (unLen != 0x90))
    {
        status = MI_ERR;
    }
    
    return status;
}


/*
 * function name: MFRC522_Write
 *Function Description: write block data
 * input parameters: blockAddr-- block address; writeData-- to block write 16 byte data
 * return value: return to MI_OK
 */
uchar MFRC522_Write(uchar blockAddr, uchar *writeData)
{
    uchar status;
    uint recvBits;
    uchar i;
	uchar buff[18]; 
    
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalulateCRC(buff, 2, &buff[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {   
		status = MI_ERR;   
	}
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)		//Write 16Byte data to FIFO
        {    
        	buff[i] = *(writeData+i);   
        }
        CalulateCRC(buff, 16, &buff[16]);
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
        
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        {   
			status = MI_ERR;   
		}
    }
    
    return status;
}


/*
 * function name: MFRC522_Halt
 * function description: Command card to enter sleep state
 * input parameters: None
 * return value: None
 */
void MFRC522_Halt(void)
{
	uchar status;
    uint unLen;
    uchar buff[4]; 

    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);
 
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}
