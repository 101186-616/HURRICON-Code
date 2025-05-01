const int numLEDs = 32;
const int decoderPins[5] = {2, 3, 4, 5, 6}; // LSB→MSB

const int playerButtons[4] = {7, 8, 9, 10};
const int playerLEDs[4][2] = {
  {31,  0},  // Player 1: wrap-around pair
  { 7, 8},  // Player 2
  { 15, 16},  // Player 3
  {23, 24}   // Player 4
};

int playerScores[4] = {0,0,0,0};
bool lastButtonState[4] = {HIGH,HIGH,HIGH,HIGH};
bool hasScoredThisRevolution[4] = {false,false,false,false};

int currentLED = 0;
int direction = 1;
float delayTime = 200 // starting delay time

void setup() {
  // Decoder pins
  for(int i=0;i<5;i++){
    pinMode(decoderPins[i], OUTPUT);
    digitalWrite(decoderPins[i], LOW);
  }
  // Buttons with pull-ups
  for(int i=0;i<4;i++){
    pinMode(playerButtons[i], INPUT_PULLUP);
  }
}

void loop(){
  setDecoderOutput(currentLED);

  // Edge-detect button presses & per-revolution lockout
  for(int p=0;p<4;p++){
    bool now = digitalRead(playerButtons[p]);
    if(lastButtonState[p]==HIGH && now==LOW && !hasScoredThisRevolution[p]){
      if(currentLED==playerLEDs[p][0] || currentLED==playerLEDs[p][1]){
        playerScores[p]++;
        hasScoredThisRevolution[p] = true;      // lock out until next wrap
        delayTime *= 0.87;                     // speed up

        if(playerScores[p]>=8){
          flashWinner(p);
          resetGame();
          return;
        }

        direction *= -1;  // bounce
        break;            // one score per step
      }
    }
    lastButtonState[p] = now;
  }

  // Advance LED and detect wrap transition
  static int prevLED = 0;
  prevLED = currentLED;
  currentLED = (currentLED + direction + numLEDs) % numLEDs;

  // **Only** reset lockout flags when we *cross* the boundary:
  if( (direction==1 && prevLED==numLEDs-1 && currentLED==0)
   || (direction==-1 && prevLED==0 && currentLED==numLEDs-1) ) {
    for(int i=0;i<4;i++) hasScoredThisRevolution[i]=false;
  }

  delay((int)delayTime);
}

void setDecoderOutput(int v){
  for(int b=0;b<5;b++) 
    digitalWrite(decoderPins[b], (v>>b)&1);
}

void flashWinner(int p){
  int a = playerLEDs[p][0], b = playerLEDs[p][1];
  unsigned long t0 = millis();
  while(millis() - t0 < 2000){
    setDecoderOutput(a); delay(250);
    setDecoderOutput(b); delay(250);
  }
}

void resetGame(){
  // turn everything off
  for(int i=0;i<5;i++) digitalWrite(decoderPins[i], LOW);

  // reset scores & flags
  for(int i=0;i<4;i++){
    playerScores[i]=0;
    hasScoredThisRevolution[i]=false;
    // sync button state so a held button won't auto-score
    lastButtonState[i] = digitalRead(playerButtons[i]);
  }

  currentLED = 0;
  direction = 1;
  delayTime = 200.0;
  delay(1000); // short break
}