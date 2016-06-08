/*
* ****************************************************************************
* RP6 ROBOT SYSTEM - RP6 CONTROL M32 Examples
* ****************************************************************************
* Example: Afstandsmeting met de SRF05
*
* ****************************************************************************
*
*****************************************************************************/
// Includes:

#include "RP6ControlLib.h"       // The RP6 Control Library.
                        // Always needs to be included!

#include "RP6I2CmasterTWI.h"   // I2C Master Library

#include "main.h"

/*****************************************************************************/
/*****************************************************************************/
// Include our new "RP6 Control I2C Master library":

//#include "RP6Control_I2CMasterLib.h"

/*****************************************************************************/


int main(void)
{
   initRP6Control(); 

   initLCD();
   showScreenLCD("SRF05 afstands","meter");
   mSleep(1000);
   setLEDs(0b0000);
   
   sound(100,40,64);
   sound(170,40,0);
   clearLCD();
   
   setup();
      
   unsigned int range;   
   

   while(true)
   {    
      
      startRange();         // Send a high on the trigger pin to start a ranging
      range = getEcho();      // Wait for the echo line to go high and then measure the length of this high
      displayData(range);
      
      startTimer(0xFFFF);      // Delay before taking another ranging
      waitForTimer();
            
   }
return 0;   
}   
   

   // Afstandsmeting
void startTimer(unsigned int time)
{
   OCR1A = time;         // Minuteur(compte jusqu'a...)
   TIFR = 0x10;         // Clear timer campare match flag
   TCNT1 = 0x00;          // Clear timer to zero
   TCCR1B = START_CLK;      // Start timer running 1:8 prescaler
}

void waitForTimer(void)
{
   while(!(TIFR&0x10));   // wait for timer to set compare match flag
   TCCR1B = STOP_CLK;      // Stop the timer
}


void setup(void)
{
   DDRD=0x20;         // Port D pin 5 ouput triggerpin SRF05
   TCCR1A = 0x00;         // Set timer up in CTC mode
   TCCR1B = 0x08;   
}

 
void startRange(void)
{
   DDRD=0x20;               // make D5 output
   PORTD = ( 1<<PORTD5 );      // Send trigger pin D5 high
   startTimer(0x0007);         // Wait around 10uS before sending it low again
   waitForTimer();
   PORTD = ( 0<<PORTD5 );      // Send trigger pin D5 low
}

unsigned int getEcho(void)
{
uint8_t range;               
   DDRD=0x00;               // Make pin D5 input
   while(!(PIND&0x20));      // Wait for echo pin D5 to go high,  this indicates the start of the incoming pulse
   TCNT1 = 0x00;             // Clear timer to zero
   TCCR1B = START_CLK_N;      // Start timer running 1:8 prescaler in normal mode
   while((PIND&0x20));      // Wait for echo pin D5 to go low signaling that the pulse has ended
   TCCR1B = STOP_CLK;         // Stop the timer and set back to CTC mode   
   range = TCNT1/116;         // Read back value in the timer counter register, this number divided by 116 will give us the range in CM
   return(range);
   
}

void displayData(unsigned int r)
{
   clearLCD();
   writeStringLCD("Afstand is ");
   writeIntegerLCD(r, DEC);
   writeStringLCD(" cm");
} 

//  Defines

#define START_CLK   0x0A            // Start clock with 1:8 prescaler CTC mode
#define START_CLK_N   0x02            // Start clock running 1:8 prescaler in normal mode
#define STOP_CLK   0x08            // Stop clock
//#define LCD03      0xC6            // Address of LCD03

// Prototypes

// main.c
void startRange(void);
unsigned int getEcho(void);
void displayData(unsigned int r);
void setup(void);                  // General chip setup
void startTimer(unsigned int time);      // Starts the timer running for a number of uS defined by value of time
void waitForTimer(void);            // Waits for the timer to stop



/*autre exemple***/
// SRF05 connecté en mode un seul pin (émission/réception)

int pinSRF = 7; //pin digital pour l'envoi et la réception des signaux
int vSon=59; //valeur de temps en µs d'un aller retour du son sur 1cm
void setup() {
  Serial.begin(9600); //on initialise la communication série
}
//boucle principale
void loop() {
  int distance=mesureDistance(); //on récupère la valeur de distance grâce à la fonction créee plus bas
  Serial.println(distance); // on affiche la distance en cm
}

//fonction de mesure de distance avec SRF05
int mesureDistance() {
  unsigned long mesure = 0; // variable de mesure
  unsigned long cumul = 0; //variable pour la moyenne
  for (int t = 0; t < 10; t++) { // boucle pour effectuer 10 mesures
    pinMode (pinSRF, OUTPUT); //on prépare le pin pour envoyer le signal
    digitalWrite(pinSRF, LOW); //on commence à l'état bas
    delayMicroseconds(2); //on attend que le signal soit clair
    digitalWrite(pinSRF, HIGH);//mise à l'état haut
    delayMicroseconds(10); //pendant 10 µs
    digitalWrite(pinSRF, LOW); //mise à l'état bas
    pinMode(pinSRF, INPUT); //on prépare le pin pour recevoir un état
    mesure = pulseIn(pinSRF, HIGH); // fonction pulseIn qui attend un état haut et renvoie le temps d'attente
    cumul+=mesure; //on cumule les mesures
    delay(50); //attente obligatoire entre deux mesures
  }
  mesure=cumul/10; //on calcule la moyenne des mesures
  mesure=mesure/vSon;//on transforme en cm
  return mesure; //on renvoie la mesure au programme principal
}
/*
#define trigPin 13
#define echoPin 12
#define ledVerte 11
#define ledRouge 10

int trig = 12; 
int echo = 11; 
long lecture_echo; 
long cm;

void setup() 
{ 
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  pinMode(ledVerte, OUTPUT); 
  pinMode(ledRouge, OUTPUT); 
  digitalWrite(trig, LOW); 
  pinMode(echo, INPUT); 
  Serial.begin(9600); 
}

void loop(){ 
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trig, LOW); 
  lecture_echo = pulseIn(echo, HIGH); 
  cm = lecture_echo / 58; 
  Serial.print("Distancem : "); 
  Serial.println(cm); 
  delay(1000); 
}*/