# Communication-and-Programming-EEPROM-for-TLI4971

Program for STM32 microcontroller that communicates with a current sensor TLI4971 via its custom communication protocol and to reset its EEPROM to a desired content (with the use of ~20V power supply). TLI4971 has a custom single-wire communication protocol. 

The Infineons own programmer costs $300, which puts it beyond many people's reach. My solution requires only a microcontroller and a power supply.
There is an evolution-board for the TLI4971 current sensor for around $20 but it doesn't reset its EEPROM, and the changes to memory revert back to default values after a restart. <br/>
<img width="320" height="640" alt="image" src="https://github.com/user-attachments/assets/bfabd59c-5d06-4113-9eed-ac0f89f13ce8" /><br/>

This project required:
1. Creating electronic circuit for communication and controlling 20.5V voltage supply.
2. Writing a program for STM32 that will simultaneously send and receive data.
# Electronic circuit
<img width="450" height="725" alt="image" src="https://github.com/user-attachments/assets/7c668b5b-b76f-4629-9de2-6aba0d61120e" /><br/>
Electronic cicuit with NPN-PNP switch to control 20 V and a voltage regulator to control when the current sensor powers up.
# STM32 program
Program is based on a 100μs timer interrupt. With switches, boolean operations, boolean algebra etc.
```c

```
# Using program

<img width="320" height="541" alt="image" src="https://github.com/user-attachments/assets/f4abff87-b302-458f-8756-296a5e6fc584" /><br/>

<img width="320" height="624" alt="image" src="https://github.com/user-attachments/assets/d9a98eb2-094e-41bf-b9da-3cfc1df7ae4d" /><br/>
