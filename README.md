# MSP430 Double for Testing

This project aims to demonstrate a viable alternative for software testing for ESP32 microcontrollers using the MSP430FR5969 as an external device simulator.

The test techniques used in the project were TDD and HILS and the architecture consists of using three components: the computer, the device under test (DUT) and the device that simulates the external device (Double). The DUT device used was the ESP32 and the DUT device was the MSP430FR5969. 
For more information about the architecture and codes used in ESP32, visit the [micropython_test_lib](https://github.com/saramonteiro/micropython_test_lib) project developed by Sarah Monteiro.

Three applications have been developed for the MSP430FR5969: a GPS device, an RTC device, and a device that stores data sent on SPI bus. By assuming the role of Double, the microcontroller operates in slave mode.

Code development for the MSP430FR5969 was performed using Code Composer Studio. When running the test script, the software must already be integrated into the MSP430FR5969. 
