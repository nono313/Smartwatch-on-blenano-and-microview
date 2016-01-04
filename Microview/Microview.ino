#include <Time.h>
#include <MicroView.h>
#include <Wire.h>
#include <QueueList.h>
//#include <MemoryFree.h> // Search for memory leak


#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
#define TIME_FUNCTION

char message[256] = "Very very very very very very very very very very very very very long string";
QueueList<String> queue;
int strLen = strlen(message);
bool queueFilled = false;

bool synced = false;

/**
 * Init function
 */
void setup() {
  /* Setup GPIO */
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);

  /* Setup communication protocols */
  Serial.begin(9600);
  Wire.begin(0x44>>1);                // join i2c bus with address #4
  
  Serial.print("BEGIN\n");

  /* Clean the screen */
  uView.begin();
  uView.clear(PAGE);

#ifdef TIME_FUNCTION
  uView.setCursor(0, 0);
  uView.print("Waiting\nfor sync\nmessage");
  uView.display();
  char buffer[50] = {0};

  /* Register callback for i2c sync messages */
  Wire.onReceive(syncEvent);

  /* 
   * Loop as long as a sync message has not been detected by the 
   * syncEvent callback.
   */
  while(synced == false) {
    delay(1000);
  }
#endif

  /* Register general callback for i2c incomming messages */
  Wire.onReceive(receiveEvent);

  /* Clear screen */
  uView.clear(PAGE);
  uView.display();
  
  noInterrupts();           // disable all interrupts
  /* Initialise Timer1 registers */
  TCCR1A = 0;   // Normal mode operation
  TCCR1B = 0;
  TCNT1  = 0;   //Timer value

  OCR1A = 200;            // Compare Match register 16MHz/1024 * 200 -> period = 0.0128 s
  TCCR1B |= (1 << WGM12);   // CTC mode = TOP is OCR1A value, the timer is set to 0 after the threshold is reached
  TCCR1B |= (1 << CS12) | (1 << CS10);    // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();
}

/* Globals */
int j = 0, k;
int initialCoord = -2000;        // Coordinate (inside or outside the screen) of the first character of the current string
// Flags
bool lastRound = false;        // Indicates that we want to stop displaying anything after the current string is shown
bool currentlyRunning = false;        // Gives the state of the screen (either displaying a scrolling message or not)
int nDisplayed = -1;

/*
 * Interrupt routine for the Timer
 * Used to scroll text on the screen of the Microview
 */
ISR(TIMER1_COMPA_vect) {         // timer compare interrupt service routine
  if (initialCoord <= -strLen * (1 + uView.getFontWidth())) {
    ++nDisplayed;
    /*
     * If the flag lastRound is set, we finish to display the current string
     * and then leave a screen empty by disabling the interrupt calling that routine
     */ 
    if (lastRound && nDisplayed > 0) {
      TIMSK1 &= ~(1 << OCIE1A);      // Disable interrupt at the end of the string
      currentlyRunning = false;
    }
    else {
      initialCoord = uView.getLCDWidth();
      currentlyRunning = true;
      //digitalWrite(5,1);
    }
  }
  else {
    //uView.clear(PAGE);
    for (k = 0; k < strLen; ++k) {
      if (initialCoord + k * (1 + uView.getFontWidth()) >= -1 - uView.getFontWidth() &&
          initialCoord + k * (1 + uView.getFontWidth()) < LCDWIDTH) {
        uView.drawChar(initialCoord + k * (1 + uView.getFontWidth()), j, message[k]);
      }
    }
    uView.display();
    --initialCoord;
  }
}

void loop() {
  queueFilled = !queue.isEmpty();
  // If not running, we display the time on the screen
  if (!currentlyRunning) {
#ifdef TIME_FUNCTION
    digitalClockDisplay();
#endif
  }
  if (queueFilled) {
    if (currentlyRunning) {
      lastRound = true;
      //            digitalWrite(6,1);
      while (currentlyRunning) {
        delay(100);
      }
      //            digitalWrite(6,0);
    }
    noInterrupts();
    uView.clear(PAGE);
    uView.display();
    strcpy(message, queue.pop().c_str());
    //digitalWrite(6,1);
    //      digitalWrite(6,0);
    strLen = strlen(message);
    nDisplayed = -1;
    initialCoord = -2000;
    //Display message in a loop if last character is \r
    if (message[strlen(message) - 1] == '\r') {
      lastRound = false;
      message[strlen(message) - 1] = '\0';
    }
    TIMSK1 |= (1 << OCIE1A);
    queueFilled = false;
    interrupts();
  }
}

void digitalClockDisplay() {
  // digital clock display of the time
  uView.setCursor(0, 0);
  uView.setFontType(3);
  char str[10];
  sprintf(str, "%02d:%02d:%02d", hour(), minute(), second());
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

void processSyncString(String str) {
   unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  //if (strncmp(TIME_HEADER, str, 1) == 0) {
  Serial.println(str);
    str.remove(0,1);
    Serial.println(str);
    pctime = str.toInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
      adjustTime(3600);    //Add offset for timezone (GMT sent
    }
  //}
}

void syncEvent(int howMany) {
  int i = 0;
  char buffer[256];
  //   Serial.println("!");
  Serial.print("Received ");
  Serial.print(howMany);
  Serial.println(" bytes");
  while (0 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    buffer[i++] = c;
    Serial.print(c);         // print the character
  }
  buffer[i] = '\0';

  Serial.println("Sync event");
  processSyncString(buffer);
  Serial.println(millis());
  synced = true;
}

/*
 * This function executes whenever data is received from master
 * this function is registered as an event, see setup()
 */
void receiveEvent(int howMany)
{
  digitalWrite(3, !digitalRead(3));
  int i = 0;
  char buffer[256];
  //   Serial.println("!");
  Serial.print("Received ");
  Serial.print(howMany);
  Serial.println(" bytes");
  while (0 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    buffer[i++] = c;
    Serial.print(c);         // print the character
  }
  buffer[i] = '\0';
  if(buffer[0] == 'T') {
#ifdef TIME_FUNCTION
      digitalWrite(5, !digitalRead(5));
      processSyncString(buffer);
#endif
  }
  else {
    memmove(buffer, buffer+1, strlen(buffer));
    Serial.print(queue.count());
    Serial.println(" elements in the queue");
    if(queue.count() < 10) {
        queue.push(buffer);
        queueFilled = true;    //Set flag for the main loop to copy from the queue to screen buffer (message)
    }
    else {
     //    Serial.println("Queue Filled !");
    }
    //  Serial.print("freeMemory()=");
    //Serial.println(freeMemory());
  }

}
