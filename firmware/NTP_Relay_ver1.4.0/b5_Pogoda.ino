//********** ОБРАБОТКА ПОГОДЫ С ЯНДЕКСА **********//


//Функция обновления данных температуры и часового пояса с Яндекса
void yandex_Pogoda() {
  if ((millis() - lastTime_YA) > TIMER_UPDATE || flag_YA == true) {  // Отправляем HTTP GET запрос на Яндекс раз в "TIMER_UPDATE" миллисекунд или если поднят флаг принулительного запроса

    String serverPath = "https://yandex.com/time/sync.json?geo=" + String(REGION_ID);  // Формируем строку: адрес сервера с кодом региона

    http.begin(client, serverPath);  // Запускаем HTTP клиент
    int httpCode = http.GET();       // Отправляем запрос, вызывая метод GET для объекта http.

    if (httpCode > 0) {  // Если значение больше 0, то это стандартный код HTTP. Если значение меньше 0, то это ошибка клиента, связанная с подключением

      String response = http.getString();                           // Получаем ответ сервера
      DeserializationError error = deserializeJson(doc, response);  // Десериализируем документ JSON

      // Проверяем успешность парсинга
      if (!error) {  // Если ошибок десериализации нет, то выводим

        JsonObject clocks_66 = doc["clocks"][String(REGION_ID)];
        const char* offsetString = clocks_66["offsetString"];                       // Извлекаем значение часового пояса из JSON
        offsetUTC = "г. " + String(GOROD_YA) + " (" + String(offsetString) + "):";  // Формируем строку (к примеру) формата "г. Омск (UTC+6:00):" и пишем её в глобальную переменную

        JsonObject clocks_66_weather = clocks_66["weather"];
        int weather_temp = clocks_66_weather["temp"];  // Извлекаем значение температуры из JSON
        pogoda = String(weather_temp) + " °C";         // Формируем строку (к примеру) формата "-3 °C" и пишем её в глобальную переменную
      }
    }

    http.end();              // Закрываем соединение HTTP
    flag_YA = false;         // Опускаем флаг
    lastTime_YA = millis();  // Обнуляем таймер
  }
}
