#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h" 

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "chasdai";
const char* password = "1122334455";

ESP8266WebServer server(80);

const char* www_username = "admin";
const char* www_password = "esp8266";

String login = "";
String myWeb1 = "";
String myWeb2 = "";
String loginFail = "";
String NotFound = "";

int gpio0_pin = 12;
int gpio2_pin = 13;

bool flag = 0;

String parseFile(String path){
  String str = "";
    File file = SPIFFS.open(path , "r");
  if (!file) {
    str = "failed to load"  + path;
    return str;
  } else{
    while (file.available()) {
    str += (char)file.read();
    }
    file.close();
  }
  return str;
}

bool is_authentified(){
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;  
}

void handleLogin(){
  if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    server.send(200, "text/html", login);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if (server.arg("USERNAME") == "Fiction" &&  server.arg("PASSWORD") == "Pulp" ){
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Log in Successful");
      flag=0;
      return;
    }
  Serial.println("Log in Failed");
  flag=1;
  }
  if (!flag)
    server.send(200, "text/html", login);
  else
    server.send(200, "text/html", loginFail);
}

void handleRoot1(){
  Serial.println("Enter handleRoot1");
  flag=0;
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  String state=server.arg("state");
  if (state == "on") digitalWrite(13, LOW);
  else if (state == "off") digitalWrite(13, HIGH);
  server.send(200, "text/html", myWeb1);
}


void handleRoot1SON(){
  Serial.println("Enter handleRoot1SON");
  flag=0;
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  digitalWrite(gpio0_pin, HIGH);
  server.send(200, "text/html", myWeb1);
}


void handleRoot1SOFF(){
  Serial.println("Enter handleRoot1SOFF");
  flag=0;
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  digitalWrite(gpio0_pin, LOW);
  server.send(200, "text/html", myWeb1);
}

void handleRoot2(){
  Serial.println("Enter handleRoot2");
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  server.send(200, "text/html", myWeb2);
}

void handleRoot2SON(){
  Serial.println("Enter handleRoot2SON");
  flag=0;
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  digitalWrite(gpio2_pin, HIGH);
  server.send(200, "text/html", myWeb2);
}

void handleRoot2SOFF(){
  Serial.println("Enter handleRoot2SOFF");
  flag=0;
  String header;
  if (!is_authentified()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  digitalWrite(gpio2_pin, LOW);
  server.send(200, "text/html", myWeb2);
}

void handleNotFound(){
  server.send(200, "text/html", NotFound);
}

void setup(void){
 
  // preparing GPIOs
  pinMode(gpio0_pin, OUTPUT);
  digitalWrite(gpio0_pin, LOW);
  pinMode(gpio2_pin, OUTPUT);
  digitalWrite(gpio2_pin, LOW);

  delay(1000);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  SPIFFS.begin();
  login =  parseFile("/login.html");
  loginFail =  parseFile("/loginFail.html");
  myWeb1 = parseFile("/myWeb1.html");
  myWeb2 = parseFile("/myWeb2.html");
  NotFound = parseFile("/NotFound.html");
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
 
  server.on("/", handleRoot1);
  server.on("/login", handleLogin);  
  server.on("/FirstPage", handleRoot1);
  server.on("/SecondPage", handleRoot2);
  server.on("/socket1On", handleRoot2SON);
  server.on("/socket1Off", handleRoot2SOFF);
  server.on("/socket2On", handleRoot1SON);
  server.on("/socket2Off", handleRoot1SOFF);  
  /*
  server.on("/", [](){
    server.send(200, "text/html", login);
    Serial.println("defaultttttt");
  });
  server.on("/socket1On", [](){
    server.send(200, "text/html", myWeb2);
    digitalWrite(gpio0_pin, HIGH);
    //delay(1000);
  });
  server.on("/socket1Off", [](){
    server.send(200, "text/html", myWeb2);
    digitalWrite(gpio0_pin, LOW);
    //delay(1000); 
  });
  server.on("/socket2On", [](){
    server.send(200, "text/html", myWeb1);
    digitalWrite(gpio2_pin, HIGH);
    //delay(1000);
  });
  server.on("/socket2Off", [](){
    server.send(200, "text/html", myWeb1);
    digitalWrite(gpio2_pin, LOW);
    //delay(1000); 
  });
    server.on("/FirstPage", [](){
    server.send(200, "text/html", myWeb1);
    //delay(1000);
  });
    server.on("/SecondPage", [](){
    server.send(200, "text/html", myWeb2);
    //delay(1000);
  });
    server.onNotFound([](){
    server.send(200, "text/html", login);
  });*/
  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize ); 
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void){
  server.handleClient();
} 
