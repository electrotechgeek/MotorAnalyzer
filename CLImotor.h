/*
 * MotorAnalyzer Serial communications - Motor
 */

void motorSetupCLI()
{
  char motorQuery = 'a';
  uint8_t validMotorQuery = false;
  cliBusy = true;
  
  Serial.print("Entering Motor Setup CLI...\n");
  
  while(true)
  {
    if (!validMotorQuery) 
      Serial.print("Motor CLI -> ");
    
    while (!Serial.available() && !validMotorQuery);
    
    if (!validMotorQuery) 
      motorQuery = Serial.read();
    
    if (!validMotorQuery) 
      Serial.print("\n");
      
    switch (motorQuery)
    {
      case '?':
        Serial.print("a - display current motor settings  | A - Change PWM Frequency in Hz            - ex: A400;\n");
        Serial.print("b -                                 | B - Change minimum armed throttle         - ex: B1200;\n");
        Serial.print("c -                                 | C - Change maximum throttle               - ex: C1989\n");
        /*Serial.print("d -                                 | D - \n");
        Serial.print("e -                                 | E - \n");
        Serial.print("f -                                 | F - \n");
        Serial.print("g -                                 | G - \n");*/
        Serial.print("\n");        
        Serial.print("w -                                 | W - Write values to EEPROM\n");
        break;
        
      case 'a':
        // print settings
        Serial.print("A - PWM Frequency: "); 
        validMotorQuery = false;
        motorQuery = 'x';
        break;
        
      case 'x':
        // quit
        Serial.print("Exiting Motor CLI...\n");
        cliBusy = false;
        validMotorQuery = false;
        return;
        break;
        
      case 'A':
        // change motor pwm frequency
        int temppwmFreq;
        temppwmFreq = readFloatSerial();
        Serial.print("Old PWM Freq: "); Serial.print(pwmFreq); Serial.print("\n");
        changePWMfreq(temppwmFreq);
        Serial.print(", new PWM Freq: "); Serial.print(pwmFreq); Serial.print("\n");
        Serial.print("Re-initializing motors at new frequency...");
        query = 'a';
        validQuery = false;
        break;
        
      case 'B':
        // change primary test execution length
        break;
        
      case 'W':
        // write eeprom
        Serial.println("writing values to EEPROM . . .");
        writeEEPROM();
        motorQuery = 'x';
        break;
    }
  }
}
