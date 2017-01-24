-- reads 2 analogue voltages (Batt/Solar) via i2c from attiny85 adc
att_adr=0x13 --attiny adc
period=10000   -- allow time to check 2 sensors before xmit
adc2delay=tmr.create()
adc3delay=tmr.create()
write_i2c(att_adr,0x00,2) -- write # of adc to be sampled into reg 0 to start sampling
tmr.register(adc2delay,period,tmr.ALARM_SINGLE,function()
     -- get reg 2 = adc2 = battery
    BattVolts=string.format("%02d",string.byte(read_i2c(att_adr, 2, 1)))/25 
    print("\r\n BattVolts="..BattVolts)
    write_i2c(att_adr,0x00,3)
    -- don;t start next sample until first done
    tmr.register(adc3delay,period,tmr.ALARM_SINGLE,function()
        -- get reg 3 = adc3 = panel
        PvVolts=string.format("%02d",string.byte(read_i2c(att_adr, 3, 1)))/25 
        print("\r\n PvVolts="..PvVolts)
        end)
    tmr.start(adc3delay)    
    end)
tmr.start(adc2delay)

