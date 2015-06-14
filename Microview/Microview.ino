#include <Time.h>  
#include <MicroView.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
//#define TIME_FUNCTION

char message[256] = "Very very very very very very very very very very very very very long string", queue[256];
int strLen = strlen(message);

void setup() {
      Serial.begin(9600);
  // put your setup code here, to run once:
    setSyncProvider( requestSync);  //set function to call when sync required
    uView.begin();
    uView.clear(PAGE);
    pinMode(6,OUTPUT);
    pinMode(5,OUTPUT);
    pinMode(3,OUTPUT);
    
#ifdef TIME_FUNCTION
    uView.setCursor(0,0);
    uView.print("Waiting\nfor sync\nmessage");
    uView.display();
    while(timeStatus() == timeNotSet) {    //Wait for an initial time sent through the serial port

    if (Serial.available() > 0) {
        processSyncMessage();    // Read the time and set the internal time (see Time lib)
      }
    }
#endif
    uView.clear(PAGE);
    uView.display();
     noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    
    OCR1A = 200;            // compare match register 16MHz/256/2Hz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12) | (1 << CS10);    // 1024 prescaler 
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt 
  //  Serial.flush();
///    while (Serial.read() >= 0) { }
    interrupts(); 
}

int j = 0, k, i = -2000;
bool lastRound = false, currentlyRunning = false;
int nDisplayed = -1;

ISR(TIMER1_COMPA_vect) {         // timer compare interrupt service routine
    if(i <= -strLen*(1+uView.getFontWidth())) {
        ++nDisplayed;
        if(lastRound && nDisplayed > 0) {
            TIMSK1 &= ~(1 << OCIE1A);      // Disable interrupt at the end of the string
            currentlyRunning = false;
                        digitalWrite(5,0);
        }
        else {
            i = uView.getLCDWidth();
            currentlyRunning = true;
            digitalWrite(5,1);
        }
    }
    else {  
//        uView.clear(PAGE);
        for(k=0; k < strLen; ++k) {
            if(i+k*(1+uView.getFontWidth()) >= -1-uView.getFontWidth() && 
                i+k*(1+uView.getFontWidth()) < LCDWIDTH) {
                uView.drawChar(i+k*(1+uView.getFontWidth()),j,message[k]);
            }
        }
        uView.display();
        --i;
    }
}

void loop() {
  // put your main code here, to run repeatedly:
     if(!currentlyRunning) {
         #ifdef TIME_FUNCTION
        digitalClockDisplay();
        #endif
    }
    if (Serial.available() > 0) {
        strcpy(queue, Serial.readStringUntil('\n').c_str());
        if(currentlyRunning) {
            lastRound = true;
            digitalWrite(6,1);
            while(currentlyRunning) { delay(100);}
            digitalWrite(6,0);
        }
        noInterrupts();
        uView.clear(PAGE);
        uView.display();
        strcpy(message, queue);
//digitalWrite(6,1);
  //      digitalWrite(6,0);
        strLen = strlen(message);
        nDisplayed = -1;
        i = -2000;
         if(message[strlen(message)-1] == '\r') {
                lastRound = false;
                message[strlen(message)-1] = '\0';
            }
        TIMSK1 |= (1 << OCIE1A);
        interrupts();


    }
}

void digitalClockDisplay(){
  // digital clock display of the time
  uView.setCursor(0,0);
  uView.setFontType(3);
  char str[10];
  sprintf(str, "%02d:%02d:%02d", hour(),minute(),second());
  uView.clear(PAGE);
  uView.print(str);
  uView.display();
  uView.setFontType(0);
  Serial.print(str);
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

/* Read the time from the serial port and set the timestamp */
void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
       adjustTime(3600);    //Add offset for timezone (GMT sent
     }
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}
 
