import argparse
import serial
import os
import sys
import numpy as np

parser = argparse.ArgumentParser()
#parser.add_argument("tty")
#parser.add_argument("image_path")
args = parser.parse_args()

def checksum(bytecode):
    return int(np.array(list(bytecode), dtype=np.int32).sum())

def main():
	#print(args.tty)
	#print(args.image_path)
	ser = serial.Serial("/dev/ttyUSB0", 115200)
    
	try:
		print("Seial init seccuss!")
	except:
		print("Serial init failed!")
		exit(1)
	args.image_path = "./kernel8.img"
	image_path = args.image_path
	image_size = os.stat(image_path).st_size
	print(image_size)    
	with open(image_path, 'rb') as f:
		bytecode = f.read()

	image_checksum = checksum(bytecode)
	print("image checksum: %d",image_checksum)

	ser.write(image_size.to_bytes(4, byteorder="big"))
	ser.write(image_checksum.to_bytes(4, byteorder="big"))

	print(f"Image Size: {image_size}, Checksum: {image_checksum}")

	per_chunk = 128
	chunk_count = image_size // per_chunk
	chunk_count = chunk_count + 1 if image_size % per_chunk  else chunk_count

	for i in range(chunk_count):
		sys.stdout.write('\r')
		sys.stdout.write("%d/%d" % (i + 1, chunk_count))
		sys.stdout.flush()
		ser.write(bytecode[i * per_chunk: (i+1) * per_chunk])
		while not ser.writable():
			pass        

if __name__ =="__main__":
	main()
