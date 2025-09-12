gpio.to_out(25)

while 1 do
 
gpio.write(25, 1)
sys.post(" pin state", gpio.read(25))

sys.delay_ms(1)

gpio.write(25, 0)
--sys.post(" pin state", gpio.read(25))

sys.delay_ms(1)

--sys.post("one", 1)
--sys.post("two",2)

end
