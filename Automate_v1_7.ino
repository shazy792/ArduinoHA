     // Automate By Shazy792
//ARDUINO 1.0+ ONLY

//Libraries
#include <Ethernet.h>
#include <SPI.h>
#include <IRremote.h>
#include "DHT.h"

//Initialize Variables
boolean override = false;
boolean reading = false;
IRrecv irrecv(23);
decode_results results;
boolean out = false;
boolean playin = false;
boolean pcorpi = false;
boolean musicPlaying = false;
boolean rmusicPlaying = false;
IPAddress playerserver(192,168,2,102);
IPAddress rplayerserver(192,168,2,103);
boolean rspk = true;
String Title;
String Artist;
String Album;
boolean State;
boolean Stopped;
int i = 0;
int volume = 0;
boolean power = false;
int numon = 0;
float hum = 0;
float tempc = 0;
float tempf = 0;
float hind = 0;
boolean Startscreen = false;
boolean Screench = false;

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(2, DHT11);

// IP Configuration
byte ip[] = { 192, 168, 2, 101 };   //Manual setup only
byte gateway[] = { 192, 168, 2, 1 }; //Manual setup only
byte subnet[] = { 255, 255, 255, 0 }; //Manual setup only 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = EthernetServer(80); //port 80
EthernetClient player;

// Initial Setup
void setup()
{   //Pins 10,11,12 & 13 are used by the ethernet shield
  pinMode(47,OUTPUT); //Lamp
  pinMode(45,OUTPUT); //Tube Secondary
  pinMode(43,OUTPUT); //Extension
  pinMode(41,OUTPUT); //Saver
  pinMode(39,OUTPUT); //Blank
  pinMode(37,OUTPUT); //Tube
  pinMode(35,OUTPUT); //Speaker
  pinMode(33,OUTPUT); //Blank
  pinMode(31,OUTPUT); //Fan
  pinMode(24,INPUT); //Hall effect Input
  pinMode(32,INPUT); //Not used / Not sure :D
  pinMode(3,OUTPUT); //Fan power control
  pinMode(4,OUTPUT); //Fan speed control
  analogWrite(4,0); //Fan speed zero at boot

//Everything Low ot Boot
digitalWrite(47,LOW);
digitalWrite(45,LOW);
digitalWrite(43,LOW);
digitalWrite(41,LOW);
digitalWrite(39,LOW);
digitalWrite(37,LOW);
digitalWrite(35,LOW);
digitalWrite(33,LOW);
digitalWrite(31,HIGH); // Fan On at Boot
digitalWrite(3,LOW);

//Timer
  cli();//stop interrupts
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  OCR1A = 65534;//!!!! = (16*10^6) / (1*1024) - 1 (must be <65536) 
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();//allow interrupts

  //Serial.begin(9600); //Serial For Debugging
  irrecv.enableIRIn(); // Start the IR receiver
  Ethernet.begin(mac, ip, gateway, subnet); //for manual setup
  server.begin(); //Ethernet Server Begin
  dht.begin(); // DHT Sensor Begin
}

// Calling Functions Repeatedly
void loop()
{ checkForClient();
  remote();  
  door();
  //night();
  startsc();
}
//Sub Functions
void fan()
{  digitalWrite(31,!digitalRead(31)); coolfan();}
 void lamp()
{  digitalWrite(47,!digitalRead(47)); coolfan();}
 void saver()
{  digitalWrite(41,!digitalRead(41)); coolfan();}
 void tube()
{  digitalWrite(37,!digitalRead(37)); coolfan();}
 void exten()
{  digitalWrite(43,!digitalRead(43)); coolfan();}
 void speaker()
{  digitalWrite(35,!digitalRead(35)); coolfan();}
 void tubese()
{  digitalWrite(45,!digitalRead(45)); coolfan();}

 void rspeaker()
 { if (rspk == true){rSspk("GET /False HTTP/1.1"); rspk = false;}
else if (rspk == false){rSspk("GET /True HTTP/1.1"); rspk = true;}
 }

 void beep(int x)
{ tone(22,3200,x);
 }
 void gettemp()
{ hum = dht.readHumidity(); // Read Humidity
  tempc = dht.readTemperature(); // Read temperature as Celsius
  tempf = dht.readTemperature(true); // Read temperature as Fahrenheit (For Heat Index i.e Real Feel)
  hind = dht.computeHeatIndex(tempf, hum);
  hind = hind - 32;
  hind = hind * 5;
  hind = hind / 9;
}
//Cooling Fan Conditional
void coolfan()
{ 
  for (int cnt = 31; cnt < 49; cnt+=2)
{ if (digitalRead(cnt)==1){numon += 1;} Serial.print(cnt);}
if (numon == 1){analogWrite(4,20); if (digitalRead(3) == 0) {digitalWrite(3,HIGH);}}
else if (numon == 2){analogWrite(4,60); if (digitalRead(3) == 0) {digitalWrite(3,HIGH);}}
else if (numon == 3){analogWrite(4,80); if (digitalRead(3) == 0) {digitalWrite(3,HIGH);}}
else if (numon == 4){analogWrite(4,100); if (digitalRead(3) == 0) {digitalWrite(3,HIGH);}}
else if (numon > 4){analogWrite(4,120); if (digitalRead(3) == 0) {digitalWrite(3,HIGH);}}
else {analogWrite(4,0); if (digitalRead(3) == 1) {digitalWrite(3,LOW);}}
numon = 0;
}

 //Ethernet Server
 void checkForClient(){
 EthernetClient client = server.available();
if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;

    while (client.connected()) {
      if (client.available()) {

        if(!sentHeader){
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          sentHeader = true;
        }

        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info

        if(reading){

           switch (c) {
            case '2':
            saver();
              break;
            case '3':
            tube();
              break;
            case '4':
            lamp();
              break;
            case '5':
            fan();
              break;
            case '6':
            exten();
              break;
            case '7':
            speaker();
              break;
            case '8':
            rspeaker();
              break;
            case '9':

              break;
            case '10':

              break;
            case '11':
            tubese();
              break;
            case 'a':
              { 
                client.print("Saver = ");
                client.print(digitalRead(41));
                }
              break;
              case 'b':
              { 
                client.print("Tube = ");
                client.print(digitalRead(37));
                }
              break;
              case 'c':
              { 
                client.print("Lamp = ");
                client.print(digitalRead(47));
                }
              break;
              case 'd':
              { 
                client.print("Fan = ");
                client.print(digitalRead(31));
                }
              break;
              case 'e':
              { 
                client.print("Extension = ");
                client.print(digitalRead(43));
                }
              break;
              case 'f':
              { 
                client.print("Speaker = ");
                client.print(digitalRead(35));
                }
                break;
              case 'g':
              client.print("Rpi Fan = ");

                break;
              case 'h':
              client.print("Rpi HDD = ");

                break;
              case 'i':
              client.print("Rpi Speaker = ");
              client.print(rspk);
                break;
              case 'j':
              client.print("Tubese = ");
              client.print(digitalRead(45));
                break;
              case 'z':
              { 
                for (int i = 31; i < 49; i+=2)
                { client.print(i);
                  client.print("=");
                  client.print(digitalRead(i));
                }
                client.print("Rpi Fan = ");

                client.print("Rpi HDD = ");

                client.print("Rpi Speaker = ");
                client.print(rspk);  
            }
              break;
          case 's':
              { sleep();
                }
              break;  
    case 'k':
      delay(25);
      rMusicState();
      client.print(Title);
      client.print("\n");
      client.print(Artist);
      client.print("\n");
      client.print(Album);
      client.print("\n");
      client.print(State);
    break; 
   case 'l':
     Out();
    break;
     case 'm':
    if (pcorpi == true){musicVar("MediaPlayPause");if (musicPlaying == true) {musicPlaying =false;}else if (musicPlaying == false) {musicPlaying = true;}}
    else if (pcorpi == false){rMusicState(); if (State == true){ rmusicVar("pause");} else if (State == false) {rmusicVar("play");}}
     break;  
 case 'n':
    if (pcorpi == true){musicVar("MediaNextTrack");}
    else if (pcorpi == false){rmusicVar("next");}
     break;
 case 'o':
    if (pcorpi == true){musicVar("MediaPreviousTrack");}
    else if (pcorpi == false){rmusicVar("previous");}
     break;
      case 'p':
    if (pcorpi == true){musicVar("VolumeUp");}
    else if (pcorpi == false){rMusicVolume();rmusicVar("setvol "+String(volume + 5));}
     break;
      case 'q':
    if (pcorpi == true){musicVar("VolumeDown");}
    else if (pcorpi == false){rMusicVolume();rmusicVar("setvol "+String(volume - 5));}
     break;
  case 'r':
     gettemp();
     client.print(hum);
     client.print("%\n");
     client.print(tempc);
     client.print("*C\n");
     client.print(hind);
     client.print("*C");
    break;
   }
   }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

while (client.connected()){client.stop();}
//client.stop(); // close the connection:

  } 
}

// IR Control
void remote()
{
  if (irrecv.decode(&results)) {
  switch (results.value)
  {
  case 251:
  case 3778927144:
      beep(50); // 1
      saver();
  break;
  case 16635: // 2
    beep(50);
    tube();
  break;
  case 8443: // 3
    beep(50);
    lamp();
  break;
  case 24827: // 4
    beep(50);
    fan();
  break;
  case 4347: // 5
    beep(50);
    exten();
  break;
  case 20731: // 6
    beep(50);
    speaker();
  break;
  case 30203: //sharpness
    beep(50);
      Alloff();
  break;
  case 14331: // osd
    beep(50);
    Out();
  break;
  case 27643: // play/pause
  case 16720605:
    beep(50); 
    if (pcorpi == true){musicVar("MediaPlayPause");if (musicPlaying == true) {musicPlaying =false;}else if (musicPlaying == false) {musicPlaying = true;}}
    else if (pcorpi == false){rMusicState(); if (State == true){ rmusicVar("pause");} else if (State == false) {rmusicVar("play");}}
  break;
  case 18171:
  case 16761405:
    beep(50); // next
    if (pcorpi == true){musicVar("MediaNextTrack");}
    else if (pcorpi == false){rmusicVar("next");}
  break;
  case 1787: // previous
  case 16712445:
    beep(50);
    if (pcorpi == true){musicVar("MediaPreviousTrack");}
    else if (pcorpi == false){rmusicVar("previous");}
  break;
  case 17147: // select
  case 16748655:
    beep(50);
    if (pcorpi == true){musicVar("VolumeUp");}
    else if (pcorpi == false){rMusicVolume();rmusicVar("setvol "+String(volume + 5));}
  break;
  case 12027: // slow
  case 16754775:
    beep(50);
    if (pcorpi == true){musicVar("VolumeDown");}
    else if (pcorpi == false){rMusicVolume();rmusicVar("setvol "+String(volume - 5));}
  break;
  case 11259: // PBC
    beep(50);
    lock();  
  break;
  case 3835: //stop
  case 16769055:
    beep(50);
    rmusicVar("stop");
  break;
  case 29435: // >10
    beep(50);
    sleep();  
  break;
  case 10235: //return
    beep(50);
    playin = true;
    musicPlaying = true;
  break;
  case 18427: //v-index
    if (override == true) {beep(50);override = false;}
    else if (override == false) {beep(50);delay(100);beep(50);delay(100);beep(50);override = true;}
  break;
  case 12539: //7
    beep(50);
    rspeaker();
  break;
  case 28923: //8
    beep(50);

  break;
  case 2299: //9
    beep(50);

  break;
  case 18683: //0
    beep(50);
    tubese();
  case 763: // play mode
    beep(50);
    if (pcorpi == true) {pcorpi =false;}
      else if (pcorpi == false) {pcorpi = true;}
  break;
  case 16753245: // power R2
    beep(50);
    if (power == false ) {
    if (digitalRead(38)==0){speaker();}
    if (rspk == false ){rspeaker();}  
    power = true; }
      else if (power == true){
    if (digitalRead(38)==1){speaker();}
    if (rspk == true){rspeaker();}  
    power = false; }
  break;
  case 2811: // Time
    beep(50);
    if (Startscreen == false ) {Startscreen = true;}        
    else if (Startscreen == true ) {Startscreen = false;}
  break;

}
        irrecv.resume(); // Receive the next value
}
  
}

// Check Door Status
void door()
 {
  if (out == true&&digitalRead(24)==1) {
 if (digitalRead(41)==0){delay(750); beep(50); saver(); }
 rMusicState();
if (playin==true&&rmusicPlaying==false){ if (rspk == false){rspeaker();}delay(300);rmusicVar("play"); playin = false;}
out = false;
  }
   }

// Out true
void Out()
{     delay(5000);
      if (override == false){
      if (digitalRead(41)==1){saver();}
      if (digitalRead(37)==1){tube();}
      if (digitalRead(45)==1){tubese();}
      if (digitalRead(47)==1){lamp();}
      if (digitalRead(31)==1){fan();}
      if (digitalRead(35)==1){speaker();}
      rMusicState();
      if (rmusicPlaying == true) {rmusicVar("pause");}
      if (rspk == true) {rspeaker();}}  
      lock();
      out = true;
 }

// Fan on when night
//void night()
//{ 
//if (digitalRead(32)==1&&digitalRead(5)==0){ pin5();}
//}

// Music Control For PC 
void musicVar(String com){
    String final = "GET /action?type=Keyboard&command=%7B"+com+"%7D%2C HTTP/1.1";
    char command[80];
  final.toCharArray(command,sizeof(command));
    //Serial.print(command);
if (player.connect(playerserver,8655)){
   player.println(command); 
   delay(25);
}
while (player.connected()){player.stop();}
   player.flush();
}

// Music Control For R Pi 
void rmusicVar(String com){  
  if (player.connect(rplayerserver,6600)){
   player.println(com);
   player.println("close");
   delay(25);
}
while (player.connected()){player.stop();}
   player.flush();
}


// R Pi Music State
void rMusicState() { 
String inData = "";
boolean reading = true;
  if( player.connect(rplayerserver,6600)){
      player.println("status");
      player.find("state: "); // variations = artist? album? title? count? random? repeat? state?
while (reading == true)
   { char c = player.read();
     inData += c;
     // Process message when new line character is recieved
      if (c == '\n')
   { inData.trim(); 
      if (inData == "play"){rmusicPlaying = true; State = true; Stopped = false;}
      else if (inData == "pause"){rmusicPlaying = false; State = false; Stopped = false;}
      else if (inData == "stop"){rmusicPlaying = false; Stopped = true; State = false;}
      inData = ""; // Clear recieved buffer
      reading = false;       
    }
        }
player.println("close");  
   }
while (player.connected()){player.stop();}
player.flush();
delay(25);
rSongInfo();
}


//Rpi Song Info
void rSongInfo() { 
String inData;
boolean reading = true;
  if( player.connect(rplayerserver,6600)){
      player.println("currentsong");
      player.find("Artist: "); // variations = artist? album? title? count? random? repeat? state?
  while (reading == true)
   { char c = player.read();
     inData += c;
     
    // Process message when new line character is recieved
      if (c == '\n')
  {    Artist = inData;
      reading = false;
      inData = ""; // Clear recieved buffer
   }
      }

reading = true;
player.find("Title: "); // variations = artist? album? title? count? random? repeat? state?
while (reading == true)
   { char c = player.read();
     inData += c;
     // Process message when new line character is recieved
      if (c == '\n')
      {  Title = inData;
         reading = false;
         inData = ""; // Clear recieved buffer
      }
        }
 
 reading = true;
player.find("Album: "); // variations = artist? album? title? count? random? repeat? state?
while (reading == true)
   { char c = player.read();
     inData += c;
     // Process message when new line character is recieved
      if (c == '\n')
   {  Album = inData;
      reading = false;
      inData = ""; // Clear recieved buffer
   }
        }
player.println("close");  
}  
while (player.connected()){player.stop();}
player.flush();
delay(25);
}

//Rpi Volume
void rMusicVolume() { 
String inData = "";
boolean reading = true;
  if( player.connect(rplayerserver,6600)){
      player.println("status");
      player.find("volume: "); //variations :: check mpd documentation
  while (reading == true)
   { 
     
    char c = player.read();
        inData += c;
    if (c == '\n')
  { 
    //Serial.println(inData);
    volume = inData.toInt();
    inData = ""; // Clear recieved buffer
    reading = false;
   }
     }
player.println("close");  
   }
while (player.connected()){player.stop();}
player.flush();
delay(25);
}


//Lock PC
void lock(){
if (player.connect(playerserver,8655)){
     player.println("GET /pc?mode=lock HTTP/1.1"); 
     delay(25);
}
while (player.connected()){player.stop();}
   player.flush();
 }

// Sleep Command
void sleep()
{ if (digitalRead(41)==1){saver();}
if (digitalRead(37)==1){tube();}
if (digitalRead(45)==1){tubese();}
if (digitalRead(47)==0){lamp();}
TCCR1B = ((1<<CS12) | (1<<CS10) | (1<<WGM12));
}

// R Pi Pin Control
void rSspk(String comm) { 
    //Serial.print(comm);
    if (player.connect(rplayerserver,8000)){
   player.println(comm); 
   delay(25);
}
while (player.connected()){player.stop();}
   player.flush();
delay(25);
}

// All Off
void Alloff()
{
      if (digitalRead(31)==1){fan();}
      if (digitalRead(47)==1){lamp();}  
      if (digitalRead(41)==1){saver();}
      if (digitalRead(37)==1){tube();}
      if (digitalRead(45)==1){tubese();}
      if (digitalRead(43)==1){exten();}
      if (digitalRead(35)==1){speaker();}
}

// Interrupt Servive Routine
ISR(TIMER1_COMPA_vect) {//Interrupt at freq of 1kHz to measure reed switch
i++;
if (i == 8){
lamp();
i = 0;
TCCR1B = 0;}
}

//Start Screen on door open
void startsc()
{
    if (Startscreen == true&&digitalRead(24)==1&&Screench == false) { Screench = true; musicVar("Rwin"); delay(10); }
    else if (digitalRead(24)==0&&Screench == true) {  Screench = false;}

}
