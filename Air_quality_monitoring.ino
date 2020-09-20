#include <lmic.h>

#include <hal/hal.h>

#include <SPI.h>



#include "LowPower.h"  // TODO: remove and test. Low power is not useful for this sensor

int TX_COMPLETE = 0;



#ifdef CREDENTIALS

static const u1_t NWKSKEY[16] = NWKSKEY1;

static const u1_t APPSKEY[16] = APPSKEY1;

static const u4_t DEVADDR = DEVADDR1;

#else

static const u1_t NWKSKEY[16] = { 0xC9, 0xFF, 0xD6, 0x1F, 0x87, 0x74, 0xE8, 0xED, 0x89, 0xE2, 0x57, 0xB4, 0xA2, 0xF4, 0x7A, 0x9E };

static const u1_t APPSKEY[16] = { 0x47, 0x02, 0x7F, 0x29, 0x2C, 0x7E, 0xE9, 0xDC, 0xE9, 0x0B, 0xC8, 0x22, 0xF0, 0x09, 0x1F, 0xE9 };

static const u4_t DEVADDR = 0x2601183A;

#endif



void os_getArtEui (u1_t* buf) { }

void os_getDevEui (u1_t* buf) { }

void os_getDevKey (u1_t* buf) { }



static uint8_t mydata[] = "     ";

static osjob_t sendjob;



int i = 10; 

// Schedule TX every this many seconds (might become longer due to duty

// cycle limitations).

const unsigned TX_INTERVAL = 2000;  // mq: not used anymore because scheduling mechanism is not used

// Pin mapping Dragino Shield

const lmic_pinmap lmic_pins = {

    .nss = 10,

    .rxtx = LMIC_UNUSED_PIN,

    .rst = 9,

    .dio = {2, 6, 7},

};

 //Dust sensor starts

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000           


/*
I/O define
*/
const int iled = 7;                                            //drive the led of sensor
const int vout = 0;                                            //analog input

/*
variable
*/
float density, voltage;
int   adcvalue;

/*
private function
*/
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
   if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

//Dustsensor end

void onEvent (ev_t ev) {

    Serial.print(os_getTime());

    Serial.print(": ");

    switch(ev) {

        case EV_SCAN_TIMEOUT:

            Serial.println(F("EV_SCAN_TIMEOUT"));

            break;

        case EV_BEACON_FOUND:

            Serial.println(F("EV_BEACON_FOUND"));

            break;

        case EV_BEACON_MISSED:

            Serial.println(F("EV_BEACON_MISSED"));

            break;

        case EV_BEACON_TRACKED:

            Serial.println(F("EV_BEACON_TRACKED"));

            break;

        case EV_JOINING:

            Serial.println(F("EV_JOINING"));

            break;

        case EV_JOINED:

            Serial.println(F("EV_JOINED"));

            break;

        case EV_RFU1:

            Serial.println(F("EV_RFU1"));

            break;

        case EV_JOIN_FAILED:

            Serial.println(F("EV_JOIN_FAILED"));

            break;
            case EV_REJOIN_FAILED:

            Serial.println(F("EV_REJOIN_FAILED"));

            break;

            break;

        case EV_TXCOMPLETE:

            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));

                TX_COMPLETE = 1;



            if(LMIC.dataLen) {

                // data received in rx slot after tx

                Serial.print(F("Data Received: "));

                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);

                Serial.println();

            }

            // Schedule next transmission

           //

           os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);

            break;

        case EV_LOST_TSYNC:

            Serial.println(F("EV_LOST_TSYNC"));

            break;

        case EV_RESET:

            Serial.println(F("EV_RESET"));

            break;

        case EV_RXCOMPLETE:

            // data received in ping slot

            Serial.println(F("EV_RXCOMPLETE"));

            break;

        case EV_LINK_DEAD:

            Serial.println(F("EV_LINK_DEAD"));

            break;

        case EV_LINK_ALIVE:

            Serial.println(F("EV_LINK_ALIVE"));

            break; default:

            Serial.println(F("Unknown event"));

            break;

    }

}



void do_send(osjob_t* j){

    // Check if there is    not a current TX/RX job running

    if (LMIC.opmode & OP_TXRXPEND) {

        Serial.println(F("OP_TXRXPEND, not sending"));

    } else {

        // Prepare upstream data transmission at the next possible time.

        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);

        Serial.println(F("Packet queued"));

    }

    // Next TX is scheduled after TX_COMPLETE event.

}



void setup() {

    Serial.begin(115200);

    Serial.println(F("Starting"));



    #ifdef VCC_ENABLE

    // For Pinoccio Scout boards

    pinMode(VCC_ENABLE, OUTPUT);

    digitalWrite(VCC_ENABLE, HIGH);

    delay(1000);

    #endif



    // LMIC init

    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.

    LMIC_reset();

 // Set static session parameters. Instead of dynamically establishing a session

    // by joining the network, precomputed session parameters are be provided.

    #ifdef PROGMEM

    // On AVR, these values are stored in flash and only copied to RAM

    // once. Copy them to a temporary buffer here, LMIC_setSession will

    // copy them into a buffer of its own again.

    uint8_t appskey[sizeof(APPSKEY)];

    uint8_t nwkskey[sizeof(NWKSKEY)];

    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));

    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));

    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);

    #else

    // If not running an AVR with PROGMEM, just use the arrays directly 

    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    #endif



    // Set up the channels used by the Things Network, which corresponds

    // to the defaults of most gateways. Without this, only three base

    // channels from the LoRaWAN specification are used, which certainly

    // works, so it is good for debugging, but can overload those

    // frequencies, so be sure to configure the full frequency range of

    // your network here (unless your network autoconfigures them).

    // Setting up channels should happen after LMIC_setSession, as that

    // configures the minimal channel set.

    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band

    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band

    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

    // TTN defines an additional channel at 869.525Mhz using SF9 for class B

    // devices' ping slots. LMIC does not have an easy way to define set this

    // frequency and support for class B is spotty and untested, so this // frequency is not configured here.



    // Disable link check validation

    LMIC_setLinkCheckMode(0);
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)

    LMIC_setDrTxpow(DR_SF7,14);



    // Start job

   do_send(&sendjob);



  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);                                     //iled default closed
}

void loop()
{
  /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;

      mydata[0] = 0x04;    // message type: dust measurement

      mydata[1] = (int)density >> 8;

      mydata[2] = (int)density & 0xFF;
    
  /*
  display the result
  */
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n"); 

    LMIC_setTxData2(i, mydata, sizeof(mydata)-1, 0);

  
    delay(1000);

     os_runloop_once();
}



















