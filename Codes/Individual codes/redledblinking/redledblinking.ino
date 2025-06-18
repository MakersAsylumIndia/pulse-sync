int led_pin = 12;
int led_brightness = 0;
bool increasing = true;

void setup(){
    pinMode(led_pin, OUTPUT);
}

void loop(){
  if(increasing == true){
    // increase brightness value
    led_brightness = led_brightness + 1;
  }
  else{
    // decrease brightness value
    led_brightness = led_brightness - 1;
  }
  

  // if maximum brightness reached, start reducing brightness
  if(led_brightness >= 255){
    increasing = false;
  }
  // if minimum brightness reached, start increasing brightness
  else if(led_brightness <= 0){
    increasing = true;
  }

  // set the brightness of the led using the brightness value
  analogWrite(led_pin, led_brightness);

  // set a delay before changing brightness again
  delay(5);
}