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

production_code = "dut_master.py"
build = "python -m mpy_cross -s -march=xtensa "
DUT_PORT = "/dev/ttyUSB0"
DOUBLE_PORT = "/dev/ttyACM1"
send = "ampy --port "
# From set-up:
# Building, connection and sending phase
# try:
# 	print("Building production code...")
# 	os.system(build+production_code)
# 	print("Cleaning the filesystem...")
# 	dut_serial = SerialInterface(DUT_PORT, 115200)
# 	dut_serial.connect_to_serial()
# 	dut_serial.clean_file_sys()
# 	print("Sending built production code...")
# 	os.system(send+DUT_PORT+" put "+production_code)#.replace(".py",".mpy"))
# except:
# 	sys.exit('fail to upload file(s)')
# Uncomment the next line for not to run the Test
# sys.exit()

print("Connecting to DUT device...")
dut_serial = SerialInterface(DUT_PORT, 115200)
dut_serial.connect_to_serial()

# Testing Phase
class Test_Template(unittest.TestCase):
	#Creates a serial connection and import the classes
	def setUp(self):
		print('\n')
		global dut_serial
		print("Connecting to DOUBLE device...")
		self.double_serial = SerialInterface(DOUBLE_PORT, 115200)
		self.double_serial.connect_to_serial()
		dut_serial.repl("from dut_master import Dut_master", 0.1)

	def test_reading_registers_without_indicating_address(self):
		print("\nTesting the method read_registers_wo_address()")
		expected_readings = [0,1,2,3]
		gotten_readings = []
		print("Expected Readings "+str(expected_readings))
		# 1 - Objects Creation
		dut_serial.repl("master = Dut_master(13,12,14,15)",0.2)
		# 2 - Input Injection
		self.double_serial.repl("1 2 3z", 1)
		# 3 - Results gathering
		gotten_readings = dut_serial.repl("master.read_registers_wo_address(4)",0.6)[2]
		gotten_readings = gotten_readings.decode()
		# 4 - Assertion
		print("Gotten value: "+gotten_readings)
		gotten_readings |should| equal_to (str(expected_readings))

	def test_reading_registers_indicating_address(self):
		print("\nTesting the method read_registers_w_address()")
		expected_readings = [0,1,2,3]
		expected_written = '14,0,0,0,'
		gotten_readings = []
		print("Expected Readings "+str(expected_readings))
		# 1 - Objects Creation
		dut_serial.repl("master = Dut_master(13,12,14,15)",0.2)
		# 2 - Input Injection
		self.double_serial.repl("0 1 2 3z", 1)
		# 3 - Results gathering
		gotten_readings = dut_serial.repl("master.read_registers_w_address(14,4)",0.6)[2]
		gotten_readings = gotten_readings.decode()
		gotten_written = self.double_serial.repl("G",0.6)[0]
		sleep(1)
		gotten_written = gotten_written.decode()
		# 4 - Assertion
		print("Gotten value: "+gotten_readings)
		gotten_readings |should| equal_to (str(expected_readings)) 
		gotten_written |should| equal_to (str(expected_written))

	def test_writing_registers_without_indicating_address(self):
		print("\nTesting the method write_registers_wo_address()")
		expected_written = '1,2,3,'
		gotten_values = []
		print("Expected written values "+str(expected_written))
		# 1 - Objects Creation
		dut_serial.repl("master = Dut_master(13,12,14,15)",0.2)
		# 2 - Input Injection 
		self.double_serial.repl("0 0 0z",1)
		dut_serial.repl("master.write_registers_wo_address([1,2,3])",1)
		# 3 - Results gathering
		gotten_values = self.double_serial.repl("G", 0.6)
		sleep(1)
		gotten_values = gotten_values[0].decode()
		# 4 - Assertion
		print("Gotten value: "+gotten_values)
		gotten_values |should| equal_to (expected_written)

	def test_writing_registers_indicating_address(self):
		print("\nTesting the method write_registers_w_address()")
		expected_written = '14,1,2,3,'
		print("Expected written values "+expected_written)
		# 1 - Objects Creation
		dut_serial.repl("master = Dut_master(13,12,14,15)",0.2)
		# 2 - Input Injection 
		self.double_serial.repl("0 0 0 0z",1)
		dut_serial.repl("master.write_registers_w_address(14, [1,2,3])",0.6)
		# 3 - Results gathering
		gotten_values = self.double_serial.repl("G", 1)
		sleep(1)
		gotten_values = gotten_values[0].decode()
		print(gotten_values)
		# 4 - Assertion
		print("Gotten value: "+gotten_values)
		gotten_values |should| equal_to (expected_written)

	#closes serial 
	def tearDown(self):
		dut_serial.repl("master.deinit(); del master; del Dut_master;", 0.4)
		self.double_serial.repl("R", 1)
		self.double_serial.close_serial()

if __name__ == '__main__':
    unittest.main()
