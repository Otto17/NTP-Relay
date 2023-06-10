//********** ФУНКЦИЯ LOOP **********//


void loop() {
  rel_State = !digitalRead(SW_RELAY);  // Считываем состояние реле

  ntp.tick();     // Тикер для NTP, он синхронизирует время по своему таймеру
  ui.tick();      // Тикер WEB UI (для опроса и обработки всех функций библиотеки)
  ButRes.tick();  // Тикер для обработчика кнопки (нужен чтобы корректно считались все таймауты)

  ResetButton();    // Постоянно проверяем функцию обработчика кнопки сброса на нажатие, удержание и отпускание
  WorkRelay();      // Постоянно проверяем функцию обработчика реле
  yandex_Pogoda();  // Постоянно проверяем функцию обновления данных температуры и часового пояса с Яндекса

  //Проверка состояния реле
  if (rel_State == true) {  // Если флаг поднят...
    RelayState = StateOn;   // Индикатор реле горит, тогда показываем надпись ON рядом с состоянием реле
    ColorTitle = GP_GREEN;  // Индикатор реле горит, тогда надпись ON красим в зелёный
  }

  if (rel_State == false) {  // Если флаг опущен...
    RelayState = StateOff;   // Индикатор реле горит, тогда показываем надпись OFF рядом с состоянием реле
    ColorTitle = GP_RED;     // Индикатор реле горит, тогда надпись OFF красим в красный
  }



  //Получение Даты и времени с NTP
  NewDate = ntp.dateString();                 // Получаем строку даты формата "ДД.ММ.ГГГГ"
  NewTime = ntp.timeString();                 // Получаем строку времени формата "ЧЧ:ММ:СС"
  lastSyncNTP = ntp.getFullFormLastUpdate();  // Получаем строку последней успешной синхронизации в формате "ДД-ММ-ГГ ЧЧ:ММ:СС"


  //Если флаг поднят, то моргаем LED индикатором на модуле ESP раз в 40 мс (без задержки кода)
  if (flagRes == true) digitalWrite(INDICATOR, (millis() / 40) % 2);  // Позволяет визуально определить когда можно отпускать кнопку сброса настроек



  // static uint32_t tmr_test;  // Таймер для опроса датчика с заданной периодичностью
  // if (millis() - tmr_test >= 1000) {  // Опрос и обработка раз в 1 секунду
  //   tmr_test = millis();

  //   Serial.println("");
  //   Serial.print("Время начала: ");
  //   Serial.println(Start_str);
  //   Serial.print("Время окончания: ");
  //   Serial.println(End_str);
  //   Serial.print("Время NTP (сек): ");
  //   Serial.println(NTP_str);
  //   Serial.println("");
  // }
}