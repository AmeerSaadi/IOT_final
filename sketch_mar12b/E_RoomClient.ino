#include <DHT_U.h>
#include <DHT.h>

#define FAN_PIN D4

#define DHTPIN D7
#define DHTTYPE DHT22

#define MUX_A D5
#define MUX_B D6
#define MUX_C D7
#define MUX_IN A0

#define RED_LED D1
#define GREEN_LED D2
#define BLUE_LED D3

#define RED_BTN D5
#define GREEN_BTN D6
#define BLUE_BTN D7

DHT dht(DHTPIN, DHTTYPE);
