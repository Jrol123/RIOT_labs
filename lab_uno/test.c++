
#define BUTTON_PIN 2
#define NUM_LEDS 4
#define LONG_PRESS_TIME 1000 // время длительного нажатия для переключения режима

int ledPins[NUM_LEDS] = {3, 4, 5, 6};
bool ledStates[NUM_LEDS] = {false, false, false, false};

unsigned long lastMillis[NUM_LEDS] = {0, 0, 0, 0};
unsigned long interval[NUM_LEDS] = {1000, 500, 20, 10}; // Интервалы для мигания светодиодов

unsigned long buttonPressedTime = 0;
bool buttonWasPressed = false;
bool longPress = false;
bool mode = false; // false = режим 1, true = режим 2

void setup()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        pinMode(ledPins[i], OUTPUT);
    }
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    checkButton();
    if (!mode)
    {
        runMode1();
    }
    else
    {
        runMode2();
    }
}

void checkButton()
{
    bool buttonState = digitalRead(BUTTON_PIN) == LOW;
    if (buttonState && !buttonWasPressed)
    {
        buttonPressedTime = millis();
        buttonWasPressed = true;
        longPress = false;
    }

    if (!buttonState && buttonWasPressed)
    {
        if (millis() - buttonPressedTime > LONG_PRESS_TIME)
        {
            mode = !mode; // Переключение режима
        }
        else
        {
            rotateLEDs(); // Перемещение функций светодиодов
        }
        buttonWasPressed = false;
    }
}

void runMode1()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (i == 0 || i == 1)
        {
            blinkLED(i, i == 0 ? interval[i] : interval[i] / 2);
        }
        else if (i == 2)
        {
            fadeLED(i, 20);
        }
        else if (i == 3)
        {
            fadeLED(i, 5);
        }
    }
}

void runMode2()
{
    static int currentLED = 0;
    static unsigned long changeTime = 0;
    if (millis() - changeTime >= interval[2])
    {
        digitalWrite(ledPins[currentLED], LOW);
        currentLED = (currentLED + 1) % NUM_LEDS;
        digitalWrite(ledPins[currentLED], HIGH);
        changeTime = millis();
    }
}

void blinkLED(int led, unsigned long interval)
{
    if (millis() - lastMillis[led] >= interval)
    {
        ledStates[led] = !ledStates[led];
        digitalWrite(ledPins[led], ledStates[led] ? HIGH : LOW);
        lastMillis[led] = millis();
    }
}

void fadeLED(int led, int step)
{
    static int brightness = 0;
    static bool increasing = true;
    static unsigned long fadeMillis = 0;

    if (millis() - fadeMillis >= step)
    {
        analogWrite(ledPins[led], brightness);
        if (increasing)
        {
            brightness += 5;
            if (brightness >= 255)
                increasing = false;
        }
        else
        {
            brightness -= 5;
            if (brightness <= 0)
                increasing = true;
        }
        fadeMillis = millis();
    }
}

void rotateLEDs()
{
    // Rotate led functions
    if (!longPress)
    {
        longPress = true;
        unsigned long tempInterval = interval[NUM_LEDS - 1];
        for (int i = NUM_LEDS - 1; i > 0; i--)
        {
            interval[i] = interval[i - 1];
        }
        interval[0] = tempInterval;
    }
}