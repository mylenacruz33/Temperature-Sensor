#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 12  // Pino onde o sensor DS18B20 está conectado

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress sensor1;  // Endereço do sensor

unsigned long delayTime;

void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);
  Serial.println();
  Serial.println(F("MEGA | DS18B20 inicializando..."));

  // Inicializa o sensor DS18B20
  sensors.begin();

  // Localiza e verifica o endereço do sensor
  if (!sensors.getAddress(sensor1, 0)) {
    Serial.println("MEGA | Erro: Sensor não encontrado!");
    while (1);
  } else {
    Serial.print("MEGA | Endereço do sensor: ");
    mostrarEnderecoSensor(sensor1);
  }

  Serial.println("MEGA | Mega transmitindo dados para ESP8266.");
  delayTime = 1000;

  Serial.println();
}

float temperature;

void loop() {
  while (Serial3.available()) {
    Serial.print((char)Serial3.read());
  }

  static long timer = millis() - 10000;
  if (millis() - timer < 5000) return;
  timer = millis();

  printDS18B20();
  Serial.print("MEGA | ");
  Serial3.print(temperature);
  Serial3.println(" K");
}

void printDS18B20() {
  sensors.requestTemperatures();  // Solicita a leitura da temperatura
  delay(200);  // Aguarda 200ms para garantir a leitura

  temperature = sensors.getTempC(sensor1);  // Lê a temperatura em Celsius
  temperature = temperature + 273.15;  // Converte para Kelvin

  if (temperature != DEVICE_DISCONNECTED_C) {
    Serial.print("MEGA | Temperatura = ");
    Serial.print(temperature);
    Serial.println(" K");
  } else {
    Serial.println("MEGA | Erro: Nenhuma leitura válida.");
  }

  Serial.println();
}

// Função auxiliar para mostrar o endereço do sensor
void mostrarEnderecoSensor(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}