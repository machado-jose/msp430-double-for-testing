# Do the serial interface between PC and DUT or DOUBLE
from serial_interface_upython import SerialInterface
# Library for testing with different asserts
from should_dsl import should
# Test class from wich our class inhereints
import unittest
# Operational System Interface
import os
import sys
# Utils
import time
from time import sleep

production_code = "dut_conf_time.py"
double_code = "double_DS3231.py"
auxiliar_code = "DS3231.py"
build = "python -m mpy_cross -s -march=xtensa "
DUT_PORT = "/dev/ttyUSB0"
DOUBLE_PORT = "/dev/ttyACM1"
send = "ampy --delay 1 --port "

print('\n')
print("Connecting to DUT device...")
dut_serial = SerialInterface(DUT_PORT, 115200)
dut_serial.connect_to_serial()	

# From set-up:
# Building, connection and sending phase
# try:
# 	print("Building production code...")
# 	os.system(build+production_code)
# 	print("Building auxiliar code...")
# 	os.system(build+auxiliar_code)
# 	print("Cleaning the filesystem...")
# 	dut_serial.clean_file_sys()
# 	dut_serial.close_serial()
# 	print("Sending built production code...")
# 	os.system(send+DUT_PORT+" put "+production_code)#.replace(".py",".mpy"))
# 	print("Sending built auxiliar_code...")
# 	os.system(send+DUT_PORT+" put "+auxiliar_code)#.replace(".py",".mpy"))
# except:
# 	sys.exit('fail to upload file(s)')
# Uncomment the next line for not to run the Test
# sys.exit()		

# Testing Phase
class Test_DS3231(unittest.TestCase):
	#Creates a serial connection and import the classes
	def setUp(self):
		global dut_serial
		print('\n')
		print("Connecting to DOUBLE device...")
		self.double_serial = SerialInterface(DOUBLE_PORT, 9600)
		self.double_serial.connect_to_serial()
		dut_serial.repl("from dut_conf_time import ConfTime", 0.4)

	def test_set_date_time_static(self):
		print("\nTesting the method set_date_time() in the static mode")
		set_datetime = [2021,7,5,1,9,10,34] # datatime format: [Year, month, day, weekday, hour, minute, second]
		expected_datetime = "21,7,5,1,9,10,34,"
		print("Set value: "+str(set_datetime))
		print("Expected response: " + expected_datetime)
		# 1 - Objects Creation
		dut_serial.repl("rtc_conf = ConfTime(21,22)", 1)
		# 2 - Input Injection
		dut_serial.repl("rtc_conf.set_date_time("+str(set_datetime)+")", 0.2)
		# 3 - Results gathering
		gotten_datetime = self.double_serial.repl("t",0.2)[0]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode() 
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (str(expected_datetime))


	# Using an internal rtc from the double device 
	def test_set_date_time_dynamic(self):
		print("\nTesting the method set_date_time in the dynamic mode")
		set_datetime = [2021,7,5,1,9,10,34] # datatime format: [Year, month, day, weekday, hour, minute, second]
		my_delay = 2
		expected_datetime = "21,7,5,1,9,10,36,"
		print("Set value: "+str(set_datetime))
		print("Delay time: "+str(my_delay))
		print("Expected response: " + expected_datetime)
		# 1 - Objects Creation
		dut_serial.repl("rtc_conf = ConfTime(21,22)", 1)
		# 2 - Input Injection
		dut_serial.repl("rtc_conf.set_date_time("+str(set_datetime)+")", 0.4)
		self.double_serial.repl("a", 0.2)
		sleep(my_delay)
		# 3 - Results gathering
		gotten_datetime = self.double_serial.repl("t",0.2)[0]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode() 
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (expected_datetime)

	def test_get_date_time(self):
		print("\nTesting the method get_date_time()")
		set_datetime = [2021,7,5,1,9,10,34] # datatime format: [Year, month, day, weekday, hour, minute, second]
		expected_datetime = [2021,7,5,1,9,10,34]
		print("Set value: "+str(set_datetime))
		print("Expected response: " + str(expected_datetime))
		# 1 - Objects Creation
		dut_serial.repl("rtc_conf = ConfTime(21,22)", 1)
		# 2 - Input Injection
		self.double_serial.repl("21 7 5 1 9 10 34s", 2)
		# 3 - Results gathering
		gotten_datetime = dut_serial.repl("rtc_conf.get_date_time()",0.6)[2]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode() 
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (str(expected_datetime))

	# Test both set and get time, this case can be used along with a double device or the device itself (autotest)
	def test_set_get_date_time(self):
		print("\nTesting the methods set_date_time() and get_date_time()")
		set_datetime = [2021,7,5,1,9,10,32] # datatime format: [Year, month, day, weekday, hour, minute, second]
		my_delay = 2
		expected_datetime = [2021,7,5,1,9,10,34]
		# 1 - Objects Creation
		dut_serial.repl("rtc_conf = ConfTime(21,22)", 1)
		# 2 - Input Injection
		dut_serial.repl("rtc_conf.set_date_time("+str(set_datetime)+")", 0.6)
		self.double_serial.repl("a", 0.2)
		sleep(my_delay)
		# 3 - Results gathering
		gotten_datetime = dut_serial.repl("rtc_conf.get_date_time()",0.4)[2]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode() 
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (str(expected_datetime))

	#closes serial and make the descomissioning
	def tearDown(self):
		# 5 - descomissioning
		dut_serial.repl("del rtc_conf; del ConfTime", 0.4)
		self.double_serial.repl("x",0.6)

if __name__ == '__main__':
    unittest.main()
