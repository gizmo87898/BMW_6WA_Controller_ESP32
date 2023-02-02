import socket
import struct
import serial
# Create UDP socket.
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Open Serial port
ser = serial.Serial('COM3', 115200, timeout=0.5)


# Bind to BeamNG OutGauge.
sock.bind(('127.0.0.1', 4444))

while True:
    data = sock.recv(96)
    if not data:
        break
    outsim_pack = struct.unpack('I4sH2c7f2I3f16s16si', data)
    serialpacket = bytes(str(round(outsim_pack[5],2)) + ';' + str(int(outsim_pack[6])) + ";" + str(int(outsim_pack[11])) + ";" + str(int(outsim_pack[9]*100)) + ";", "utf-8")
    # speed,rpm, engtemp,
    ser.write(serialpacket)
    #print(serialpacket)
    print(format(outsim_pack[13], '#018b'))



# Release the socket.
sock.close()

# release com port
ser.close()