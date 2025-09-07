gpio.to_out(25)

while 1 do
 
   gpio.write(25, 1)
print( gpio.read(25))
    sys.delay_ms(20)
gpio.write(25, 0)
print(gpio.read(25))
sys.delay_ms(20)
end
