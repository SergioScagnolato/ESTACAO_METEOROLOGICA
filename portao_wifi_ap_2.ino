   /*
 * Versao AP - usando lib 2.5.0 - dhcp ok, ota nao ok
 * 30-04-19 - OTA -usar esp8266 lib versao 2.4.2 para OTA
 * 16-05-19 - OTA teste versao 2.5.1 - ota ok, nao funcioina dhcp server
 * 14-07-19 - OTA teste versao 2.5.2 - ota nao , dhcp ok - dificuldade do celular localizar rede
 * 10/09/2019 - CONEXAO COM OUTRO ESP8266 
 * 23/09/2019 - correcoes
 * 18/09/2019 - botao automatico disable apos acionamento
 * 19/10/2019 - indentifica MAC do cliente conectado : Notebook 30 14 4A 33 50 7E, celular CC 61 E5 6F F0 0A, carolina  AC 1F 74 0A 0C 34
 * 23/10/2019 - armazena os mac que conectaram
 * 13/12/2019 - delay na leitura do sensor de luz e extensao da antena
 * 09/01/2020 - informa valor do sensor de luz ao esp-slave
 * 06/02/2020 - ULR para acender a luz do portao
 * 12/02/2020 - correcao bug na lampda do portao e links de acender/apagar luz garagem
 * 14/02/2020 - loga abertura automatica
 * 09/04/2020 - WATCHDOG
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <WiFiUdp.h>
// #include "DHTesp.h"
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//SSID e Password do ESP
const char* ssid = "PORTAO";
const char* password = "teresa3-2";
const byte canal_wifi = 2;
const byte max_connection=4;
const char* host_ota = "ESP_portao_master";   //Nome que o NodeMCU tera na rede OTA

String cliente1="192.168.4.100";             // ip do ESP cliente
String host_cliente1="Host: "+cliente1;

#define MODOWIFI WIFI_PHY_MODE_11B     // maior alcance, 11 Mbps  - celular encontra na pesquisa de redes wifi
//#define MODOWIFI WIFI_PHY_MODE_11G   // menor alcance, 54 Mbps - celular nao encontra na pesquisa de wifi, so o notebook
//#define MODOWIFI WIFI_PHY_MODE_11N   

String MAC_CASA     = "30144A33507E";
String MAC_CELULAR  = "CC61E56FF00A";
String MAC_CAROLINA = "AC1F740A0C34";

//char *mac_conectado[20];
//int indice_mac_conectado=0;

//DHTesp dht;      // objeto sensor de temperatura e umidade DHT22

const byte DEBUG=0;                     // 0 NAO PRODUZ SAIDA SERIAL; 1 = PRODUZ SAIDA SERIAL
unsigned long previous_conexao=millis();
unsigned long previous_vivo=millis();
unsigned long current;
long atual_boot;
unsigned long atual_vivo;

long previous_leitura_sensor_luz=0;
long tempo_leitura_sensor_luz=10000;  // 1 minuto +- contando com o delay do loop

#define POTENCIA10 125
#define POTENCIA25 254
#define POTENCIA50 512
#define POTENCIA75 767
#define POTENCIA100 1023

String output5State = "off";
String output4State = "off";
String estadoLampada = "off";
String estadoLampadaGaragem = "off";

String autolamp = "off";
String autolamp_garagem = "off";

int potencia_lampada=0;  // 0=apagada  1023=maximo
int potencia_padrao=POTENCIA25;

const int RELE_PORTAO   = D2;
const int RELE_LAMPADA  = D1;
const int LED_VERDE     = D3;
const int LED_VERMELHO  = D7;
const int PORTA_LAMPADA = D6;         // pino para PWM na lampada
const int GPIO16       =  D0;
//const int SENSOR_TEMPERATURA = 2;

byte volatile aceso_portao,aceso_garagem=0;
int acender_luz=35;

long aliveInterval = 90;       // led de 'alive'
bool estadopisca=HIGH;
long previousMillis = 0;        // Variável de controle do tempo
long previousMillis_alive = 0;        // Variável de controle do tempo

//IPAddress local_IP(192,168,1,200);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);


ESP8266WebServer server(80); //Server on port 80

//==============================================================
//     PAGINA PRINCIPAL
//==============================================================
void handleRoot() {
 digitalWrite(LED_VERMELHO,HIGH);
 String MAIN_page="<!DOCTYPE html><html>";
 MAIN_page+="<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
 //MAIN_page+="<link rel=icon href=data:,>";
 MAIN_page+="<style>";
 MAIN_page+=" html { ";
 MAIN_page+="font-family: Sans-serif; ";
 MAIN_page+="    display: inline-block; ";
 MAIN_page+="margin: 0px auto; ";
 MAIN_page+="text-align: ";
 MAIN_page+="center;";
 MAIN_page+="   }";
 MAIN_page+=" .button {";
 MAIN_page+="    background-color: #195B6A; ";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 26px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".button:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+=" .buttonpequeno {";
 MAIN_page+="    background-color: #195B6A; ";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 14px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".buttonpequeno:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+=" .mais {";
 MAIN_page+="   background-color: #77878A;";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 18px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".mais:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+="</style></head><body>";
 MAIN_page+="\n\n<script>\n";
 MAIN_page+="function butaoff() {\n";
 MAIN_page+="document.getElementById('botaoabrir').disabled=true;\n";
 MAIN_page+="window.location.href='/5/on';\n";
 MAIN_page+="}\n";
 MAIN_page+="</script>\n\n";
// MAIN_page+="<h3>Versao AP - 2.0</h3>";

struct station_info *stat_info;
 struct ip4_addr *IPaddress;
 IPAddress address;
 stat_info = wifi_softap_get_station_info();
 IPaddress = &stat_info->ip;
 address = IPaddress->addr;
 char mac[14];
 String mac_numerofinal;
 sprintf(mac,"%02X%02X%02X%02X%02X%02X",stat_info->bssid[0],stat_info->bssid[1],stat_info->bssid[2],stat_info->bssid[3],stat_info->bssid[4],stat_info->bssid[5]);
 mac_numerofinal=mac;

/*
 //armazena o MAC
 // pesquisa
 bool tem_mac=false;
 int t;
 for(t=0;t<indice_mac_conectado;t++) {
  if(!strcmp(mac_conectado[t],mac)) tem_mac=true;
 }
 if(!tem_mac) {
  strcpy(mac_conectado[indice_mac_conectado],mac);
  indice_mac_conectado++;
  if(indice_mac_conectado>=20) indice_mac_conectado=0;
 }
*/
  if(mac_numerofinal==MAC_CASA or mac_numerofinal==MAC_CELULAR)
   MAIN_page+="<br><b><h3>Ola S&eacute;rgio</b></h3><br>";
  else if(mac_numerofinal ==  MAC_CAROLINA)
   MAIN_page+="<br><b><h3><FONT COLOR=RED>Ola minha pequena !</b></h3><FONT COLOR=BLACK><br>";
  else
   MAIN_page+="<br><b><h3>Ola !</b></h3><br>";
   
// MAIN_page+="<br>MAC ";
// MAIN_page+=mac_numerofinal;
 if(DEBUG==1) {
  Serial.println("**** MAC : ");
  Serial.print(mac_numerofinal);
  Serial.print(" ****");
  Serial.println("");
 }
    
 MAIN_page+="\n<p><button class=button id=botaoabrir onclick='butaoff()'>AUTOMATICO</button></p>";

 if(autolamp=="on") {
   MAIN_page+="\n<p><input type=checkbox  onclick='location.href=\"/7/off\"' checked> LAMPADA AUTOMATICA PORTAO</p>";
} else {
 if (estadoLampada=="off") {
   MAIN_page+="\n<p><a href=\"/6/on\"><button class=\"buttonpequeno\">ACENDER LAMPADA PORTAO</button></a></p>";
   MAIN_page+="\n<p><input type=checkbox  onclick='window.location.href=\"/7/on\"'> LAMPADA AUTOMATICA PORTAO</p>";
 } else {
   MAIN_page+="\n<p><a href=\"/6/off\"><button class=\"buttonpequeno\">APAGAR LAMPADA PORTAO</button></a></p>";
   MAIN_page+="\n<p><input type=checkbox  onclick='window.location.href=\"/7/on\"' > LAMPADA AUTOMATICA PORTAO</p>";
 }
}
 if(autolamp_garagem=="on") {
   MAIN_page+="\n<p><input type=checkbox  onclick='location.href=\"/8/off\"' checked> LAMPADA AUTOMATICA GARAGEM</p>";
} else {
 if (estadoLampadaGaragem=="off") {
   MAIN_page+="\n<p><a href=\"/9/on\"><button class=\"buttonpequeno\">ACENDER LAMPADA GARAGEM</button></a></p>";
   MAIN_page+="\n<p><input type=checkbox  onclick='window.location.href=\"/8/on\"'> LAMPADA AUTOMATICA GARAGEM</p>";
 } else {
   MAIN_page+="\n<p><a href=\"/9/off\"><button class=\"buttonpequeno\">APAGAR LAMPADA GARAGEM</button></a></p>";
   MAIN_page+="\n<p><input type=checkbox  onclick='window.location.href=\"/8/on\"' > LAMPADA AUTOMATICA GARAGEM</p>";
 }
}

 MAIN_page+= "\n<p><a href=\"/mais/on\"><button class=\"button mais\">Mais...</button></a></p>";


 MAIN_page+="\n<br>\n</body>\n</html>";
 server.send(200, "text/html", MAIN_page);
 digitalWrite(LED_VERMELHO,LOW);
}


/************************** MAIS ****************************/
void handleMais() {
 String MAIN_page="<!DOCTYPE html><html>";
 MAIN_page+="<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
 //MAIN_page+="<link rel=icon href=data:,>";
 MAIN_page+="<style>";
 MAIN_page+=" html { ";
 MAIN_page+="font-family: Sans-serif; ";
 MAIN_page+="    display: inline-block; ";
 MAIN_page+="margin: 0px auto; ";
 MAIN_page+="text-align: ";
 MAIN_page+="center;";
 MAIN_page+="   }";
 MAIN_page+=" .button {";
 MAIN_page+="    background-color: #195B6A; ";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 26px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".button:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+=" .button2 {";
 MAIN_page+="   background-color: #77878A;";
 MAIN_page+=" }";
 MAIN_page+=".button2:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 MAIN_page+=" .mais {";
 MAIN_page+="   background-color: #195b6a;";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 18px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".mais:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
  
 MAIN_page+="</style></head><body><h3>Versao AP - 2.0</h3>";

 MAIN_page+="<p><a href=\"/\"><button class=\"button mais\">VOLTAR</button></a></p>";
             
if (output4State=="off") 
  MAIN_page+= "<p><a href=\"/4/on\"><button class=\"button\">ABRIR MANUAL</button></a></p>";
else 
  MAIN_page+="<p><a href=\"/4/off\"><button class=\"button button2\">FECHAR MANUAL</button></a></p>";

if(autolamp=="on") {
   MAIN_page+="<p><input type=checkbox  onclick='location.href=\"/7/off\"' checked> LAMPADA AUTOMATICA</p>";
} else {
 if (estadoLampada=="off") {
   MAIN_page+="<p><a href=\"/6/on\"><button class=\"button\">ACENDER LAMPADA</button></a></p>";
   MAIN_page+="<p><input type=checkbox  onclick='window.location.href=\"/7/on\"'> LAMPADA AUTOMATICA</p>";
 } else {
   MAIN_page+="<p><a href=\"/6/off\"><button class=\"button button2\">APAGAR LAMPADA</button></a></p>";
   MAIN_page+="<p><input type=checkbox  onclick='window.location.href=\"/7/on\"' > LAMPADA AUTOMATICA</p>";
 }
}
MAIN_page+="<p>Potencia lampada</p>";
if(potencia_lampada==POTENCIA10)
 MAIN_page+="<p><input type=radio name=potencia value='125' onclick='window.location.href=\"/10/on\"' checked> 10%";
else
 MAIN_page+="<p><input type=radio name=potencia value='125' onclick='window.location.href=\"/10/on\"'> 10%";
if(potencia_lampada==POTENCIA25)
 MAIN_page+=" <input type=radio name=potencia value='254' onclick='window.location.href=\"/25/on\"' checked> 25%";
else
 MAIN_page+=" <input type=radio name=potencia value='254' onclick='window.location.href=\"/25/on\"'> 25%";
if(potencia_lampada==POTENCIA50)
 MAIN_page+=" <input type=radio name=potencia value='512' onclick='window.location.href=\"/50/on\"' checked> 50%";
else
 MAIN_page+=" <input type=radio name=potencia value='512' onclick='window.location.href=\"/50/on\"'> 50%";

if(potencia_lampada==POTENCIA75)
 MAIN_page+=" <input type=radio name=potencia value='767' onclick='window.location.href=\"/75/on\"' checked> 75%";
else
 MAIN_page+=" <input type=radio name=potencia value='767' onclick='window.location.href=\"/75/on\"'> 75%";

if(potencia_lampada==POTENCIA100)
 MAIN_page+=" <input type=radio name=potencia value='1023' onclick='window.location.href=\"/100/on\"' checked> 100%</p>";
else
 MAIN_page+=" <input type=radio name=potencia value='1023' onclick='window.location.href=\"/100/on\"'> 100%</p>";
            
 current=millis();

 MAIN_page+="<br>Sensor de luz ";
 int contador,leitura=0;
 for(contador=0;contador<2000;contador++) {
   leitura+=analogRead(A0);
//   delay(6);
 }
 leitura=leitura/2000;
 MAIN_page+=String((int)leitura);
 MAIN_page+=" ( ";
 MAIN_page+=String(acender_luz);
 MAIN_page+=" )<br>VIVO A ";

 atual_vivo = current - previous_vivo;
   int vivo=(atual_vivo/1000/60);
   String msg=" minutos";
   if(vivo>=60){
    vivo=vivo/60;
    msg=" hora";
    if(vivo>1) msg+="s";
    if(vivo>=24) {
     vivo=vivo/24;
      msg=" dia";
      if(vivo>1) msg+="s";
      if(vivo>=30) {
       vivo=vivo/30;
       msg=" mes";
       if(vivo>1) msg+="s";
      }
    }
   }
   MAIN_page+=String(vivo);
   MAIN_page+=msg;
 
 MAIN_page+="<br>";
 int tipowifi=WiFi.getPhyMode();
 msg="--";
 if(tipowifi==1) msg="812.11B";
 if(tipowifi==2) msg="812.11G";
 if(tipowifi==3) msg="812.11N"; 
 //MAIN_page+=String(WiFi.getPhyMode());
 MAIN_page+=msg;
 MAIN_page+=" Canal ";
 MAIN_page+=String(WiFi.channel());
 MAIN_page+="<br>";
 //MAIN_page+="<p><a href=\"/mais/tehu\"><button class=\"button mais\">Temperatura e Humidade</button></a></p>";
 MAIN_page+="<br></body></html>";
 server.send(200, "text/html", MAIN_page);
 digitalWrite(LED_VERMELHO,LOW);
}

/*
void handleMaistehu() {
 //DADOS TEMPRATURA E UMIDADE
  int cc;
  float humidade,temperatura;
  for(cc=0;cc<3;cc++) {
   delay(dht.getMinimumSamplingPeriod());
   humidade = dht.getHumidity();
   temperatura = dht.getTemperature();
  }  
  char buff[12];
  sprintf(buff,"%5.1f",humidade);

 String MAIN_page="<!DOCTYPE html><html>";
 MAIN_page+="<head><meta name=viewport content=width=device-width, initial-scale=1>";
 //MAIN_page+="<link rel=icon href=data:,>";
 MAIN_page+="<style>";
 MAIN_page+=" html { ";
 MAIN_page+="font-family: Sans-serif; ";
 MAIN_page+="    display: inline-block; ";
 MAIN_page+="margin: 0px auto; ";
 MAIN_page+="text-align: ";
 MAIN_page+="center;";
 MAIN_page+="   }";
 MAIN_page+=" .button {";
 MAIN_page+="    background-color: #195B6A; ";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 26px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=" .button2 {";
 MAIN_page+="   background-color: #77878A;";
 MAIN_page+=" }";
 MAIN_page+=" .mais {";
 MAIN_page+="   background-color: #195b6a;";
 MAIN_page+="    border: none; ";
 MAIN_page+="    color: white; ";
 MAIN_page+="    padding: 16px 30px;";
 MAIN_page+="    text-decoration: none; ";
 MAIN_page+="    font-size: 18px; ";
 MAIN_page+="    margin: 2px; ";
 MAIN_page+="    cursor: pointer;";
 MAIN_page+=" }";
 MAIN_page+=".mais:active {";
 MAIN_page+=" background-color: #3e8e41; ";
 MAIN_page+=" box-shadow: 0 5px #666; ";
 MAIN_page+=" transform: translateY(4px);";
 MAIN_page+="}";
 
 MAIN_page+="</style></head><body><h3>Versao AP - 2.0</h3>";

 MAIN_page+="<p><a href=\"/mais/on\"><button class=\"button mais\">VOLTAR</button></a></p><br>";
 
  MAIN_page+="<p style='background-color: #FFFF00'>Humidade : ";
  MAIN_page+=String(buff);
  sprintf(buff,"%5.1f",temperatura);
  MAIN_page+=" %   Temp.:";
  MAIN_page+=String(buff);
  MAIN_page+=" C </p> ";
 MAIN_page+="<br></body></html>";
 server.send(200, "text/html", MAIN_page);
 digitalWrite(LED_VERMELHO,LOW);
}
*/
   

/************* AUTOMATICO *********************/

void acionando() {
 String MAIN_pag="<!DOCTYPE html><html>";
 MAIN_pag+="<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
 MAIN_pag+="<style>";
 MAIN_pag+=" html { ";
 MAIN_pag+="font-family: Sans-serif; ";
 MAIN_pag+="    display: inline-block; font-size:22px;";
 MAIN_pag+="margin: 0px auto; WEIGHT:600;";
 MAIN_pag+="text-align: ";
 MAIN_pag+="center;";
 MAIN_pag+="   }</style>";
 MAIN_pag+="<body><br><br><br><br><center><b>ACIONANDO PORTAO....<br>Logando...";
 //MAIN_pag+="<script>location.href='/'</script>";
 MAIN_pag+="<BR></BODY></HTML>";
 server.send(200, "text/html", MAIN_pag);
 handleAutomatico();
}

void handleAutomatico() {
 if(DEBUG==1)
  Serial.println("->>>> MODO AUTOMATICO");
 
  output5State = "on";
// digitalWrite(LED_VERMELHO, LOW);
 digitalWrite(LED_VERDE,HIGH);
 digitalWrite(RELE_PORTAO,LOW);
 if(estadoLampada=="on") 
  analogWrite(PORTA_LAMPADA,POTENCIA100);
 delay(900);
 digitalWrite(RELE_PORTAO,HIGH);
 delay(12200);                        // tempo que portao aguarda comando pra fechar
 digitalWrite(RELE_PORTAO,LOW);
 delay(900);
 digitalWrite(RELE_PORTAO,HIGH);
 digitalWrite(LED_VERDE,LOW);
 delay(5500);     // tempo que a luz fica acesa apos comando de fechar o portao
 if(estadoLampada=="on")
  analogWrite(PORTA_LAMPADA,potencia_lampada);
 output5State = "off";
 output4State = "off";

// *********** loga a abertura, registra data hora e mac ****************
 struct station_info *stat_info;
 struct ip4_addr *IPaddress;
 IPAddress address;
 stat_info = wifi_softap_get_station_info();
 IPaddress = &stat_info->ip;
 address = IPaddress->addr;
 char mac[14];
 String mac_numerofinal;
 sprintf(mac,"%02X%02X%02X%02X%02X%02X",stat_info->bssid[0],stat_info->bssid[1],stat_info->bssid[2],stat_info->bssid[3],stat_info->bssid[4],stat_info->bssid[5]);
 mac_numerofinal=mac;
 int vezes=10;
 WiFiClient client;
 digitalWrite(LED_VERDE,HIGH);
 digitalWrite(LED_VERMELHO,HIGH);
 do {
  String GETStr="GET ";
  if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
   GETStr+="/abriuportao?mac="+mac_numerofinal;           // parametros ao URL
   GETStr+=" HTTP/1.0";
   client.println(GETStr); 
   client.println(host_cliente1);
   client.println("User-Agent: Sergio");
   client.println();
   if(DEBUG==1)  Serial.println(GETStr);
   vezes=0;
  } else {
   if(DEBUG==1) {
    Serial.println("NAO conectou a ");
    Serial.println(host_cliente1);
   } 
  }
  vezes--;
 } while(vezes>0);
 client.stop();
 digitalWrite(LED_VERDE,LOW);
 digitalWrite(LED_VERMELHO,LOW);

}
              
void handleAbreManual() {
 if(DEBUG==1)
  Serial.println("->>>> MANUAL ABRIR");
 output4State = "on";
 if(estadoLampada=="on")
  analogWrite(PORTA_LAMPADA,POTENCIA100);
 digitalWrite(LED_VERDE,HIGH);
 digitalWrite(RELE_PORTAO,LOW);
 delay(800);
 digitalWrite(RELE_PORTAO,HIGH);
 handleMais();
}

void handleFechaManual() {
// digitalWrite(LED_VERMELHO, HIGH);
 digitalWrite(LED_VERDE,LOW);
 digitalWrite(RELE_PORTAO,LOW);
 delay(800);
 digitalWrite(RELE_PORTAO,HIGH);
 if(DEBUG==1)
  Serial.println("->>>> MANUAL FECHAR");
 output4State = "off";
 if(estadoLampada=="on")
  analogWrite(PORTA_LAMPADA,potencia_lampada);
 handleMais();
}

void handleAcenderLampada() {
 estadoLampada="on";
 analogWrite(PORTA_LAMPADA,potencia_padrao);
 handleMais();
}

void handleApagarLampada() {
 analogWrite(PORTA_LAMPADA,0);
 estadoLampada="off";
 handleMais();
}

void handleAcenderLampadaGaragem() {
 estadoLampadaGaragem="on";
// digitalWrite(GARAGEM_LAMPADA,HIGH);
  WiFiClient client;
 if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
  if(DEBUG==1) {
   Serial.print("conectou a ");
   Serial.println(cliente1);
   Serial.println(host_cliente1);
  } 
  String GETStr="GET ";
  GETStr+="/garagem/on";           // parametros ao URL
  GETStr+=" HTTP/1.0";
  client.println(GETStr); 
  client.println(host_cliente1);
  client.println("User-Agent: Sergio");
  client.println();
  if(DEBUG==1) 
   Serial.println(GETStr);
 } else {
    if(DEBUG==1) {
     Serial.println("NAO conectou a ");
     Serial.println(host_cliente1);
    } 
 }
 client.stop();
 handleRoot();
}


void handleApagarLampadaGaragem() {
 //digital(GARAGEM_LAMPADA,LOW);
 estadoLampadaGaragem="off";
  WiFiClient client;
 if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
  if(DEBUG==1) {
   Serial.print("conectou a ");
   Serial.println(cliente1);
   Serial.println(host_cliente1);
  } 
  String GETStr="GET ";
  GETStr+="/garagem/off";           // parametros ao URL
  GETStr+=" HTTP/1.0";
  client.println(GETStr); 
  client.println(host_cliente1);
  client.println("User-Agent: Sergio");
  client.println();
  if(DEBUG==1) 
   Serial.println(GETStr);
 } else {
    if(DEBUG==1) {
     Serial.println("NAO conectou a ");
     Serial.println(host_cliente1);
    } 
 }
 client.stop();
 handleRoot();
}



void handleLampadaAutomaticaOff() {
 autolamp="off";
 handleMais();
}

void handleLampadaAutomaticaOn() {
 autolamp="on";
 handleMais();
}

void handleLampadaGaragemAutomaticaOff() {
 autolamp_garagem="off";
 handleMais();
}

void handleLampadaGaragemAutomaticaOn() {
 autolamp_garagem="on";
 handleMais();
}

void handlePotenciaLampada10() {
   potencia_lampada=POTENCIA10;
   potencia_padrao=potencia_lampada;
   analogWrite(PORTA_LAMPADA,potencia_lampada);
   handleMais();
}

void handlePotenciaLampada25() {
   potencia_lampada=POTENCIA25;
   potencia_padrao=potencia_lampada;
   analogWrite(PORTA_LAMPADA,potencia_lampada);
   handleMais();
}

void handlePotenciaLampada50() {
   potencia_lampada=POTENCIA50;
   potencia_padrao=potencia_lampada;
   analogWrite(PORTA_LAMPADA,potencia_lampada);
   handleMais();
}

void handlePotenciaLampada75() {
   potencia_lampada=POTENCIA75;
   potencia_padrao=potencia_lampada;
   analogWrite(PORTA_LAMPADA,potencia_lampada);
   handleMais();
}

void handlePotenciaLampada100() {
   potencia_lampada=POTENCIA100;
   potencia_padrao=potencia_lampada;
   analogWrite(PORTA_LAMPADA,potencia_lampada);
   handleMais();
}

void handleSincronizaGaragem() {
 if (autolamp_garagem=="on") {
  String GETStr="GET ";
  WiFiClient client;
  if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
   if(aceso_portao==0) {
    aceso_garagem=0;
    GETStr+="/garagem/off";           // parametros ao URL
   } else {
    aceso_garagem=1;
    GETStr+="/garagem/on";           // parametros ao URL
   }
   GETStr+=" HTTP/1.0";
   client.println(GETStr); 
   client.println(host_cliente1);
   client.println("User-Agent: Sergio");
   client.println();
   if(DEBUG==1) 
    Serial.println(GETStr);
   } else {
    if(DEBUG==1) {
    Serial.println("NAO conectou a ");
    Serial.println(host_cliente1);
   } 
  }
  client.stop();
 }  
}


// envia o valor do sensor de luz para o esp-slave
void lersensordeluz() {
  int ler,lux=0;
  for(ler=0;ler<2000;ler++) {
   lux+=analogRead(A0);
//   delay(6);
  }
  lux=lux/2000;
  String vrluz=(String)lux;
  String GETStr="GET ";
  WiFiClient client;
  if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
   GETStr+="/sensordeluz?luz="+vrluz;           // parametros ao URL
   GETStr+=" HTTP/1.0";
   client.println(GETStr); 
   client.println(host_cliente1);
   client.println("User-Agent: Sergio");
   client.println();
   if(DEBUG==1) 
    Serial.println(GETStr);
   } else {
    if(DEBUG==1) {
    Serial.println("NAO conectou a ");
    Serial.println(host_cliente1);
   } 
  }
  client.stop();
}

void handleluzportaoon() {
 potencia_lampada=potencia_padrao;
 analogWrite(PORTA_LAMPADA,potencia_lampada);
 estadoLampada="on";
 estadoLampadaGaragem="on";
}

void handleluzportaooff() {
 analogWrite(PORTA_LAMPADA,0);
 potencia_lampada=0;
 estadoLampada="off";
 estadoLampadaGaragem="off";
}


/*
void handleMostraMac() {
 String MAIN_page="<!DOCTYPE html><html>";
 MAIN_page+="<head><meta name=viewport content=width=device-width, initial-scale=1>";
 MAIN_page+="<body>";
 int t;
 MAIN_page+="<br>Total de MAC : ";
 MAIN_page+=indice_mac_conectado;
 MAIN_page+="<br><br>";
 for(t=0;t<indice_mac_conectado;t++) {
   MAIN_page+="<br>MAC ";
   MAIN_page+=mac_conectado[t];
  }
 MAIN_page+="</body>";
 MAIN_page+="</html>";
 server.send(200, "text/html", MAIN_page);
}
*/
//===============================================================
//                  SETUP
//===============================================================
void setup(void){
  if(DEBUG==1) {
   Serial.begin(9600);
   Serial.println("");
  }

  pinMode(RELE_PORTAO, OUTPUT);
  pinMode(RELE_LAMPADA, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(PORTA_LAMPADA,OUTPUT);
  pinMode(GPIO16,OUTPUT);
  analogWriteFreq(500);                         // frequencia do PWM em Hz

//  dht.setup(SENSOR_TEMPERATURA, DHTesp::DHT22); // Connect DHT sensor to GPIO 2
 
  digitalWrite(RELE_PORTAO, HIGH);
  digitalWrite(RELE_LAMPADA, HIGH);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(GPIO16, LOW);

  WiFi.mode(WIFI_AP);           // Apenas AP
//  WiFi.mode(WIFI_AP_STA);         // AP + ST
  WiFi.setPhyMode(MODOWIFI);
 IPAddress ip(192, 168, 4, 1);
 IPAddress gateway(192, 168, 4, 1);
 IPAddress subnet(255, 255, 255, 0);
 WiFi.config(ip, gateway, subnet,gateway);

 //uint8 mode = 0;
 //wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &mode);
  
  WiFi.softAP(ssid, password,canal_wifi,false,max_connection);  // Inicia HOTspot
  wifi_softap_dhcps_start();    //inicia DHCP sercer

  if(DEBUG==1) {
   IPAddress myIP = WiFi.softAPIP(); //Get IP address
   Serial.print("HotSpt IP:");
   Serial.println(myIP);
  }
  server.on("/", handleRoot);                        // pagina principal
  server.on("/5/on", acionando);              // abre automatico
  server.on("/4/on", handleAbreManual);              // abre manual
  server.on("/4/off", handleFechaManual);            // fecha manual
  
  server.on("/6/on", handleAcenderLampada);          // acende lampada
  server.on("/6/off", handleApagarLampada);          // apaga lampada
  
  server.on("/9/on", handleAcenderLampadaGaragem);          // acende lampada
  server.on("/9/off", handleApagarLampadaGaragem);          // apaga lampada
  
  
  server.on("/7/off", handleLampadaAutomaticaOff);   // desativa lampada automatica
  server.on("/7/on", handleLampadaAutomaticaOn);     // ativa lampada automatica

  server.on("/8/off", handleLampadaGaragemAutomaticaOff);   // desativa lampada automatica
  server.on("/8/on", handleLampadaGaragemAutomaticaOn);     // ativa lampada automatica
  
  server.on("/sincroniza_garagem",handleSincronizaGaragem);
  
  server.on("/10/on", handlePotenciaLampada10);      // 10% de potencia na lampada
  server.on("/25/on", handlePotenciaLampada25);      // 25% de potencia na lampada
  server.on("/50/on", handlePotenciaLampada50);      // 50% potencia na lampada
  server.on("/75/on", handlePotenciaLampada75);      // 75% potencia na lampada
  server.on("/100/on", handlePotenciaLampada100);    // 100% potencia na lampada
  server.on("/mais/on", handleMais);                 // segunda tela

  server.on("/sensorluz", lersensordeluz);                 // envia leitura do sensor de luz
  
  server.on("/luzportao/on", handleluzportaoon);                 // liga luz do portao
  server.on("/luzportao/off", handleluzportaooff);                 // desliga luz do portao
  
  
  server.begin();                  // inicia server
  if(DEBUG==1) 
   Serial.println("HTTP server started");
//  digitalWrite(LED_VERMELHO,HIGH);
 
 //atualizador OTA
  ArduinoOTA.setPort(8266);     // Porta
  ArduinoOTA.setHostname(host_ota);  // hostname
/*
ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    if(DEBUG==1)
     Serial.println("upload via OTA tipo " + type);
  });
  ArduinoOTA.onEnd([]() {
    if(DEBUG==1)
     Serial.println("\nFim OTA");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if(DEBUG==1)
     Serial.printf("Progresso OTA: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if(DEBUG==1)
     Serial.printf("Erro OTA[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      if(DEBUG==1)
       Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      if(DEBUG==1)
       Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      if(DEBUG==1)
       Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      if(DEBUG==1)
       Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      if(DEBUG==1)
       Serial.println("End Failed");
    }
  });
*/  
  ArduinoOTA.begin(); 
  
  /**** WATCHDOG *****/
 ESP.wdtEnable(8000);
}
//===============================================================
//                     LOOP
//===============================================================
void loop(void){
  server.handleClient();  //Handle client requests
  MDNS.update();
  
  unsigned long current_leitura_sensor_luz=millis();
  if( current_leitura_sensor_luz - previous_leitura_sensor_luz > tempo_leitura_sensor_luz) { 
   if(autolamp=="on")
     iluminacao();
   if(autolamp_garagem=="on") 
     iluminacao_garagem(); 
   previous_leitura_sensor_luz=millis();
  }
  ArduinoOTA.handle();   
  delay(7);
  unsigned long currentMillis_alive = millis();    //Tempo atual em ms
  if (currentMillis_alive - previousMillis_alive > aliveInterval) { 
   digitalWrite(LED_VERMELHO,estadopisca); 
   estadopisca=!estadopisca;
   previousMillis_alive = currentMillis_alive;    // Salva o tempo atual
  } 

  
  ESP.wdtFeed();  // RESETA O WATCHDOG
}

/**********************************************************/

void iluminacao() {
  int lux=0,v=0;
  for(v=0; v<50; v++)
   lux+=analogRead(A0);
  lux=lux/50;
  if(lux < acender_luz) {
   aceso_portao=1;
   analogWrite(PORTA_LAMPADA,potencia_lampada);
   if(DEBUG==1)
    Serial.println(potencia_lampada);
   estadoLampada="on";
  } else {
   aceso_portao=0;
   analogWrite(PORTA_LAMPADA,0);
   estadoLampada="off";
  }
} 
void iluminacao_garagem() {
 if(aceso_garagem!=aceso_portao) {
  String GETStr="GET ";
  WiFiClient client;
  if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
   if(aceso_portao==0) {
    aceso_garagem=0;
    GETStr+="/garagem/off";           // parametros ao URL
   } else {
    aceso_garagem=1;
    GETStr+="/garagem/on";           // parametros ao URL
   }
   GETStr+=" HTTP/1.0";
   client.println(GETStr); 
   client.println(host_cliente1);
   client.println("User-Agent: Sergio");
   client.println();
   if(DEBUG==1) 
    Serial.println(GETStr);
   } else {
    if(DEBUG==1) {
    Serial.println("NAO conectou a ");
    Serial.println(host_cliente1);
   } 
  }
  client.stop();
 }  
}


/*
void iluminacao_garagem() {
  // COMANDA O OUTRO ESP
  WiFiClient client;
 if (client.connect(cliente1, 80)) {           // conecta ao esp cliente
  if(DEBUG==1) {
   Serial.print("conectou a ");
   Serial.println(cliente1);
   Serial.println(host_cliente1);
  } 
  String GETStr="GET ";
  if(aceso_garagem==1)
   GETStr+="/garagem/on";           // parametros ao URL
  else
   GETStr+="/garagem/off";           // parametros ao URL
  GETStr+=" HTTP/1.0";
  client.println(GETStr); 
  client.println(host_cliente1);
  client.println("User-Agent: Sergio");
  client.println();
  if(DEBUG==1) 
   Serial.println(GETStr);
 } else {
    if(DEBUG==1) {
     Serial.println("NAO conectou a ");
     Serial.println(host_cliente1);
    } 
 }
 client.stop();
 return;
}
*/
