//Инициализируем библиотеки
#include <Arduino.h>
#include <cgAnem.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#define ADC_pin 34 //задаём значение пина АЦП

ClimateGuard_Anem cgAnem(ANEM_I2C_ADDR); // Инициализируем CG_Anem
TFT_eSPI tft = TFT_eSPI();  // Инициализируем дисплей

uint32_t ADC;  // Переменная для значений АЦП
uint32_t timer_meas; // Таймер для измерений анемометра
uint32_t timer_bat;  // Таймер для измерения заряда батареи

// Функция используется для мигания индикатора батареи при низком заряде
void lowbat(int del) {
  tft.fillRect(87, 12, 9, 8, 0xFFFF);
  delay(del);
  tft.fillRect(87, 12, 9, 8, 0x0000);
  delay(del);
}

//Инициализируем дисплей и анемометр для дальнейшей работы
void setup() {
  Wire.begin(); 
  cgAnem.init();
  tft.init();
  tft.setRotation(0); // Задаём книжную ориентацию дисплея
  tft.fillScreen(TFT_BLACK); // Чистим дисплей
  tft.setTextColor( TFT_WHITE, TFT_BLACK); // Задаём цвет шрифта и фона
  cgAnem.set_duct_area(100); // Задаём площадь поперечного сечения для расчёта расхода. Меняется программно, измеряется в см^2
  
  // Функция таймера служит для предварительного нагрева анемометра перед использованием
  for(int i = 10; i > 0; i--){
    tft.setTextSize(3);
    tft.setCursor(50, 110);
    tft.print(i);
    delay(1000);
    tft.fillScreen(TFT_BLACK);
  }
  
  tft.setTextSize(1); //Задаём размер шрифта 
}
void loop() {
  //Каждую секунду происходит опрос анемометра и вывод новых параметров
  if (millis() - timer_meas > 1000) { 
    timer_meas = millis();
    char buf1[20];
    char buf2[20];
    sprintf(buf1, "V: %.1f ", cgAnem.getAirflowRate()); // Создаём буферы для более удобного выведения на экран
    sprintf(buf2, "T: %.1f ", cgAnem.getTemperature());
    // Проверяем, обновляются ли данные с анемометра. Если да - выводим их, если нет - предупреждаем об ошибке
    if (cgAnem.data_update()) {
      tft.drawString(buf1, 10, 50, 2);
      tft.drawString("m/s", 60, 50, 2);
      tft.drawString(buf2, 10, 108, 2);
      tft.drawString("C", 60, 108, 2);
      tft.drawString("Consumption:", 10, 158, 2); // Расход не получилось уместить в одну строчку, поэтому его мы выводим отдельно
      tft.drawString(String(cgAnem.calculateAirConsumption()), 10, 185, 2);
      tft.drawString("m3/h", 60, 185, 2);
    }
    else {
      tft.drawString("ERROR" , 0, 150, 2);
    }
  }
  //Опрашиваем АЦП каждые 5 секунд и при изменении показателей изменяем текущий уровень заряда
  if (millis() - timer_bat > 5000) {
    timer_bat = millis();
    ADC = analogRead(ADC_pin);
    tft.drawRect(85, 10, 40, 12, 0xFFFF);
    tft.fillRect(125, 13, 4, 6, 0xFFFF);
    if (ADC >= 1600) {
      tft.drawString("100%", 5, 9, 2);
      tft.fillRect(87, 12, 36, 8, 0xFFFF);
    }
    if (ADC <= 1600 && ADC >= 1500) {
      tft.drawString("75%", 5, 9, 2);
      tft.fillRect(87, 12, 27, 8, 0xFFFF);
    }
    if (ADC <= 1500 && ADC >= 1450) {
      tft.drawString("25%", 5, 9, 2);
      tft.fillRect(87, 12, 9, 8, 0xFFFF);
    }
    if (ADC <= 1450 ) {
      tft.setTextColor(TFT_RED);
      tft.drawString("LOW", 5, 9, 2);
      lowbat(1000);
    }
  }
}
