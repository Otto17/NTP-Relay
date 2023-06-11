//********** ФУНКЦИЯ SETUP **********//


void setup() {
  //Объявление и настройка пинов
  pinMode(INDICATOR, OUTPUT);  // LED Индикатор на модуле ESP
  pinMode(SW_RELAY, OUTPUT);   // Пин управления (Вкл./Выкл. реле через оптопару)
  pinMode(BUT_RES, INPUT);     // Кнопка сброса настроек ESP

  digitalWrite(INDICATOR, HIGH);  // По умолчанию индикатор выключен (инверсия)
  digitalWrite(SW_RELAY, HIGH);   // Устанавливаем высокое значение PNP транзисторе управляющим оптопарой (выключен при HIGH)


  //Отладка
  // Serial.begin(115200);  // Инициализируем Serial
  // delay(2000);           // Что бы успеть открыть монитор порта для получения самых первых данных


  //Считываем структуру с данными WiFi из EEPROM
  EEPROM.begin(EEPROM_BEGIN);  // Инициализируем EEPROM
  EEPROM.get(0, portalCfg);    // Считываем структуру начиная с нулевой ячейки


  //Проверка EEPROM. При полном её стирании с завода или самостоятельно через Arduino IDE (в этих случаях ячейки заполнены значениями 255)
  byte Check_EEPROM;            // Сюда будем считывать данные первой ячейки из EEPROM (для проверки - есть ли какие данные в ней или нет)
  EEPROM.get(0, Check_EEPROM);  // Считываем нулевой адрес из EEPROM


  //Если нажата кнопка Reset при старте ESP ИЛИ переменная из структуры пустая (при сбросе настроек ESP мы заполнением EEPROM нулями) ИЛИ
  // EEPROM находится в заводском состоянии (при стирании всего Flash из Arduino IDE или просто новый чип), то память заполнена значениями 255, то...
  if (!digitalRead(BUT_RES) || String(portalCfg.ssid) == "" || Check_EEPROM == 255) portalRun(PORTAL_TIMEOUT);  // Запускаем портал настроек WiFi и авторизации на WEB с таймаутом "PORTAL_TIMEOUT"


  //Получение SSID и пароля WiFi с портала и сохранение их в EEPROM как структуру
  if (portalStatus() == SP_SUBMIT) {  // Если кнопка "Сохранить" с портала нажата, то получаем данные с формы через структуру
    EEPROM.begin(EEPROM_BEGIN);       // Инициализируем EEPROM
    EEPROM.put(0, portalCfg);         // Подготавливаем структуру к записи с нулевой ячейки (сохраняем её в ОЗУ)
    EEPROM.commit();                  // Записываем из ОЗУ в EEPROM
    EEPROM.end();                     // И очищаем данные из ОЗУ
    WiFi.softAPdisconnect();          // Отключаем режим AP
  }


  //Считываем из EEPROM время начала и окончания работы реле, заданное на WEB интерфейсе
  EEPROM.get(80, Start_str);  // Считываем с 70 ячейки EEPROM время начала работы реле (одним числом "int")
  EEPROM.get(100, End_str);   // Считываем с 80 ячейки EEPROM время окончания работы реле (одним числом "int")


  /*
  Перевод времени начала и окончания работы реле на WEB интерфейс в поля "GP.TIME()".
  Переводим из секунд (считанных из EEPROM) в часы, минуты и секунды и пишем их разные переменные для установки в тип GPtime "pool_TimeOn" и "pool_TimeOff".
  Так же используем длинное число секунд для удобной работы с ним при сравнении для работы реле.

ПРИМЕР (https://allcalc.ru/node/2072):
Например переведём 83540 секунд:
1. Разделим на 3600 и узнаем сколько часов - 83540 / 3600 = 23 (целых) часа.
2. Получим остаток 83540 - 23 × 3600 = 740 секунд и разделим на 60, получаем 12 (целых) минут.
3. Получим остаток 740 - 12 × 60 = 20 - это будут оставшиеся секунды.
Получаем в итоге 83540 секунд - это 23 часа, 12 минут и 20 секунд.
*/

  // Расчёт времени из длинных секунд для начала работы реле
  int hourOn_Load = Start_str / 3600;                 // Получаем часы из секунд считанных из EEPROM
  int Buf_On = Start_str - (hourOn_Load * 3600);      // Промежуточный буфер для получения минут и секунд
  int minuteOn_Load = Buf_On / 60;                    // Получаем минуты
  int secondOn_Load = Buf_On - (minuteOn_Load * 60);  // Получаем секунды

  GPtime Load_TimeOn(hourOn_Load, minuteOn_Load, secondOn_Load);  // Создаём временный локальный список типа "GPtime(int hour, int minute, int second)"
  pool_TimeOn = Load_TimeOn;                                      // Копируем локальный тип GPtime "Load_TimeOn" в глобальный тип GPtime "pool_TimeOn". После этого действия на WEB интерфейсе будет выведено сохранённое ранее время "С hh:mm:ss"


  // Расчёт времени из длинных секунд для окончания работы реле
  int hourOff_Load = End_str / 3600;                     // Получаем часы из секунд считанных из EEPROM
  int Buf_Off = End_str - (hourOff_Load * 3600);         // Промежуточный буфер для получения минут и секунд
  int minuteOff_Load = Buf_Off / 60;                     // Получаем минуты
  int secondOff_Load = Buf_Off - (minuteOff_Load * 60);  // Получаем секунды

  GPtime Load_TimeOff(hourOff_Load, minuteOff_Load, secondOff_Load);  // Создаём временный локальный список типа "GPtime(int hour, int minute, int second)"
  pool_TimeOff = Load_TimeOff;                                        // Копируем локальный тип GPtime "Load_TimeOff" в глобальный тип GPtime "pool_TimeOff". После этого действия на WEB интерфейсе будет выведено сохранённое ранее время "До hh:mm:ss"


  //Яндекс Погода
  offsetUTC = "Город - Not sync";  // Выводим на WEB интерфейс статическую надпись, пока не получим данные часового пояса от Яндекса
  pogoda = "Not sync";             // Выводим на WEB интерфейс статическую надпись, пока не получим данные температуры от Яндекса


  //ОТЛАДКА
  // Serial.print("SSID: ");
  // Serial.println(portalCfg.ssid);
  // Serial.print("Pass WiFi: ");
  // Serial.println(portalCfg.pass);


  //Подключаемся к WiFi
  // Serial.print("Connect to: ");
  // Serial.println(portalCfg.ssid);
  WiFi.mode(WIFI_STA);                         // Запускаем WiFi в режиме "Станция"
  WiFi.hostname(HOSTNAME);                     // Задаём имя хоста
  WiFi.begin(portalCfg.ssid, portalCfg.pass);  // Берём данные из структуры
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
  }
  // Serial.println();
  // Serial.print("Connected! Local IP: ");
  // Serial.println(WiFi.localIP());


  //Кнопка cброса настроек ESP
  ButRes.setHoldTimeout(SET_HOLD_TIMEOUT);  // Устанавливаем время удержания кнопки


  //NTP
  ntp.setHost(SET_HOST);            // Задаём свой сервер NTP
  ntp.setGMT(SET_GMT);              // Задаём часовой пояс (в часах)
  ntp.setPeriod(SET_PERIOD * 60U);  // Задаём период обновления "в минутах" (количество секунд = количество минут * 60), (U - unsigned, выделяем память для беззнаковой константы)
  ntp.begin();                      // Запускаем NTP


  //Функции GyverPortal
  ui.attachBuild(build);  // Подключаем конструктор UI и запускаем его
  ui.attach(action);      // Подключаем функцию с действиями от WEB интерфейса
  ui.start(M_DNS);        // Запустить портал с поддержкой mDNS
  ui.log.start(64);       // Передали размер буфера
  ui.enableOTA();         // Подключить OTA обновление (без пароля)
}
