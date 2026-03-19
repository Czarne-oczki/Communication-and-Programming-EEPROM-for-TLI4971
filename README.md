# Communication (and programming of EEPROM) with TLI4971
## Overview - Justification for the project
There is a certain current sensor on the market - TLI4971, that is both precise and cheap, but the evaluation board sold with the current sensor only changes RAM of the current sensor. Therefore you cannot permanently change settings of the current sensor (range, operation mode, "over current detection threshhold"), and you are stuck with the default settings unless you go through the trouble of changing the settings each time you power up the current sensor. There is a programmer for the current sensor but it's expensive. 
<br/>I've set the goal for this project to :
1. communicate with the current sensor (via its own special protocol),
2. permanently change its EEPROM contents with the use of 20V power supply.

<!---Program for STM32 microcontroller that communicates with a current sensor TLI4971 via its custom communication protocol and to reset its EEPROM to a desired content (with the use of ~20V power supply). TLI4971 has a custom single-wire communication protocol. 

The Infineons own programmer costs $300, which puts it beyond many people's reach. My solution requires only a microcontroller and a power supply.
There is an evolution-board for the TLI4971 current sensor for around $20 but it doesn't reset its EEPROM, and the changes to memory revert back to default values after a restart. <br/> --->
<img width="320" height="640" alt="image" src="https://github.com/user-attachments/assets/bfabd59c-5d06-4113-9eed-ac0f89f13ce8" /><br/>

This project required:
1. Creating electronic circuit for communication and controlling 20.5V voltage supply.
2. Writing a program for STM32 that will simultaneously send and receive data.
# Electronic circuit 
<img width="450" height="725" alt="image" src="https://github.com/user-attachments/assets/7c668b5b-b76f-4629-9de2-6aba0d61120e" /><br/>
Electronic cicuit with NPN-PNP switch to control 20 V and a voltage regulator to control when the current sensor powers up.
# STM32 program
Program is based on a 100μs timer interrupt. With switches, boolean operations, boolean algebra etc. Full code can be found at "Code/Src/main.c". A small part of code is shown below:
```c
if(htim->Instance == TIM3){ // The main timer interrupt. Runs every 100us.


		switch(checkmark){
			case 0:
				break;
			case 1: //Powering up the sensor
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, TIFEN_Matrix[bit_counter]); //setting 0 to Aout.
				HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, SET); //Turning on voltage stabilizer TC1014
				bit_counter ++ ;
				if(bit_counter >= 5){
					bit_counter = 0;
					checkmark = 2;
				}
				break;
			case 2: //Writing enter interface sequence
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0); //Triggers step-down signalizes start of a frame. (Useful only for reading with osciloscope)
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, FrameMatrix[(picker & WordABCD) && 1][bit_counter]); //FrameMatrix[x][y] has frame struction. The [x] is logic 0/1. The [y] is the sequence of the frame.
				bit_counter ++;
				if(bit_counter >= 6){
					HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,1);
					bit_counter = 0;
					picker = picker << 1; //the picker goes one bit up, 1 -> 2 -> 4 -> 8 ->... -> 32768. After that the whole word has been sent, and picker resets to picker = 1.
					if(!picker){ // When picker (uint16_t) reaches value of 65536 it will set itself to '0' since the highest number for uint16_t is 65,535. Therefore the condition !picker.
						picker = 1;
						checkmark = 3;
//						checkmark = 7; // Uncomment this line, and comment out checkmark =3; line to write custom message and read the answer from TLI4971
					}
				}
```
# Using the program
The programs default operation is to:
1. read EEPROM contents,
2. store the data in EPROMreg[1] matrix,
3. the program now overwrites EEPROM contents with desired values and saves it with the use of 20V,
4. after which the program asks the current sensor to read back the new EEPROM contents and store it in EPROMreg[2]. <br/>
<br/>
Both EEPROM contents can be read in "live expressions" section of debugger: <br/>
<img width="400" height="628" alt="Zrzut ekranu 2026-02-02 101313" src="https://github.com/user-attachments/assets/f6841fac-e152-44b6-a00f-000292ac45b9" />

The image above shows changed and saved values on adress 0x40 (0xc00c -> 0xc038) and 0x42 (0x4 -> 0x9). The changes on these adrresses changed the range of current sensor and the operation mode based on table below: <br/>
<img width="500" height="747" alt="image" src="https://github.com/user-attachments/assets/24bb4973-9614-4c1c-b5af-8531ecb0fe50" />


<!---<img width="320" height="541" alt="image" src="https://github.com/user-attachments/assets/f4abff87-b302-458f-8756-296a5e6fc584" /><br/>

<img width="320" height="624" alt="image" src="https://github.com/user-attachments/assets/d9a98eb2-094e-41bf-b9da-3cfc1df7ae4d" /><br/> --->

