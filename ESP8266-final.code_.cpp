#include <ESP8266WiFi.h> // Biblioteca para conexão Wi-Fi
#include <ESPAsyncTCP.h> // Biblioteca para comunicação assíncrona via TCP
#include <ESPAsyncWebSrv.h> // Biblioteca para criação de servidor web assíncrono
#include <OneWire.h> // Biblioteca para comunicação com dispositivos OneWire
#include <DallasTemperature.h> // Biblioteca para trabalhar com sensores DS18B20

// Credenciais de acesso Wi-Fi
const char* ssid = "LMSwifi";
const char* password = "dylanisgod";

// Cria o objeto do servidor web na porta 80
AsyncWebServer server(80);

// Cria uma fonte de eventos em "/events"
AsyncEventSource events("/events");

// Variáveis de temporizador
unsigned long lastTime = 0; // Última vez que o temporizador foi acionado
unsigned long timerDelay = 30000;

// Configuração do sensor DS18B20
#define ONE_WIRE_BUS 12 // Pino onde o sensor está conectado
OneWire oneWire(ONE_WIRE_BUS); // Objeto para comunicação OneWire
DallasTemperature sensors(&oneWire); // Objeto para manipular o sensor DS18B20
DeviceAddress sensor1; // Endereço do sensor DS18B20

float temperature; // Variável para armazenar a temperatura lida

// Lê a temperatura do sensor
int getSensorReadings() {
  if (Serial.available() == 0) return 0;
  char text[100] = { 0 };
  if (Serial.readBytesUntil('\n', (byte*)text, 100)) {
    Serial.println(text);
    if (sscanf(text, "%f", &temperature) == 1) return 1;
  }
  return 0;
}

// Inicializa a conexão Wi-Fi
void initWiFi() {
  WiFi.mode(WIFI_STA); // Define o modo Wi-Fi como estação
  WiFi.begin(ssid, password); // Conecta ao Wi-Fi
  Serial.print("ESP | Conectando ao WiFi ...");
  while (WiFi.status() != WL_CONNECTED) { // Aguarda conexão
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.print("ESP | IP: ");
  Serial.println(WiFi.localIP()); // Exibe o IP obtido
}

// Função para processar placeholders no HTML
String processor(const String& var) {
  getSensorReadings(); // Atualiza a temperatura
  if (var == "TEMPERATURE") { // Substitui o placeholder pelo valor da temperatura
    return String(temperature);
  }
  return String();
}

// HTML para a página do web server
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=PT+Serif:ital,wght@0,400;0,700;1,400;1,700&display=swap" rel="stylesheet">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color:rgb(80, 122, 184); color: white; font-size: 1rem; display: flex; align-items: center; justify-content: center; padding: 10px; font-family: "PT Serif", serif; font-size: 48px;}
    .topnav img { position: absolute; left: 10px; width: 300px; height: auto; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <img src="https://portal.if.uff.br/wp-content/uploads/2020/08/cropped-logo_if_neg_transp.png" alt="logo" />
    <span>LMS WEB SERVER</span>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p><i class="fas fa-thermometer-half" style="color:#e91b1b;"></i> TEMPERATURA</p><p><span class="reading"><span id="temp">%TEMPERATURE%</span> K</span></p>
      </div>
      <div class="card">
        <p><i class="fas fa-angle-double-down" style="color:#e1e437;"></i> PRESSAO</p><p><span class="reading"><span id="pres">N/A</span> hPa</span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('pressure', function(e) {
  console.log("pressure", e.data);
  document.getElementById("pres").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

// Função principal de inicialização
void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial
  delay(2000);
  Serial.println("\n\nESP | ESP8266 inicializando...");
  initWiFi(); // Configura o Wi-Fi

  // Configurações do servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
  request->send_P(200, "text/html", index_html, processor); 
});

// Configura a fonte de eventos
events.onConnect([](AsyncEventSourceClient* client) {
  if (client->lastId()) { // Trata reconexões
    Serial.printf("ESP | Cliente reconectado! Último ID de mensagem recebido: %u\n", client->lastId());
  }
  client->send("Olá!", NULL, millis(), 10000); // Envia um evento inicial
});
server.addHandler(&events); // Adiciona o handler de eventos
server.begin(); // Inicia o servidor
}

// Loop principal
void loop() {
    if (getSensorReadings() == 0) return;
    Serial.printf("ESP | Temperatura = %.2f K \n", temperature);
    Serial.println();
    // Envia a temperatura via eventos
    events.send("ping", NULL, millis());
    events.send(String(temperature).c_str(), "temperature", millis());
}
