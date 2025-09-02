gpio.to_out(25)

while 1 do
 
   gpio.write(25, 1)
print("led on")
print( gpio.read(25))
    sys.delay_ms(500)
  print("led off")
gpio.write(25, 0)
print(gpio.read(25))
sys.delay_ms(500)
end
