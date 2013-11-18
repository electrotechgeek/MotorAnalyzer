/* 
 * ISRs for sensor reading
 */
  
int16_t is2DeltaTime, is2PreviousTime = 0;



ISR(TIMER0_COMPA_vect)
{        //timer0 interrupt 1kHz - reads battery and load cells
  readLoadCell();
  measureBattery();
}

ISR(TIMER2_COMPA_vect)
{        //timer2 interrupt 100Hz - average battery and load
  is2DeltaTime = currentTime - is2PreviousTime;
  is2PreviousTime = currentTime;
  averageLoadCell();
  averageBattery(is2DeltaTime * 1000);
}



void initializeInterrupts() 
{
  cli(); // stop interrupts

  // set timer0 interrupt at 1kHz
  TCCR0A = 0; // set entire TCCR0A register to 0
  TCCR0B = 0; // same for TCCR0B
  TCNT0  = 0; //initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 250;// = (16*10^6) / (1000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler (CS02 = 0)
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A); 
  
  // set timer0 interrupt at 100Hz
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; //initialize counter value to 0
  // set compare match register for 2khz increments
  OCR2A = 250;// = (16*10^6) / (100*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS22 bit for 256 prescaler (CS20 and CS21 = 0)
  TCCR2B |= (1 << CS22);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A); 
  
  sei(); // allow interrupts
}
