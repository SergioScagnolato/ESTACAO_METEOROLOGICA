
/***** 22/08/2019
 *  17/09/2019 - adicionado medidor de sinal wifi
 *  28/10/2019 - opcao de desligar os leds indicativos
 *  15/12/2019 - sensor de temperatura
 *  16/12/2019 - ldr na porta digital
 *  23/11/2019 - botoes redondos
 *  03/01/2020 - conexao a internet e log de temperatura e umidade
 *  09/01/2020 - log do sensor de luz do esp-master
 *  06/02/2020 - acende a luz do portao junto com a da garagem pelo horario NTP
 *  15/02/2020 - primeira consulta ao NTP 5 minutos apos ligar, as proximas de 20 em 20 min
 *  24/03/2020 - loga nivel da caixa de agua
 *  26/03/2020 - ajuste da equacao da reta da leitura da caixa de agua 
 *  29/03/2020 - 2500 leituras da porta analogica
 *  29/03/2020 - fix bug ntp time update na funcao GRAVA_CS
 *  
 *  29/03/2020 - led de 'alive' para verificar travamento do esp
 *  08/04/2020 - acender as lampadas as 18:30 hr
 *  09/04/2020 - WATCHDOG
 *  20/11/2020 - MUDANCA DE HORARIO DE APAGAR LAMPADAS PARA 6:00 HRS
 *  01/12/2020 - ACENDER TAMBEM HOLOFOTE DO FUNDO JUNTO COM A GARAGEM (TROCADO HOLOFOTE POR UMA LAMPADA)
 *  01/01/2021 - ATUALIZADO LOG 2021
 *  01/01/2021 - HORARIO DE ACENDIMENTO DA LAMPADA AS 19 HR - LINHA 562
 *  01/01/2021 - ATUALIZADO LIB DA DHT
 *  09/04/2021 - HORARIO DE ACENDIMENTO DA LAMPADA AS 18:20 HR - LINHA 566
 *  05/05/2021 - TROCA DO SENSOR DE TEMPERATURA E UMIDADE PARA UM DHT-11
 *  24/08/2021 - MUDANÇA DO  PROTOCOLO PARA HTTPS PARA O LOG NA NUVEM
 *  27/09/2021 - mudanca do horario de acendimento para 18:30 - linha 644
 *  03/09/2021 - mudanca no algoritmo de acesso ao NTP
 *  13/09/2021 - Mudanca para wifi Sscagnolato 
 *  22/09/2021 - APAGAR LAMPADA AS 6 - LINHA 647 
 *  10/10/2021 - APAGAR LAMPADA AS 5 - LINHA 647
 *  06/11/2021 - ACENDER ILUMINACAO ALTERNATIVAMENTE PELA LUMINOSIDADE (130 ACENDE, 800 APAGA) (SENSOR LDR NO ESP MASTER - PORTAO) - LINHA 760
 *  10/11/2021 - ACENDER AS 19 APAGAR AS 5
 *  01/01/2022 - AJUSTE DO ANO
 *  02/01/2022 - ACENDER AS 19:20 E APAGAR AS 6  - LINHA 617
 *  09/03/2022 - ACENDER AS 19:00 E APAGAR AS 6  - LINHA 619
 *  18/03/2022 - ACENDER AS 18:40 E APAGAR AS 6  - LINHA 620
 *  13/04/2022 - ACENDER AS 18:20 E APAGAR AS 6  - LINHA 621
  */


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include "DHT.h"
//#include <TimeLib.h>
#include <NTPClient.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
//#include <HTTPClient.h>

const byte DEBUG=0;

/****************************************/
const char* ssid     = "PORTAO";                     //VARIÁVEL QUE ARMAZENA O NOME DA REDE SEM FIO EM QUE VAI CONECTAR
const char* password = "teresa3-2";              //VARIÁVEL QUE ARMAZENA A SENHA DA REDE SEM FIO EM QUE VAI CONECTAR
/******************************************/
//DEFINIÇÃO DE IP FIXO PARA O NODEMCU
IPAddress ip(192,168,4,100); 
IPAddress gateway(192,168,4,1); 
IPAddress subnet(255,255,255,0);


/******************************************/
//const char *ssid_internet      = "BUDA2";
const char *ssid_internet      = "Sscagnolato";
const char *password_internet  = "saicapeta666";
/*******************************************/
IPAddress ST_IP(192,168,1,96); 
IPAddress ST_gateway(192,168,1,1); 
IPAddress ST_subnet(255,255,255,0);

 
//WiFiServer servidor(80); //CASO OCORRA PROBLEMAS COM A PORTA 80, UTILIZE OUTRA (EX:8082,8089) E A CHAMADA DA URL FICARÁ IP:PORTA(EX: 192.168.0.15:8082)
#define LED_WIFI D5
#define LED_ACESSO D6

#define RELE_LUZGARAGEM     D1
#define RELE_HOLOFOTECASA   D2
#define RELE_HOLOFOTEFUNDO  D3
#define RELE_RESERVA        D7
#define SENSOR_LDR          7
#define ANO_LOG             2022


// TIPO DO SENSOR DE TEMPERATURA
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define CAPACIDADE_CAIXA_AGUA 500.0

// DHT Sensor
uint8_t SENSOR_TEMPERATURA = D4;     // GPIO DO SENSOR DE TEMPERATURA


#define MODOWIFI WIFI_PHY_MODE_11B     // maior alcance, 11 Mbps  - celular encontra na pesquisa de redes wifi
//#define MODOWIFI WIFI_PHY_MODE_11G   // menor alcance, 54 Mbps - celular nao encontra na pesquisa de wifi, so o notebook
//#define MODOWIFI WIFI_PHY_MODE_11N   

WiFiServer servidor(80); 
//ESP8266WebServer servidor(80);
bool ligado_garagem=false;
bool ligado_holofotecasa=false;
bool ligado_holofotefundo=false;
int luz=0;

String servidor1="192.168.4.1";             // ip do ESP servidor
String host_servidor1="Host: "+servidor1;

String autolamp="on";

// Variable to store the HTTP request
String header;

long previousMillis = 0;        // Variável de controle do tempo
long redLedInterval = 150;      // Tempo em ms do intervalo a ser executado
long aliveInterval = 90;       // led de 'alive'
bool redLedState=LOW;
bool estadopisca=HIGH;
int piscadas=0;
long previous_leitura_sensor_luz=0;
long tempo_leitura_sensor_luz=10000;  // 1 minuto +- contando com o delay do loop
int leds_enable=1;
int conexao=3;                // 0 = conectao ao ESP, 1 = conectado internet, 3 = nao conectado

String valorsensordeluz="ND";            // valor do sensor do luz do ESP-MASTER
String valormac,mac="ND";                  // valor do mac que solicitou abertuda do portao


// INICIALIZA SENSOR DE TEMPERATURA.
DHT dht(SENSOR_TEMPERATURA, DHTTYPE);

// NTP
String formattedDate;
String dayStamp;
String timeStamp;
const long utcOffsetInSeconds = -10800;      // fuso horario gmt-3 (gmt-1 = -3600)
String site_csv="scagnolato.com.br";             // site onde grava o csv - ha um script la que monta o csv
String host_site_csv="Host: "+site_csv;
long previous_grava_csv=0;
long previous_ler_sensor_luz=0;
byte primeiro_acesso=1;            // ver linha 463
long tempo_grava_csv=5* 60 *1000;  // grava dados no csv a cada 5 minutos no statup, depois a cada 10 min (ver linha 525)
int horas,minutos;               // acende a lampada a partir das 19 hr a apaga as 6 hr, se nao houver conexao com o master do portao

// Cliente NTP 
//WiFiUDP ntpUDP;
// ****************** pool de ntp a conectar

 const char *ip_servidor_ntp="200.186.125.195";

//NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utcOffsetInSeconds);
//NTPClient timeClient(ntpUDP, "200.160.7.186", utcOffsetInSeconds);         // a.st1.ntp.bt
//NTPClient timeClient(ntpUDP, "201.49.148.135", utcOffsetInSeconds);          // b.st1.ntp.br   --> trava ?
//NTPClient timeClient(ntpUDP, "200.186.125.195", utcOffsetInSeconds);       // c.st1.ntp.br
//NTPClient timeClient(ntpUDP, "200.20.186.76", utcOffsetInSeconds);         // d.st1.ntp.br
//NTPClient timeClient(ntpUDP, "200.160.0.8", utcOffsetInSeconds);           // a.ntp.br
//NTPClient timeClient(ntpUDP, "200.189.40.8", utcOffsetInSeconds);          // b.ntp.br
//NTPClient timeClient(ntpUDP, "200.192.232.8", utcOffsetInSeconds);         // c.ntp.br



/********OTA *********/
const char* host_ota = "ESP_slave";   //Nome que o NodeMCU tera na rede OTA

/****************** PAGINA PRINCIPAL **********************/
   String PAGINA_PRINCIPAL="<!DOCTYPE html><html>\n"
          "<head><meta name=viewport content=width=device-width, initial-scale=1>"
          "<style>\n"
          "html {"
          "font-family: Sans-serif;"
          "display: inline-block;"
          "margin: 0px auto;"
          "text-align: center;}\n"
          ".switch {\n"
          "position: relative;\n"
          "display: inline-block;\n"
          "width: 60px;\n"
          "height: 34px;\n"
          "}\n"
          ".switch input {\n"
          "opacity: 0;\n"
          "width: 0;\n"
          "height: 0;\n"
          "}\n"
          ".slider {\n"
          "position: absolute;\n"
          "cursor: pointer;\n"
          "top: 0;\n"
          "left: 0;\n"
          "right: 0;\n"
          "bottom: 0;\n"
          "background-color: #ccc;\n"
          "-webkit-transition: .4s;\n"
          "transition: .4s;\n"
          "}\n"
          ".slider:before {\n"
          "position: absolute;\n"
          "content: \"\";\n"
          "height: 26px;\n"
          "width: 26px;\n"
          "left: 4px;\n"
          "bottom: 4px;\n"
          "background-color: white;\n"
          "-webkit-transition: .4s;\n"
          "transition: .4s;\n"
          "}\n"
          "input:checked + .slider {\n"
          "background-color: #2196F3;\n"
          "}\n"

          "input:focus + .slider {\n"
          "box-shadow: 0 0 1px #2196F3;\n"
          "}\n"

          "input:checked + .slider:before {\n"
          "-webkit-transform: translateX(26px);\n"
          "-ms-transform: translateX(26px);\n"
          "transform: translateX(26px);\n"
          "}\n"
          ".slider.round {\n"
          "border-radius: 34px;\n"
          "}\n"
          ".slider.round:before {\n"
          "border-radius: 50%;\n"
          "}\n"
/*
          PAGINA_PRINCIPAL+=".button {";
          PAGINA_PRINCIPAL+="background-color: #195B6A;";
          PAGINA_PRINCIPAL+="border: none;";
          PAGINA_PRINCIPAL+="color: white;";
          PAGINA_PRINCIPAL+="padding: 16px 30px;";
          PAGINA_PRINCIPAL+="text-decoration: none;";
          PAGINA_PRINCIPAL+="font-size: 14px;";
          PAGINA_PRINCIPAL+="margin: 2px;";
          PAGINA_PRINCIPAL+="cursor: pointer;}";

          PAGINA_PRINCIPAL+=".button:active {";
          PAGINA_PRINCIPAL+=" background-color: #3e8e41; ";
          PAGINA_PRINCIPAL+=" box-shadow: 0 5px #666; ";
          PAGINA_PRINCIPAL+=" transform: translateY(4px);";
          PAGINA_PRINCIPAL+="}";
  */       
          ".blockcaixa { \n"
          "margin: 0px auto;\n"
          "overflow: hidden;\n"
          "background-color:blue;\n"
          "border-radius:20px;\n"
          "width: 310px;\n"
          "height:100px;}\n"
          "</style>\n</head>\n<body>\n";
//          PAGINA_PRINCIPAL+="<h3>Versao CLIENTE - 2.0</h3>";


/*
// certificado do site scagnolato.com.br
const char* ca_cert= \
"-----BEGIN CERTIFICATE-----\n"\
"MIIFPjCCBCagAwIBAgISBODMjOI605FEnWhyvT9yhU2eMA0GCSqGSIb3DQEBCwUA\n"\
"MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD\n"\
"EwJSMzAeFw0yMTA4MjQyMDM1MDlaFw0yMTExMjIyMDM1MDhaMBwxGjAYBgNVBAMT\n"\
"EXNjYWdub2xhdG8uY29tLmJyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"\
"AQEAsQ5Driw2cvibLKGQbnqJxT0uOFoIdjcBk+tH8HsMxhmlXpet28K89HYUk+09\n"\
"WrASBf2zzWo3P4GSZRCRTX+kf4juMPh4Pe3aQANGdBhyv+Xy9HfDVTpPe4lo/Nq7\n"\
"76MuhTfNgvT2Xl8xlYMUMU8jtFWrDnctkH/At8PCqRu3uZV/jm3QcREImMUShrj4\n"\
"82srhOGOCrpxE5YBpG1ZrelXvzUKbAAlcUfWH2WLTZM7Kyz47gjOwgViiTmASo5a\n"\
"WcmwX1C+Ou15yhZHOTn/nybjyET92cwEQpkoqvOeBnLhU+IlAfJ26VE67/enb3fu\n"\
"lGtDCpb3ft139kJffU1tmRSfOwIDAQABo4ICYjCCAl4wDgYDVR0PAQH/BAQDAgWg\n"\
"MB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAMBgNVHRMBAf8EAjAAMB0G\n"\
"A1UdDgQWBBRCNgfqKIsaQq+HmyRIeYNZswfBgDAfBgNVHSMEGDAWgBQULrMXt1hW\n"\
"y65QCUDmH6+dixTCxjBVBggrBgEFBQcBAQRJMEcwIQYIKwYBBQUHMAGGFWh0dHA6\n"\
"Ly9yMy5vLmxlbmNyLm9yZzAiBggrBgEFBQcwAoYWaHR0cDovL3IzLmkubGVuY3Iu\n"\
"b3JnLzAxBgNVHREEKjAoghMqLnNjYWdub2xhdG8uY29tLmJyghFzY2Fnbm9sYXRv\n"\
"LmNvbS5icjBMBgNVHSAERTBDMAgGBmeBDAECATA3BgsrBgEEAYLfEwEBATAoMCYG\n"\
"CCsGAQUFBwIBFhpodHRwOi8vY3BzLmxldHNlbmNyeXB0Lm9yZzCCAQUGCisGAQQB\n"\
"1nkCBAIEgfYEgfMA8QB2AESUZS6w7s6vxEAH2Kj+KMDa5oK+2MsxtT/TM5a1toGo\n"\
"AAABe3oWpioAAAQDAEcwRQIgW3S1QpmDk+RCXW5pwfkwS2AvcrUWy4lzMjrCN20r\n"\
"bwYCIQD9IXLr3eaGr/Ij6GeQii30wa5AjnvNbxZTuDgtru5aXAB3APZclC/RdzAi\n"\
"FFQYCDCUVo7jTRMZM7/fDC8gC8xO8WTjAAABe3oWpgQAAAQDAEgwRgIhAKtJT7VN\n"\
"G9PHyXGiq8JZj2utqZtUfQ5UhIgbS2jOJOLdAiEAgh4hzLe2tJOtn4UV7es0g+ou\n"\
"FwgzRX0xlG5hR2mBW2UwDQYJKoZIhvcNAQELBQADggEBAEb1ViTX7L4LYKq5IsNM\n"\
"UvDZ5n1l24iHb2r/cv0QQYYebTH3GrDsVpglAksku+zR9OpII7N9ax580gKNxuyi\n"\
"67aPs2ArzKcHrlF2QauXp/BqF7rHDlf1/FLRp99U9qN1cEUEiQcbXnSQ++GdxYqy\n"\
"+4y1ImVvezUteThsSkTeRFc+aSeGoZAe1an+Gu8UxiL5rpSm9Rb1E1r1+wYF+opz\n"\
"0haiLXTlj5GYTyuFHJGFn4rb1yha8jv967YPkmE/nMiRKtn3wmddzakSviBMXWKF\n"\
"rg0+hpHMAeSfjU6yitMKZRakj1iV1nKXbS1UPms27IyEMdnQFzEfgRwGwVIIz3zy\n"\
"LKU=\n"\
"-----END CERTIFICATE-----\n";
*/





/******************** SETUP *******************/
void setup() {
if(DEBUG==1)  {
 Serial.begin(9600);
 Serial.println("DEBUG LIGADO");
}
WiFi.persistent(false);
pinMode(LED_WIFI,OUTPUT);
pinMode(LED_ACESSO,OUTPUT);
pinMode(RELE_LUZGARAGEM,OUTPUT);
pinMode(RELE_HOLOFOTECASA,OUTPUT);
pinMode(RELE_HOLOFOTEFUNDO,OUTPUT);
pinMode(RELE_RESERVA,OUTPUT);
pinMode(SENSOR_TEMPERATURA,INPUT);

//pinMode(SENSOR_LDR,INPUT_PULLUP);

digitalWrite(LED_WIFI,LOW);
digitalWrite(LED_ACESSO,LOW);
digitalWrite(RELE_LUZGARAGEM,HIGH);
digitalWrite(RELE_HOLOFOTECASA,HIGH);
digitalWrite(RELE_HOLOFOTEFUNDO,HIGH);
digitalWrite(RELE_RESERVA,HIGH);

// INICIALIZA SENSOR DE TEMPERATURA.
dht.begin();

WiFi.setPhyMode(MODOWIFI);
WiFi.mode(WIFI_STA);      // --> necessario para o web server do cliente 
if(DEBUG==1) { 
 Serial.println(""); 
 Serial.println(""); 
 Serial.print("Conectando a "); 
 Serial.print(ssid); 
} 
WiFi.begin(ssid, password); 
WiFi.config(ip, gateway, subnet, gateway);

int vezes=10;
while (WiFi.status() != WL_CONNECTED and vezes>0) { 
 digitalWrite(LED_WIFI,HIGH);
 delay(300); 
 digitalWrite(LED_WIFI,LOW);
 delay(300);
 vezes--;
}
if(WiFi.status() == WL_CONNECTED) conexao=0;        // conectado ao ESP MASTER
digitalWrite(LED_WIFI,LOW);

if(DEBUG==1) {
 Serial.println(""); 
 Serial.print("Conectado a rede sem fio "); 
 Serial.println(ssid); 
 
 Serial.print("IP para se conectar ao NodeMCU: "); 
 Serial.println(WiFi.localIP()); 
}

servidor.begin(); 
if(DEBUG==1) Serial.println("Servidor iniciado"); 

// CHECA LUZ DA GARAGEM E SINCRONIZA
if(conexao==0) sincroniza_garagem();

//timeClient.begin();


 /************** atualizador OTA *****************/
  ArduinoOTA.setPort(8266);     // Porta
  ArduinoOTA.setHostname(host_ota);  // hostname
  ArduinoOTA.begin(); 

/**** WATCHDOG *****/
ESP.wdtEnable(8000);
}


/******************************************/
/*          GRAVA O CSV                   */
/******************************************/
int grava_csv() {
  digitalWrite(LED_ACESSO,HIGH);
  digitalWrite(LED_WIFI,HIGH);
  // Cliente NTP 
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, ip_servidor_ntp, utcOffsetInSeconds);       // c.st1.ntp.br
  timeClient.begin();

  int anos,vezes1=10,vezes=10;
  do {
    while(!timeClient.update() and vezes>0) {
     vezes--;
     digitalWrite(LED_ACESSO,HIGH);
     delay(1000);
     digitalWrite(LED_ACESSO,LOW);
     delay(1000);
    }   
   if(vezes==0) vezes1=0; 
   formattedDate = timeClient.getFormattedDate();
   int splitT = formattedDate.indexOf("T");
   dayStamp = formattedDate.substring(0, splitT);
   timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
   String horario=timeStamp.substring(0,2);
   String minuto =timeStamp.substring(3,5);
   String ano=dayStamp.substring(0,4);
   horas=horario.toInt(); // horario para acender a lampada da garagem
   minutos=minuto.toInt();
   anos=ano.toInt();
   vezes1--;
   delay(10);
  } while(anos!=ANO_LOG and vezes1>0);

//  timeClient.stop();
  int retorno=vezes;
  if(vezes>0) {   // conseguiu ler o NTP
    // ********  le sensor de temperatura
   float humidade,temperatura=0;
   for(vezes=0;vezes<20;vezes++) {
     humidade += dht.readHumidity();
     temperatura += dht.readTemperature();
     delay(6);
   }
   humidade=humidade/20;
   temperatura=temperatura/20;
   char buff_temp[12],buff_humi[12],buff_litros[12];
   sprintf(buff_humi,"%03.1f",humidade);
   sprintf(buff_temp,"%03.1f",temperatura);

   // **** sensor caixa de agua *******
   float litros;
   litros=round(calcula_caixa_agua());
   sprintf(buff_litros,"%03.1f",litros);

//  HTTPClient http;
//  String conexao_log="https://www.scagnolato.com.br/estacao/grava_csv.php?data="+dayStamp+"&hora="+timeStamp+"&t="+buff_temp+"&h="+buff_humi+"&luz="+valorsensordeluz+"&litros="+buff_litros+"&primeiro="+primeiro_acesso+"&auth=1";            // parametros ao URL
//  http.begin(conexao_log, ca_cert);         //Specify the URL and certificate
//  delay(2000);
//  http.end();  

   String GETStr="GET ";
   WiFiClientSecure client_csv;    // para conectar via https
   client_csv.setInsecure();           // sem usar certificado
   vezes=10;
   do {
    if (client_csv.connect(site_csv, 443)) {           // conecta ao site para fazer o log 
     
     if(DEBUG==1) 
      Serial.println("conectado ao servidor scagnolato.com.br");
     if(anos!=ANO_LOG) {dayStamp=timeStamp="ND"; horas=-1;}
     GETStr+="/estacao/grava_csv.php?data="+dayStamp+"&hora="+timeStamp+"&t="+buff_temp+"&h="+buff_humi+"&luz="+valorsensordeluz+"&litros="+buff_litros+"&primeiro="+primeiro_acesso+"&auth=1";           // parametros ao URL
     GETStr+=" HTTP/1.0";
     client_csv.println(GETStr); 
     if(DEBUG==1) 
      Serial.println(GETStr);
     client_csv.println(host_site_csv);
     if(DEBUG==1)
      Serial.println(site_csv);
     client_csv.println("User-Agent: SERGIO");
     client_csv.println();
     vezes=0;
    }
    client_csv.stop();
    vezes--;
    delay(500);
   } while(vezes>0);

  } else {                // nao foi possivel ler o NTP, verifica o sensor de luz para gerrencias as lampadas
//  WiFi.disconnect(); 
  // solicita leitura do sensor de luz do esp-master
  conecta_master();
  lersensordeluz();
  if(luz <= 130) {     // acende pela luminosidade
   ligado_garagem = 1;
   ligado_holofotefundo=1;
   digitalWrite(RELE_LUZGARAGEM,LOW);
   digitalWrite(RELE_HOLOFOTEFUNDO,LOW);  
   sincroniza_com_portao();
  }
  if(luz >= 800) {          // apaga pela luminosidade
   ligado_garagem = 0;
   ligado_holofotefundo=0;
   digitalWrite(RELE_LUZGARAGEM,HIGH);
   digitalWrite(RELE_HOLOFOTEFUNDO,HIGH);                  
   sincroniza_com_portao();
  }
 }
  digitalWrite(LED_ACESSO,LOW);
  digitalWrite(LED_WIFI,LOW);
  return retorno;
}



/*****************************/
/****** LOGA O PORTAO ********/
/*****************************/
void loga_portao() {
 WiFi.disconnect();
 delay(10);
 conexao=3;
 int anos,vezes=10,vezes1=10;
 WiFi.begin(ssid_internet, password_internet);
 WiFi.config(ST_IP, ST_gateway, ST_subnet,ST_gateway);  
 while ( WiFi.status() != WL_CONNECTED and vezes>0) {
  digitalWrite(LED_ACESSO,HIGH);
  digitalWrite(LED_WIFI,LOW);
  delay ( 300 );
  digitalWrite(LED_ACESSO,LOW);
  digitalWrite(LED_WIFI,HIGH);
  delay(300);
  vezes--;
 }
 if(WiFi.status() == WL_CONNECTED) {  
  digitalWrite(LED_ACESSO,HIGH);
  digitalWrite(LED_WIFI,HIGH);
  // Cliente NTP 
  WiFiUDP ntpUDP;  
  NTPClient timeClient(ntpUDP, ip_servidor_ntp, utcOffsetInSeconds);       // c.st1.ntp.br
  timeClient.begin();
  vezes=10;
  do {
   timeClient.update();
   while(!timeClient.update() and vezes>0) {
    timeClient.forceUpdate();
    vezes--;
    delay(500);
   }   
   vezes1=10;
   formattedDate = timeClient.getFormattedDate();
   int splitT = formattedDate.indexOf("T");
   dayStamp = formattedDate.substring(0, splitT);
   timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
   String ano=dayStamp.substring(0,4);
   anos=ano.toInt();
   vezes1--;
   delay(10);
  } while(anos!=ANO_LOG and vezes1>0);

 // timeClient.stop();


  if(anos==ANO_LOG) {
   
   String GETStr="GET ";
   WiFiClientSecure client_csv;                   // wificlientsecure para conectar via https
   client_csv.setInsecure(); 
   vezes=10;
   do {
    if (client_csv.connect(site_csv, 443)) {           // conecta ao site scagnolato.com.br
     if(DEBUG==1) 
      Serial.println("conectado ao servidor scagnolato.com.br");
     GETStr+="/estacao/grava_portao.php?data="+dayStamp+"&hora="+timeStamp+"&mac="+valormac+"&auth=1";           // parametros ao URL
     GETStr+=" HTTP/1.0";
     client_csv.println(GETStr); 
     if(DEBUG==1) 
      Serial.println(GETStr);
     client_csv.println(host_site_csv);
     if(DEBUG==1)
      Serial.println(site_csv);
     client_csv.println("User-Agent: SERGIO");
     client_csv.println();
     vezes=0;
    }
    client_csv.stop();
    vezes--;
    delay(500);
   } while(vezes>0);
  
  }
 }
 delay(10);
 conecta_master();
 
 return;
}

/************************************************/
/******************** LOOP **********************/
/************************************************/
void loop() {

//conecta ao NTP e gera o CSV
unsigned long current_grava_csv=millis();
int vezes;
horas=minutos=-1;

// ler o sensor de luz 1 minuto antes de gravar o log
if( current_grava_csv - previous_ler_sensor_luz > (tempo_grava_csv - 60000)) { 
 // faz o pedido para o sensor de luz do ESP-MASTER
  valorsensordeluz="ND";
  lersensordeluz();
  previous_ler_sensor_luz=millis();
}

 ArduinoOTA.handle();

// conecta periodicamente a internet
if( current_grava_csv - previous_grava_csv > tempo_grava_csv) { 
  WiFi.disconnect();
  delay(10);
 conexao=3;
 vezes=10;
 WiFi.begin(ssid_internet, password_internet);
 WiFi.config(ST_IP, ST_gateway, ST_subnet,ST_gateway);  
// if(WiFi.status() == WL_NO_SSID_AVAIL) vezes=0;
 while ( WiFi.status() != WL_CONNECTED and vezes>0) {
  digitalWrite(LED_ACESSO,HIGH);
  digitalWrite(LED_WIFI,LOW);
  delay ( 300 );
  digitalWrite(LED_ACESSO,LOW);
  digitalWrite(LED_WIFI,HIGH);
  delay(300);
  vezes--;
 }
 digitalWrite(LED_ACESSO,LOW);
 digitalWrite(LED_WIFI,LOW);
 if(WiFi.status() == WL_CONNECTED and vezes>0) {
  int gravou=grava_csv();
  if(gravou>0) {
   conexao=1;
   tempo_grava_csv=10* 60 *1000;  // grava dados no csv a cada 10 minutos
   primeiro_acesso=0;             // nao é mais o primeiro acesso
   if(horas>=0) {
    if(horas>=18 or horas<6) {
     if((horas==18 and minutos>=20) or horas>=19) {           // HORARIO DE ACENDIMENTO DAS LAMPADAS
      ligado_garagem = 1;
      ligado_holofotefundo=1;
      digitalWrite(RELE_LUZGARAGEM,LOW);
      digitalWrite(RELE_HOLOFOTEFUNDO,LOW);  
     }
    } else {
     ligado_garagem = 0;
     ligado_holofotefundo=0;
     digitalWrite(RELE_LUZGARAGEM,HIGH);
     digitalWrite(RELE_HOLOFOTEFUNDO,HIGH);
    }
   }
  }
   delay(5);
 }
 conecta_master();
 sincroniza_com_portao();
 previous_grava_csv = millis();
}

 
 WiFiClient client = servidor.available();   // Listen for incoming clients
   if (client) {                             // If a new client connects,
    if(DEBUG==1)
      Serial.println("Novo cliente");          // print a message out in the serial port
     String currentLine = "";                // make a String to hold incoming data from the client
     while (client.connected()) {            // loop while the client's connected
       if (client.available()) {             // if there's bytes to read from the client,
         digitalWrite(LED_ACESSO,HIGH);
         char c = client.read();    // read a byte, then
         if(DEBUG==1)
          Serial.write(c);                    // print it out the serial monitor
         header += c;
         if (c == '\n') {                    // if the byte is a newline character
           // if the current line is blank, you got two newline characters in a row.
           // that's the end of the client HTTP request, so send a response:
           if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
             
            if(header.indexOf("GET /garagem/on")>=0) {
              ligado_garagem = 1;
              digitalWrite(RELE_LUZGARAGEM,LOW);
            } else if(header.indexOf("GET /garagem/off")>=0) {
              ligado_garagem = 0;
              digitalWrite(RELE_LUZGARAGEM,HIGH);  

           } else if(header.indexOf("GET /holofotecasa/on")>=0) {
             ligado_holofotecasa=1;
             digitalWrite(RELE_HOLOFOTECASA,LOW);
           } else if(header.indexOf("GET /holofotecasa/off")>=0) {
             ligado_holofotecasa=0;
             digitalWrite(RELE_HOLOFOTECASA,HIGH);

           } else if(header.indexOf("GET /holofotefundo/on")>=0) {
             ligado_holofotefundo=1;
             digitalWrite(RELE_HOLOFOTEFUNDO,LOW);
           } else if(header.indexOf("GET /holofotefundo/off")>=0) {
             ligado_holofotefundo=0;
             digitalWrite(RELE_HOLOFOTEFUNDO,HIGH);

           } else if(header.indexOf("GET /ledsenable/on")>=0) {
             leds_enable=1;
           } else if(header.indexOf("GET /ledsenable/off")>=0) {
             leds_enable=0;
            
           } else if(header.indexOf("GET /autolamp/on")>=0) {
             autolamp="on";
           } else if(header.indexOf("GET /autolamp/off")>=0) {
             autolamp="off";
             
           } else if(header.indexOf("GET /sensordeluz")>=0) {
              String argumentos;
              valorsensordeluz="ND";
              int pi=header.indexOf("luz=");   // ponto inicial da strng para verficar se a variavel existe
              if(pi>0) {
               int pi=header.indexOf("=");   // ponto inicial da strng
               int pf=header.indexOf("HTTP");   // ponto final da string
               // string so com a linha de argumentos
               argumentos=header.substring(pi+1,pf-1);
               luz=argumentos.toInt();
               valorsensordeluz=argumentos;
              }

           } else if(header.indexOf("GET /abriuportao")>=0) {
              String argumentos;
              valormac="ND";
              int pi=header.indexOf("mac=");   // ponto inicial da strng para verficar se a variavel existe
              if(pi>0) {
              int pi=header.indexOf("=");   // ponto inicial da strng
               int pf=header.indexOf("HTTP");   // ponto final da string
               // string so com a linha de argumentos
               argumentos=header.substring(pi+1,pf-1);
               valormac=argumentos;
              }
              loga_portao();          // grava o CSV de log do portao
             }
           // Break out of the while loop
           break;
        } else { // if you got a newline, then clear currentLine
            currentLine = "";
        }
      } else if (c != '\r') {  // if you got anything else but a carriage return character,
         currentLine += c;      // add it to the end of the currentLine
      }
    }
     ArduinoOTA.handle();
   }

   client.print(PAGINA_PRINCIPAL);// cabecalho da pagina principal
           client.print("<table border=0 width=75% align=center>");
           client.print("<p><tr><td>GARAGEM</td><td> ");
          if (!ligado_garagem) {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/garagem/on\"'>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>\n");
          } else  {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/garagem/off\"' checked>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>\n");
          }
          client.print("</p></td></tr>\n<tr><td><p>HOLOFOTE CASA</td><td> ");
          if (!ligado_holofotecasa) {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/holofotecasa/on\"'>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>");
          } else {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/holofotecasa/off\"' checked>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>");
          }
          client.print("</p></td></tr>\n<tr><td><p>HOLOFOTE FUNDO</td><td> ");
          if (!ligado_holofotefundo) {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/holofotefundo/on\"'>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>");
          } else {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/holofotefundo/off\"' checked>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>");
          }
          client.print("</td></tr>\n</p>");
          client.print("<p><tr><td>LEDS</td><td>");
          
          if (!leds_enable) {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/ledsenable/on\"'>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>");
          
          } else {
           client.print("\n<label class=switch>");
           client.print("\n<input type=checkbox onclick='window.location.href=\"/ledsenable/off\"' checked>");
           client.print("\n<span class='slider round'></span>");
           client.print("\n</label>");
          
          }
           client.print("</td></tr>\n");
          //DADOS TEMPRATURA E UMIDADE
          int cc;
          float humidade,temperatura;
           humidade = dht.readHumidity();
           temperatura = dht.readTemperature();
          char buff[12];
          sprintf(buff,"%5.1f",humidade);
          client.print("\n<tr><td colspan=2><p>Umidade : ");
          client.print(buff);
          sprintf(buff,"%5.1f",temperatura);
          client.print(" %   Temp.:");
          client.print(buff);
          client.print(" C </p></td></tr> ");
          client.print("\n</table>\n");  

          //leitura do sensor caixa de agua
          float litros;
          litros=round(calcula_caixa_agua());
          int capacidade=CAPACIDADE_CAIXA_AGUA;
          float nivel_porc=(litros*100/capacidade);
         client.print("\n<style>");
         if(litros<=100) {
          client.print( "\n.blockcaixa { \n");
          client.print("background-color:red;\n}");
         }
         client.print("\n.box2 {");
         client.print("\nmargin:");
         int topo= 100-nivel_porc;
         client.print(topo);
         client.print(" 0 0 0px;"); 
         client.print("\n background-color: gray;");
         client.print("\nwidth:310px;");
         client.print("\nheight:");
         client.print(topo);
         client.print("px;"); 
         client.print("\nborder-radius: 0px;}");
         client.print("\n</style>");

         
         client.print("\n<center><h4>NIVEL DA CAIXA</h4>");
         client.print("\n<div class=blockcaixa>");
         client.print("\n<div class=box2>");

         client.print("\n</div>");
         client.print("\n</div>");
         client.print("<br>");
         client.print(litros);
         client.print(" lts");
           client.println("\n<hr><br>Sinal ");
           String _RSSI = String(WiFi.RSSI());
           int sinal=round(((100+WiFi.RSSI()) * 100) / 60);
           String cor_sinal="green";
           if(sinal<=64) cor_sinal="yellow";
           if(sinal<=25) cor_sinal="gray";

           client.print("\n<style>\n");
           // estilo do indicador de potencia wifi
           client.print(".blocksignal { margin: 0px auto; overflow: hidden; background-color:lightgray; width: 100px; height:17px;}");
           client.print("\n.boxsignal { shape-margin: 0px; background-color: ");
           client.print(cor_sinal);
           client.print("; width: ");
           client.print(sinal);
           client.print("px;\n height: 17px; float: left; border-radius: 20px; margin: 0px; }");

           client.print("\n</style>\n");
           client.print(_RSSI);
           client.print(" dBm <BR>");
           client.print("\n<div class=blocksignal><div class=boxsignal></div></div><BR>");
           client.print("\n<br>\n</body>\n</html>");
   
   // Clear the header variable
   header = "";
   // Close the connection
   client.stop();
   digitalWrite(LED_ACESSO,LOW);
  }
 

  if(ligado_garagem==1 and leds_enable==1) {
   unsigned long currentMillis = millis();    //Tempo atual em ms
   if (currentMillis - previousMillis > redLedInterval) { 
    previousMillis = currentMillis;    // Salva o tempo atual
    redLedState= !redLedState;
    digitalWrite(LED_ACESSO, redLedState);
    piscadas++;
    if(piscadas>5) digitalWrite(LED_WIFI,HIGH);
    if(piscadas>15) {piscadas=0;digitalWrite(LED_WIFI,LOW);}
   }
  } else {
   unsigned long currentMillis = millis();    //Tempo atual em ms
   if (currentMillis - previousMillis > aliveInterval) { 
    digitalWrite(LED_WIFI,estadopisca); 
    estadopisca=!estadopisca;
    previousMillis = currentMillis;    // Salva o tempo atual
   } 
  }
  delay(10);  // delay para o webserver
  ESP.wdtFeed();    // RESETA O WATCHDOG
}


/************* FIM LOOP ******************/
/****************************************/


void sincroniza_garagem() {
  String GETStr="GET ";
 WiFiClient servidor;
 if (servidor.connect(servidor1, 80)) {    
  // pede sincronismo ao servidor
  GETStr+="/sincroniza_garagem";           // parametros ao URL
  GETStr+=" HTTP/1.0";
  servidor.println(GETStr); 
  servidor.println(host_servidor1);
  servidor.println("User-Agent: Sergio");
  servidor.println();
  servidor.stop();
 }
}

void lersensordeluz() {
  // conecta ao esp-server e pede a leitura do sensor de luz. O ESP-SERVER responde com o valor na url /sensordeluz?luz=x no esp-slave
  String GETStr="GET ";
 WiFiClient servidor;
 if (servidor.connect(servidor1, 80)) {    
  // pede sincronismo ao servidor
  GETStr+="/sensorluz";           // parametros ao URL
  GETStr+=" HTTP/1.0";
  servidor.println(GETStr); 
  servidor.println(host_servidor1);
  servidor.println("User-Agent: Sergio");
  servidor.println();
 }
 servidor.stop();
 return;
}



float calcula_caixa_agua() {
  int x=0;
  float leitura=0;
  for(x=0;x<250;x++) {
   leitura+=analogRead(A0);
   delay(1);
  }
  leitura=leitura/250.0;
                  
   //equacao da reta :
   //26/03
   // 350 => cheia
   // 367 =>  quase vazio (aviso)
   // 379 vazio
   // litros = (-189500 + (500*leitura)) / -29    <== equacao da reta
   
  float litro=round( (-189500 + (500*leitura)) / -30);
  if(litro < 0) litro=0;
  if(litro>CAPACIDADE_CAIXA_AGUA) litro=CAPACIDADE_CAIXA_AGUA;
  return(litro);
}




void sincroniza_com_portao() {
  // conecta ao ESP MASTER
 WiFi.begin(ssid, password);
 WiFi.config(ip, gateway, subnet, gateway);
 int vezes=15;
 while (WiFi.status() != WL_CONNECTED and vezes>0) { 
   digitalWrite(LED_ACESSO,LOW);
   digitalWrite(LED_WIFI,HIGH);
   delay(300); 
   digitalWrite(LED_WIFI,LOW);
   delay(300);
   vezes--;
 }
 if(WiFi.status() == WL_CONNECTED) conexao=0;
 String GETStr="GET ";
 WiFiClient servidor;
 if (servidor.connect(servidor1, 80)) {    
  // pede sincronismo ao servidor
  if(ligado_garagem==1)
   GETStr+="/luzportao/on";           // parametros ao URL
  else
   GETStr+="/luzportao/off";           // parametros ao URL
  GETStr+=" HTTP/1.0";
  servidor.println(GETStr); 
  servidor.println(host_servidor1);
  servidor.println("User-Agent: Sergio");
  servidor.println();
 }
 servidor.stop();
}



void conecta_master() {
  // conecta ao ESP MASTER
 WiFi.disconnect();
 WiFi.begin(ssid, password);
 WiFi.config(ip, gateway, subnet, gateway);
 int vezes=15;
 while (WiFi.status() != WL_CONNECTED and vezes>0) { 
   digitalWrite(LED_ACESSO,LOW);
   digitalWrite(LED_WIFI,HIGH);
   delay(300); 
   digitalWrite(LED_WIFI,LOW);
   delay(300);
   vezes--;
 }
 if(WiFi.status() == WL_CONNECTED) conexao=0;
 digitalWrite(LED_ACESSO,LOW);
 digitalWrite(LED_WIFI,LOW);
 return;
}
