import socket
import time
import select
try:
	client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client.connect(('127.0.0.1', 8890))
	# client.setblocking(0)
	# for i in range(1,3):
	client.sendall("hello world")
	recv_data = client.recv(1024)
	print recv_data
finally:
	client.close()
