![Logo](resources/WortWecker-Ikea.jpg)

=======================

# Wort Wecker PingPong
Das hier vorgestellte Projekt basiert auf dem Retro Ping-Pong Spiel von Franzis.
Dabei wird die Uhrzeit als Text dargestellt.
Der Basiscode wurde um ein Funktionen erweitert, sodass es jetzt möglich ist, 
2 Weckzeiten zu definieren.
Über 3 Tasten kann die Uhrzeit und die Weckzeiten eingestellt werden.
Im nomalen Modus kann mit der UP-Taste der Alarm 1 und der DOWN-Taste der Alarm 2 
An- und Ausgeschaltet werden.

### Zusätzlich benötigte Bauteile
* Uhrenquarz 32,768 kHz
* 3 Taster
* Piezo Signalgeber
* Stiftleiste z.B. für ISP, Taster, Piezo und Vcc
* für NTP Zeit ein ESP8266 Modul mit UART Schnittstelle 5V !

## Installation
Für die Erweiterung mit ESP8266 Modul.
Hierfür verwende ich die ESP EASY Firmware und Rules.
Bei mir verwendete Version: R142_RC5
Infos zu ESP EASY: http://www.letscontrolit.com/
Dazu muss unter: 
* Tools --> Advanced --> Rules aktiviert werden.
* Baud Rate auf 9600 Baud.
* Serial Loglevel auf 4 
*Unter Rules folgenden Code einfügen:
```
On System#Boot do    //When the ESP boots, do
timerSet,1,20      //Set Timer 1 for the next event in 60 seconds
endon
On Rules#Timer=1 do  //When Timer1 expires, do
SYSTIME:%systime%
pulse,0,1,500
timerSet,1,20    //Set Timer 1 for the next event in 60 second
endon
```
Damit wird alle 20 Sekunden ein String "SYSTIME" erzeugt und die NTP Zeit über UART mit 9600 Baud 8N1 ausgegeben.
Der Controller prüft, ob die interne Uhr gleich der NTP Zeit ist. Falls dies abweicht, wird NTP in der Anzeige ausgeblendet.
Sobald ein gültiges NTP Signal empfangen wird (über UART) wird die Uhrzeit gesetzt und NTP in der Anzeige leuchtet auf.
Der Wecker kann natürlich auch ohne das ESP8266 Modul benutzt werden.

## Bauteile
B6 und B7 = Uhrenquarz  
C4 = Taster UP  
C5 = Taster DOWN  
D2 = Taster Stellen  
D0 = UART RXD an ESP8266 Modul (NTP)   
D1 = UART TXD an ESP8266 Modul (NTP)  
D3 = Piezo Signalgeber
## Compiler
Compiliert wurde das ganze im Atmel Studio 7 (7.0.1006)
Die Fuse muss noch auf den externen Uhrenquarz umgestellt werden. 

LowFuse: 0xE4  
HighFuse: 0xC9  

Beispiel avrdude
```
# Get fuses with USBasp
```
avrdude -c USBasp -p Atmega8 -n -v
```
# Ausgabe sieht so aus:
avrdude: auto set sck period (because given equals null)
avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.02s

avrdude: Device signature = 0x1e9307
avrdude: safemode: lfuse reads as E4
avrdude: safemode: hfuse reads as D9

avrdude: safemode: lfuse reads as E4
avrdude: safemode: hfuse reads as D9
avrdude: safemode: Fuses OK

avrdude done.  Thank you.

# Nun die Fuses richtig setzen
```
avrdude -c USBasp -p Atmega8 -U lfuse:w:0xE4:m -U hfuse:w:0xC9:m
```
# Die Ausgabe muss nun so aussehen:
avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.02s

avrdude: Device signature = 0x1e9307
avrdude: reading input file "0xE4"
avrdude: writing lfuse (1 bytes):

Writing | ################################################## | 100% 0.02s

avrdude: 1 bytes of lfuse written
avrdude: verifying lfuse memory against 0xE4:
avrdude: load data lfuse data from input file 0xE4:
avrdude: input file 0xE4 contains 1 bytes
avrdude: reading on-chip lfuse data:

Reading | ################################################## | 100% 0.02s

avrdude: verifying ...
avrdude: 1 bytes of lfuse verified
avrdude: reading input file "0xC9"
avrdude: writing hfuse (1 bytes):

Writing | ################################################## | 100% 0.02s

avrdude: 1 bytes of hfuse written
avrdude: verifying hfuse memory against 0xC9:
avrdude: load data hfuse data from input file 0xC9:
avrdude: input file 0xC9 contains 1 bytes
avrdude: reading on-chip hfuse data:

Reading | ################################################## | 100% 0.00s

avrdude: verifying ...
avrdude: 1 bytes of hfuse verified

avrdude: safemode: Fuses OK

avrdude done.  Thank you.
```
# Hochladen des Codes mit avrdude
Hier im Beispiel liegt die Hex Datei im gleichen Verzeichnis wie avrdude.exe   
avrdude -c USBasp -p Atmega8 –u –U flash:w:PingPong_Word_Clock_LWe.hex
```
avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.05s

avrdude: Device signature = 0x1e9307
avrdude: NOTE: FLASH memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: reading input file "PingPong_Word_Clock_LWe.hex"
avrdude: input file PingPong_Word_Clock_LWe.hex auto detected as Intel Hex
avrdude: writing flash (5886 bytes):

Writing | ################################################## | 100% 2.82s



avrdude: 5886 bytes of flash written
avrdude: verifying flash memory against PingPong_Word_Clock_LWe.hex:
avrdude: load data flash data from input file PingPong_Word_Clock_LWe.hex:
avrdude: input file PingPong_Word_Clock_LWe.hex auto detected as Intel Hex
avrdude: input file PingPong_Word_Clock_LWe.hex contains 5886 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 1.86s



avrdude: verifying ...
avrdude: 5886 bytes of flash verified

avrdude done.  Thank you.
```

## Benutzung
Mit langen Druck auf den Taster PD2 kommt man in die Einstellungen. Das Menü sieht wie folgt aus:
![Alt text](resources/menu.png)

Beim anlegen der Spannung erleuchten alle LED´s für etwa 10 Sekunden. Während dieser Zeit kann der Modus für den Alarm-Ausgang geändert werden.
Wird während dieser Zeit der "Hoch" Knopf gedrückt, kann man an den Ausgang einen Piezo anschließen. Dieser wird dann gepulst. Drückt man den 
"Runter" Knopf, ist der Ausgang im Modus Dauersignal. Damit könnte man dann z.B. einen MP3 Player anschließen (Danke an Reimund für die Idee).
Der Modus wird im EEProm abgespeichert. Man sieht nach den ersten 10 Sekunden dauerleuchten der LED´s anschließend für 4 Sekunden den Modus.
Dabei bedeutet gestrichelte Linie = Piezo, durchgezogene Linie = permanentes Signal. 

## Druckvorlage
Die Druckvorlage ist mit dem Tool "Gimp2" erstellt. Diese ist unter resources zu finden.
![Druckvorlage](resources/logo.png)

## Contributing
1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D

## History
Author:     Martin Steppuhn
* 18.12.2009 Initial version  

Editor:		  Lars Weimar  
* 06.10.2016  
Update to Atmel Studio 7  
Delete uart routine  

* 07.10.2016 ver.0.0.1  
Moved Word lines to get Alarm1 and Alarm2 to bottom  
Added Alarm1 and Alarm2 Vars and Words  
Added routines to set alarm times  
* 08.10.2016 ver.0.0.2  
Added ADC to get 2 more inputs on ADC6, ADC7  
Added UP/DOWN buttons  
* 09.10.2016 ver.0.0.3  
Added EEPROM to save alarm times  
* 09.10.2016 ver.0.0.4  
Changed to 24h clock  
Bugfix alarm
* 03.11.2016 ver.0.0.5  
Changed pins to get the UART ports free  
* 03.11.2016 ver.0.0.6  
add UART for NTP  
* 06.11.2016 ver.0.0.7  
add NTP sync  
Bugfix time & alarm setting  
* 12.11.2016 ver.0.0.8  
Bugfix NTP - hour < 10
* 13.11.2016 ver.0.0.9   
add light up all LED for 10 sec. by init (to place mask)     
* 20.11.2016 ver.1.0.0   
Bugfix display word_12 on 24h   
first release   
* 19.11.2017 ver.1.1.0   
Bugfix Piezo   
New feature - piezo or permanent signal   

## Quellen
Schaltplan: https://github.com/Eisbaeeer/WortWecker-PingPong/blob/master/resources/PingPongSchaltbild.jpg  
Platine: https://github.com/Eisbaeeer/WortWecker-PingPong/blob/master/resources/PingPongPlatine.jpg   
Ursprungs C-Code: www.emsystech.de  
 
## License
Copyright (c) Martin Steppuhn (www.emsystech.de)

Nur für den privaten Gebrauch / NON-COMMERCIAL USE ONLY

Die Nutzung (auch auszugsweise) ist für den privaten und nichtkommerziellen 
Gebrauch erlaubt. Eine Veröffentlichung und Weiterverwendung des Quellcodes 
ist möglich wenn diese Nutzungsbedingungen incl. Copyright beiliegen
und die Quelle verlinkt ist. (www.emsystech.de)

Bei kommerziellen Absichten nehmen Sie bitte Kontakt mit uns auf 
(info@emsystech.de)

Keine Gewähr auf Fehlerfreiheit, Vollständigkeit oder Funktion. Benutzung 
auf eigene Gefahr. Es wird keinerlei Haftung für direkte oder indirekte 
Personen- oder Sachschäden übernommen.
