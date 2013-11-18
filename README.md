MotorAnalyzer
=============

There are thousands of combinations of motor, ESC, and propeller, but data on the overall performance of the common combinations is scarce, incomplete, and scattered across forums. The first step to solving this problem is to create a system that can accurately, reliably, and safely control a motor and log as much data as possible. MotorAnalyzer is an effort to design a system that can do all of this and more.

When purchasing a motor, propeller, or ESC, there are many questions you must answer before finding the best setup for your situation. How much thrust do you need? What are you going to do with this motor (airplane? multirotor?)? How fast of a response do you want? Which is most efficient? What size/pitch prop works best? Etc. etc... None of these questions are easily answered without just buying the motors and testing them yourself, but there are no data-logging-motor-analyzing systems out there that I'm aware of. If more people had access to a simple to use, easy to build motor analyzer, a database of information on various motors, propellers, and ESCs and as many combinations of these as you can imagine could be created.

The goal right now is to develop some software that works and can easily be improved upon. A simple Arduino base is used for now for its easy access.

Current functionality:
- motor control via standard brushless ESC - uses PWM communication
- load measurement using a load cell (wheatstone bridge, what's found in most small digital scales)
- current consumption via shunt resistor
- voltage via voltage dividing resistors
- A single test type - slowly ramp motor to full throttle and back down over 60sec
- Serial communication @ 115200 BAUD
-- Data is sent over serial in real time as test executes
-- Configurable frequency of the PWM signal sent to ESCs
- Battery and load sensing controlled via interrupts - sampled at 1KHz and averaged at 100Hz

Planned funcionality:
- Multiple test types
-- Response testing - how quickly does the motor reach a new throttle command?
---- configurable start and end throttle, configurable amount of start and end throttles (do just 30% to 60% or 30 to 60 to 20 to 100 etc.)
---- configurable time between each new throttle command
-- Hover testing - how efficient is the motor at hover throttle levels
---- configurable throttle, thrust, or power consumption targets (watts or Amps)
- Complete configurability
-- test functionality as outlined above
-- sensor setup
--- battery - voltage and current offset and scaling (for different sensors, and to accomodate a wide range of motor sizes)
--- ESC - PWM signal frequency, min and max throttle levels, calibration

And a GUI would be nice, need to get a more robust CLI in place first then work on that.