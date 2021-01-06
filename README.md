# GR-working
This is my project for metering device of water I wrote when I worked at "Green radio". 
I used CubeMX to generate the backbone of my code. Then I wrote several modules like bypass protocol(thing that connects computer uart and sim7020 uart), sim7020 and sim7020_phy protocols (things that allows to communicate with TCP server and communicate with sim7020 on the physical layer), simple modules like adc stuff, rtc, http, md5, screen driver. Proprietary scheduler was used because of difficulties of low power mode on stm32. (instead of freertos f.e.)
