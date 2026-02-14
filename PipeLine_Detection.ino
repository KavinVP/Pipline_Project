#include <WiFi.h>

const char* ssid = "Galaxy";
const char* password = "12345678";

#define FLOW_IN  14
#define FLOW_OUT 27

volatile int pulseCountIn = 0;
volatile int pulseCountOut = 0;

float flowRateIn = 0;
float flowRateOut = 0;

WiFiServer server(80);

unsigned long previousMillis = 0;
const long interval = 1000;

void IRAM_ATTR pulseCounterIn() {
  pulseCountIn++;
}

void IRAM_ATTR pulseCounterOut() {
  pulseCountOut++;
}

void setup() {
  pinMode(FLOW_IN, INPUT_PULLUP);
  pinMode(FLOW_OUT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(FLOW_IN), pulseCounterIn, RISING);
  attachInterrupt(digitalPinToInterrupt(FLOW_OUT), pulseCounterOut, RISING);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }

  server.begin();
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    flowRateIn = (pulseCountIn / 7.5);
    flowRateOut = (pulseCountOut / 7.5);

    pulseCountIn = 0;
    pulseCountOut = 0;
  }

  WiFiClient client = server.available();

  if (client) {

    float difference = flowRateIn - flowRateOut;
    bool leak = false;

    if (difference > 0.5) {   // Threshold
      leak = true;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><head><title>Pipeline Monitor</title></head><body>");
    client.println("<h2>Pipeline Monitoring System</h2>");
    client.println("<p>Flow In: " + String(flowRateIn) + " L/min</p>");
    client.println("<p>Flow Out: " + String(flowRateOut) + " L/min</p>");

    if (leak) {
      client.println("<h3 style='color:red;'>LEAK DETECTED!</h3>");
    } else {
      client.println("<h3 style='color:green;'>System Normal</h3>");
    }

    client.println("</body></html>");

    client.stop();
  }
}
