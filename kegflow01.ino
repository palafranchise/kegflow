#include "LiquidCrystal_I2C.h"
#include "aws_iot_config.h"
#include <aws_iot_mqtt.h>
#include <aws_iot_version.h>
 
#define BACKLIGHT_PIN (3)
#define LED_ADDR (0x3F)  // might need to be 0x3F, if 0x27 doesn't work
LiquidCrystal_I2C lcd(LED_ADDR, 2, 1, 0, 4, 5, 6, 7, BACKLIGHT_PIN, POSITIVE) ;
 
// which pin to use for reading the flow sensor
#define FLOWSENSORPIN 7
 
volatile uint16_t pulses = 0; // count how many pulses!
volatile uint8_t lastflowpinstate; // track the state of the pulse pin
volatile uint32_t lastflowratetimer = 0; // you can try to keep time of how long it is between pulses
volatile float flowrate; // and use that to calculate a flow rate
float lastMetric = 0; // the last reported metric (we'll only send metrics in the run loop if we get new data)
float kegTotalLiters = 19.8; // the volume of the keg in liters
float retrievedLiters;

aws_iot_mqtt_client myClient; // init iot_mqtt_client
int rc = -100; // return value placeholder
char currentBeer_buf[21];
char usage_buf[25];
char usage_buf2[6];
char welcome_buf[13];
String payload;
String msg;
char msg2[128];
 
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}
 
void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}
 
 
void setup() {
 
  // Setup LCD
  lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines, turn on backlight
  
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);
 
  // Set up the client
  if((rc = myClient.setup(AWS_IOT_CLIENT_ID)) == 0) {
    // Load user configuration
    if((rc = myClient.config(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, AWS_IOT_ROOT_CA_PATH, AWS_IOT_PRIVATE_KEY_PATH, AWS_IOT_CERTIFICATE_PATH)) == 0) {
      // Use default connect: 60 sec for keepalive
      if((rc = myClient.connect()) == 0) {
        // Subscribe to "topic1"
        if((rc = myClient.subscribe("kegflow", 1, msg_callback)) != 0) {
          Serial.println("Subscribe failed!");
          Serial.println(rc);
        }

        // Get Shadow information
        lcd.setCursor(0, 0);
        lcd.print("Loading...");
      }
      else {
        Serial.println("Connect failed!");
        Serial.println(rc);
      }
    }
    else {
      Serial.println("Config failed!");
      Serial.println(rc);
    }
  }
  else {
    Serial.println("Setup failed!");
    Serial.println(rc);
  }
 
  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(2000);
}
 
void loop() // run over and over again
{ 

  myClient.shadow_init(AWS_IOT_MY_THING_NAME);
  myClient.shadow_get(AWS_IOT_MY_THING_NAME, kegflow_callback, 10);
  myClient.yield();

  lcd.setCursor(0, 0);
  lcd.print(welcome_buf);
  lcd.setCursor(0, 1);
  lcd.print(currentBeer_buf);

  lcd.setCursor(0, 3);
  lcd.print("Liters: ");
  lcd.print(usage_buf2);
  
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

  float usage = (atof(usage_buf2) + liters);
  
  //Serial.print(liters); Serial.println(" Liters");
  lcd.setCursor(0, 3);
 
  //File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
  if (liters > lastMetric) {
    lastMetric = liters;
    
    payload = "{\"state\":{\"reported\":";
    payload += "{\"liters\":";
    payload += usage;
    payload += "}}}";
    char JSON_buf[100];
    payload.toCharArray(JSON_buf, 100);
    Serial.println(JSON_buf);
    myClient.shadow_update(AWS_IOT_MY_THING_NAME, JSON_buf, strlen(JSON_buf), NULL, 5);
    
    msg = "{\"user\" : \"annon\", \"liters\" : " ;
    msg += liters;
    msg += ", \"date\" : \"";
    msg += "2016/05/04 01:30:08";
    msg += "\"}";
    msg.toCharArray(msg2, 128);
    if((rc = myClient.publish("kegflow", msg2, strlen(msg2), 1, false)) != 0) {
      Serial.println(msg2);
      Serial.println("Publish failed!");
      Serial.println(rc);
    }
  } else {
  } 
  retrievedLiters = 0.0f;
  delay(100);
}

void kegflow_callback(char* src, unsigned int len, Message_status_t flag) {
    myClient.getDesiredValueByKey(src, "currentBeer", currentBeer_buf, 50);
    myClient.getDesiredValueByKey(src, "welcomeMessage", welcome_buf, 50);
    myClient.getReportedValueByKey(src, "liters", usage_buf, 25);
    memmove(usage_buf2, usage_buf, 5);
}

// Basic callback function that prints out the message
void msg_callback(char* src, unsigned int len, Message_status_t flag) {
  if(flag == STATUS_NORMAL) {
    Serial.println("CALLBACK:");
    int i;
    for(i = 0; i < (int)(len); i++) {
      Serial.print(src[i]);
    }
    Serial.println("");
  }
}
