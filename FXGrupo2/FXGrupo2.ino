//VER CHANGELOG AL FINAL

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
unsigned long timer = millis();

int ledStrip = A0;

const uint16_t PixelCount = 120; // make sure to set this to the number of pixels in your strip
const uint16_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const uint16_t AnimCount = PixelCount / 5 * 2 + 1; // we only need enough animations for the tail and one extra

const uint16_t PixelFadeDuration = 1000; // third of a second
// one second divide by the number of pixels = loop once a second
const uint16_t NextPixelMoveDuration = 5000 / PixelCount; // how fast we move through the pixels

NeoGamma<NeoGammaTableMethod> colorGamma; // for any fade animations, best to correct gamma

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);


// what is stored for state is specific to the need, in this case, the colors and
// the pixel to animate;
// basically what ever you need inside the animation update function
struct MyAnimationState
{
  RgbColor StartingColor;
  RgbColor EndingColor;
  uint16_t IndexPixel; // which pixel this animation is effecting
};

NeoPixelAnimator animations(AnimCount); // NeoPixel animation management object
MyAnimationState animationState[AnimCount];
uint16_t frontPixel = 0;  // the front of the loop
float lum = .1;     //este es la intensidad inicial
RgbColor frontColor = HslColor(1, 1, lum);  // cambia el color del barrido



void FadeOutAnimUpdate(const AnimationParam& param)
{
  lum = lum + 0.05;
  if (lum > 0.5) lum = 0.5;
  frontColor = HslColor(1, 1, lum);

  // this gets called for each animation on every time step
  // progress will start at 0.0 and end at 1.0
  // we use the blend function on the RgbColor to mix
  // color based on the progress given to us in the animation
  RgbColor updatedColor = RgbColor::LinearBlend(
                            animationState[param.index].StartingColor,
                            animationState[param.index].EndingColor,
                            param.progress);
  // apply the color to the strip
  strip.SetPixelColor(animationState[param.index].IndexPixel,
                      colorGamma.Correct(updatedColor));
}

void LoopAnimUpdate(const AnimationParam& param)
{
  // wait for this animation to complete,
  // we are using it as a timer of sorts

  if (param.state == AnimationState_Completed)
  {
    lum = .1;
    // done, time to restart this position tracking animation/timer
    animations.RestartAnimation(param.index);

    // pick the next pixel inline to start animating
    frontPixel = (frontPixel + 1) % PixelCount; // increment and wrap
    uint16_t indexAnim;
    // do we have an animation available to use to animate the next front pixel?
    // if you see skipping, then either you are going to fast or need to increase
    // the number of animation channels
    if (animations.NextAvailableAnimation(&indexAnim, 1))
    {
      animationState[indexAnim].StartingColor = frontColor;
      animationState[indexAnim].EndingColor = RgbColor(0, 0, 0); //color de fondo negro.
      animationState[indexAnim].IndexPixel = frontPixel;
      animations.StartAnimation(indexAnim, PixelFadeDuration, FadeOutAnimUpdate);
    }
  }

}


void setup()
{
  strip.Begin();
  strip.Show();
  Serial.begin(9600);
  // we use the index 0 animation to time how often we move to the next
  // pixel in the strip
  animations.StartAnimation(0, NextPixelMoveDuration, LoopAnimUpdate);
}


void loop()
{
  //esto en necesario para que se ejecute la animacion
  animations.UpdateAnimations();
  strip.Show();


  //Serial.println(frontPixel);
  if ((frontPixel == PixelCount - 1)) {
    animations.StopAnimation(0); //para la animacion del movimiento de los leds
    delay(random(10000, 20000)); //tiempo de espera despues del movimiento de los leds
    int fx = random(0, 10); //hay que mejorar el random, que elige entre latidos o flashes
    if (fx <= 8) {
      //LATIDOS
      for (int j = 0; j < random(3, 10); j++) { //aca se setea la cantidad de latidos
        for (int i = 50; i < 250; i++) {
          for (int x = 0; x < PixelCount; x++) {
            strip.SetPixelColor(x, colorGamma.Correct(RgbColor(i, 0, 0)));
            //por ejemplo aca no van a latir los led entre el 24 y el 39
            if (x > 24 && x < 39) strip.SetPixelColor(x, RgbColor(0)); //esta exepcion es para evitar que ciertos tramos latan
          }
          strip.Show();
          delay(20); // este es el tiempo del fade in del latido.
        }

        for (int i = 255; i > 50; i--) {
          for (int x = 0; x < PixelCount; x++) {
            strip.SetPixelColor(x, colorGamma.Correct(RgbColor(i, 0, 0)));
            //por ejemplo aca no van a latir los led entre el 24 y el 39
            if (x > 24 && x < 39) strip.SetPixelColor(x, RgbColor(0)); //esta exepcion es para evitar que ciertos tramos latan
          }
          strip.Show();
          delay(30); // este es el tiempo del fade out del latido.
          //HABRIA QUE VER SI DE IMPLEMENTAR UN EASING EN EL TIEMPO DEL LATIDO
        }
      }
      for (int i = 50; i > 0; i--) {
        for (int x = 0; x < PixelCount; x++) {
          strip.SetPixelColor(x, colorGamma.Correct(RgbColor(i, 0, 0)));
          //por ejemplo aca no van a latir los led entre el 24 y el 39
          if (x > 24 && x < 39) strip.SetPixelColor(x, RgbColor( 0)); //esta exepcion es para evitar que ciertos tramos latan
        }
        strip.Show();
        delay(15); // este es el tiempo del fade out  FINAL del latido.
      }
      delay(random(5000, 8000)); //Tiempo de espera despues de hacer los latidos
    }


    if (fx >= 5) {
      //FLASHES
      for (int j = 0; j < random(15, 60); j++) { //esta es la cantidad de flashes a realizar
        int pix = random(0, PixelCount);
        strip.SetPixelColor(pix, HslColor(.6, 1, .5)); // con .5 de luminancia max
        strip.Show();
        delay(10);
        strip.SetPixelColor(pix, HslColor(.6, 1, 0)); //0 luminncia para apagar.
        strip.Show();
      }
      delay(random(2000, 10000)); //Tiempo de espera despues de hacer los flashes
    }

    animations.StartAnimation(0, NextPixelMoveDuration, LoopAnimUpdate); //reanuda el movimiento de los leds
  }

  ///////////////////////////
  //este bloque es para manejar la tira de led RGB
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 255; i++) {
      analogWrite(ledStrip, i);
      delay(i + (255 / i)); //ver si funciona el easing el +1 es para evitar la indeterminacion
    }

    for (int i = 255; i > 0; i -- ) {
      analogWrite(ledStrip, i);
      delay(i + (255 / i) + 1); //ver si funciona el easing el +1 es para evitar la indeterminacion
    }
  }
  ////////   FIN RGB /////
}





