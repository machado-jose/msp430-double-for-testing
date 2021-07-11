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

production_code = "dut_positioning.py"
auxiliar_code = "adafruit_gps.py"
build = "python -m mpy_cross -s -march=xtensa "
DUT_PORT = "/dev/ttyUSB0"
DOUBLE_PORT = "/dev/ttyACM1"
send = "ampy --port "

# From set-up:
# Building, connection and sending phase
# try:
# 	print("Building production code...")
# 	os.system(build+production_code)
# 	print("Building double code...")
# 	os.system(build+double_code)
# 	print("Building auxiliar code...")
# 	os.system(build+auxiliar_code)
# 	print("Cleaning the filesystem...")
# 	dut_serial = SerialInterface(DUT_PORT, 115200)
# 	dut_serial.connect_to_serial()
# 	dut_serial.clean_file_sys()
# 	dut_serial.close_serial()
# 	print("Sending built production code...")
# 	os.system(send+DUT_PORT+" put "+production_code)#.replace(".py",".mpy"))
# 	print("Sending built auxiliar_code...")
# 	os.system(send+DUT_PORT+" put "+auxiliar_code)#.replace(".py",".mpy"))
# except:
# 	sys.exit('fail in set-up phase')
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
		dut_serial.repl("from dut_positioning import Positioning", 0.1)

	def test_send_command_configuration(self):
		print("\nTesting the method send_command() configuring GPS to send GGA and RMC info")
		expected_command = 'PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0'
		print("Expected command: "+expected_command)
		# 1 - Objects Creation
		dut_serial.repl("tracker = Positioning(17,16,2)",0.5)
		# 2 - Input Injection
		dut_serial.repl("tracker.send_command('"+expected_command+"')", 0.5)
		sleep(1)
		# 3 - Results gathering
		gotten_command = self.double_serial.repl("y", 2.0)[0]
		# 4 - Assertion
		gotten_command = gotten_command.decode() 
		gotten_command = gotten_command.replace('\'','')
		print("Gotten command: "+gotten_command)
		gotten_command |should| equal_to (expected_command)

	def test_send_command_update_rate(self):
		print("\nTesting the method send_command() updating GPS rate to 1 second")
		command = 'PMTK220,1000'
		expected_answer = '$GPRMC,141623.523,A,2143.963,S,04111.493,W,,,301019,000.0,W*7B'
		print("Expected answer: "+expected_answer)
		# 1 - Objects Creation
		dut_serial.repl("tracker = Positioning(17,16,2)",0.5)
		# 2 - Input Injection
		dut_serial.repl("tracker.send_command('"+command+"')", 0.5)
		self.double_serial.repl("a", 1.5)
		sleep(1)
		# 3 - Results gathering
		gotten_answer = dut_serial.repl("tracker.received_command()",0.5)[2]
		# 4 - Assertion
		gotten_answer = gotten_answer.decode() # To transform from array bytes to String
		gotten_answer = '$' + gotten_answer.split('$')[1]
		print("Gotten answer: "+gotten_answer)
		gotten_answer |should| equal_to (expected_answer)

	def test_get_latitude(self):
		print("\nTesting the method get_latitude()")
		expected_latitude = -21.732717 
		print("Expected answer: "+str(expected_latitude))
		# 1 - Objects Creation
		dut_serial.repl("tracker = Positioning(17,16,2)",0.2)
		# 2 - Input Injection
		dut_serial.repl("tracker.send_command('PMTK220,1000')", 0.2)
		self.double_serial.repl("y", 1.2)[0]
		sleep(1)
		# 3 - Results gathering
		gotten_answer = dut_serial.repl("tracker.get_latitude()",0.2)[2]
		# 4 - Assertion
		gotten_answer = gotten_answer.decode() # To transform from array bytes to String 
		gotten_answer = float(gotten_answer)
		print("Gotten answer: "+str(gotten_answer))
		gotten_answer |should| close_to(expected_latitude, delta=0.000005)

	#Closes serial 
	def tearDown(self):
		# 5 Descomissioning
		dut_serial.repl("tracker.deinit(); del tracker; del Positioning",0.3)
		self.double_serial.repl("x", 0.6)
		pass

if __name__ == '__main__':
    unittest.main()
