import serial  # PySerial


#Configuracion de puerto
ser=serial.Serial(
    port = 'COM5',
    baudrate = 115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
 )

print ('Puerto abierto: ', ser.name)
print(ser)
print('')
#Flush entrada y salida
ser.flushInput()
ser.flushOutput()

#Variable para ir handleando lecturas
out_lpc = ''

#Loop de lectura
while(1):

    #Leo byte, lo parseo a string
    read_byte = str(ser.read())

    #Si llego \n (salto de linea) imprimo recepcionn completa
    if (read_byte=='\n'):
        print(out_lpc)
    else:
        #No llego \n, entonces sigo recibiendo bytes
        out_lpc+=read_byte

