# Communication-and-Programming-EEPROM-for-TLI4971

I have programmed an STM32 microcontroller to communicate with a current sensor TLI4971 via its custom communication protocol and to reset its EEPROM to a desired content. TLI4971 has a custom single-wire communication protocol so the project demanded high understanding of timer interrupts, Infineons custom protocol, transistor logic and open-drain communication behaviour.

The Infineons own programmer costs $300, which puts it beyond many people's reach. My solution requires only a microcontroller and a power supply.
There is an evolution-board for the current sensor for around $20 but it doesn't reset its EEPROM, and the changes to memory revert back to default values after a restart.

This project required:
-Creating robust single-wire communication with TLI4971 based on its documentation,
-Precise control of ~20V via NPN-PNP logic using 3.3V STM32 PIN,
-Soldering and testing self-made circuits onto soldering boards.


<img width="640" height="640" alt="image" src="https://github.com/user-attachments/assets/bfabd59c-5d06-4113-9eed-ac0f89f13ce8" />
<img width="1280" height="725" alt="image" src="https://github.com/user-attachments/assets/7c668b5b-b76f-4629-9de2-6aba0d61120e" />

<img width="904" height="541" alt="image" src="https://github.com/user-attachments/assets/f4abff87-b302-458f-8756-296a5e6fc584" />

<img width="1049" height="624" alt="image" src="https://github.com/user-attachments/assets/d9a98eb2-094e-41bf-b9da-3cfc1df7ae4d" />
