#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <FS.h>
#include "PubSubClient.h"
#include "SSD1306Brzo.h"
#include "DHT.h"
#include "mFonts.h"

//////网页端参数/////
u16 LOOP_SPAN = 500;	//每500ms刷新一次数据
u32 POST_SPAN = 180000L;//每180s发送一次数据

String CHIP_ID = (String)ESP.getChipId();
String HARD_VER = "WL3THV2DA v1 000";
String SOFT_VER = "0.1.0 Build 181229 Rel.01";

u16 SERVER_PORT = 6002;
String SERVER_IP = "183.230.40.39";
String POST_TOPIC = "$dp";
String PRODUCT_ID = "96258";
String AUTH_INFO = "12E171016001";
String DEV_ID = "19916638";

IPAddress WEB_IP(192, 168, 1, 1);
IPAddress WEB_SUBNET(255, 255, 255, 0);
String WEB_PASSWORD = "admin";
String AP_NAME = "ONENET/MQTT/" + PRODUCT_ID + "/" + DEV_ID;

String STA_SSID = "BadWi5";
String STA_PASSWORD = "87654321";

IPAddress STATIC_IP(0, 0, 0, 0);
IPAddress STATIC_SUBNET(0, 0, 0, 0);
IPAddress STATIC_GATEWAY(0, 0, 0, 0);
IPAddress STATIC_DNS(0, 0, 0, 0);

u8 PIN_SDA = 5;  // D1(IO5)→SDA
u8 PIN_SCL = 4;	 // D2(IO4)→SCL
u8 PIN_DHT = 12; // D6(IO12)→DHT
u8 PIN_BTN = 0;	 // D3(IO0)→BUTTON
u8 PIN_LED = 12; // D6(IO12)→LED
//////网页端参数/////


WiFiClient tcpClient;
PubSubClient mqttClient(SERVER_IP.c_str(), SERVER_PORT, tcpClient);
ESP8266WebServer webServer(80);
DNSServer dnsServer;
SSD1306Brzo display(0x3c, PIN_SDA, PIN_SCL);

DHT dht(PIN_DHT,DHT22);
Ticker btnTicker, mainTicker;

u8 n_checkWifi = 1, n_readDHTFailed = 0;
char s_temperature[5], s_humidity[5];
u16 n_btnDown = 0, n_btnUp = 0, n_oledOn = 0, n_readDHT = 0, n_readRSSI = 0, n_loop = 0;
u32 sumRMS = 0;
bool f_enableLoop = true, f_sendOK = true;
enum { ALWAYSON, AUTOOFF } oledMode;
enum { AUTOCONNECT, SMARTCONFIG, WEBCONFIG, CONNECTSERVER } configMode;
typedef struct {
	real32 temperature;
	real32 humidity;
	s32 rssi;
	u32 voltage;
} mData;
mData rtData = { 0,0,0,0 }, totData = { 0,0,0,0 }, avgData = { 0,0,0,0 };

/****************按键功能检测****************/
void btnTickerFunc() {							//每10ms定时查询按键状态
	if (digitalRead(0) == LOW) {                //按键处于按下状态
		n_btnUp = 0;
		if (n_btnDown < 500) {
			n_btnDown++;
			if (n_btnDown == 100) {			//按下1s触发长按功能切换WIFI配置模式
				n_checkWifi = 1;
				if (configMode == AUTOCONNECT)
					configMode = SMARTCONFIG;
				else if (configMode == SMARTCONFIG)
					configMode = AUTOCONNECT;
				else if (configMode == WEBCONFIG)
					configMode = AUTOCONNECT;
				else if (configMode == CONNECTSERVER)
					configMode = SMARTCONFIG;
			}
			else if (n_btnDown == 500) {	//按下5s触发长按功能进入WEB配置模式
				n_checkWifi = 1;
				configMode = WEBCONFIG;
			}
		}
	}
	else {                                      //按键处于松开状态
		n_btnUp++;
		if (n_btnUp > 5) {
			btnTicker.detach();
			if (n_btnDown > 5 && n_btnDown < 100) {
				if (oledMode == ALWAYSON) 		//按下50ms~1000ms触发短按功能切换屏幕长亮/延时10s息屏
					oledMode = AUTOOFF;
				else if (oledMode == AUTOOFF) {
					oledMode = ALWAYSON;
					display.displayOn();
				}
			}
			n_btnDown = 0;
			n_btnUp = 0;
			attachInterrupt(0, btnInterFunc, FALLING);
		}
	}
}

void btnInterFunc() {                           //按键下降沿中断触发
	n_btnDown++;
	btnTicker.attach_ms(10, btnTickerFunc);
	detachInterrupt(0);
}

/****************主循环定时器****************/
void mainTickerFunc() {
	f_enableLoop = true;
	if (oledMode == AUTOOFF)
		if (n_oledOn == 20) 					//定时20×500ms=10s自动关闭屏幕
			display.displayOff();
		else
			n_oledOn++;
	else if (oledMode == ALWAYSON)
		n_oledOn = 0;
}

/****************主程序****************/
void setup() {
	//Serial.begin(115200);
	display.init();
	dht.begin();
	WiFi.mode(WIFI_STA);
	ArduinoOTA.setHostname(AP_NAME.c_str());
	ArduinoOTA.setPassword(AUTH_INFO.c_str());
	ArduinoOTA.onStart([]() {
		display.clear();
		display.setFont(ArialMT_Plain_16);
		display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
		display.drawString(64, 24, "OTA Update");
		//Serial.print("Start OTA update");
		display.display();
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		display.drawProgressBar(4, 40, 120, 8, progress / (total / 100));
		//Serial.print(".");
		display.display();
	});

	ArduinoOTA.onEnd([]() {
		display.clear();
		display.setFont(ArialMT_Plain_16);
		display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
		display.drawString(64, 24, "Waiting for");
		display.drawString(64, 40, "Restart");
		//Serial.print("updating complete");
		display.display();
	});
	ArduinoOTA.begin();
	pinMode(PIN_BTN, INPUT);
	attachInterrupt(PIN_BTN, btnInterFunc, FALLING);
	mainTicker.attach_ms(LOOP_SPAN, mainTickerFunc);    //配置主循环500ms定时中断
	//Serial.println("Start program");
}

void loop() {
	ArduinoOTA.handle();
	if (configMode == WEBCONFIG) {
		dnsServer.processNextRequest();
		webServer.handleClient();
	}
	if (!f_enableLoop)
		return;
	f_enableLoop = false;
	n_loop++;

	///////////////切换自动连接/智能配置模式///////////////
	if (configMode == AUTOCONNECT) {
		//Serial.print("AutoConnect Mode : ");
		if (WiFi.status() != WL_CONNECTED) {
			if (n_checkWifi == 1) {
				//dnsServer.stop();
				WiFi.mode(WIFI_STA);
				WiFi.stopSmartConfig();
				SPIFFS.end();
				WiFi.config(STATIC_IP, STATIC_GATEWAY, STATIC_SUBNET, STATIC_DNS);
				WiFi.begin(STA_SSID.c_str(), STA_PASSWORD.c_str());
				//Serial.print("Connecting Wifi  ");
			}
			else if (n_checkWifi > 40)
				n_checkWifi = 0;
			n_checkWifi++;
		}
		else if (mqttClient.state() != MQTT_CONNECTED) {
			configMode = CONNECTSERVER;
			mqttClient.connect(DEV_ID.c_str(), PRODUCT_ID.c_str(), AUTH_INFO.c_str());
			//Serial.print("Connecting Server  ");
			n_checkWifi = 1;
		}
		else {
			mqttClient.loop();
			n_checkWifi = 0;
			//Serial.print("Connection OK  ");
		}
	}
	else if (configMode == SMARTCONFIG) {
		//Serial.print("SmartConfig Mode : ");
		if (n_checkWifi == 1) {
			//Serial.print("Start SmartConfig  ");
			WiFi.beginSmartConfig();
		}
		else if (n_checkWifi > 40)
			n_checkWifi = 0;
		n_checkWifi++;
		if (WiFi.smartConfigDone()) {
			//Serial.print("SmartConfig Done  ");
			configMode = AUTOCONNECT;
			STA_SSID = WiFi.SSID();
			STA_PASSWORD = WiFi.psk();
			STATIC_IP = 0u;
			STATIC_GATEWAY = 0u;
			STATIC_SUBNET = 0u;
			STATIC_DNS = 0u;
			WiFi.setAutoConnect(true);
		}
	}
	else if (configMode == WEBCONFIG) {
		//Serial.print("WebConfig Mode : ");
		if (n_checkWifi == 1) {
			//Serial.print("Start WebConfig  ");
			WiFi.stopSmartConfig();
			WiFi.mode(WIFI_AP);
			SPIFFS.begin();
			WiFi.softAP(AP_NAME.c_str());
			WiFi.softAPConfig(WEB_IP, WEB_IP, WEB_SUBNET);
			dnsServer.start((byte)53, "*", WEB_IP);
			webServer.on("/", handleRoot);
			webServer.on("/login", HTTP_POST, handleLogin);
			webServer.on("/load", HTTP_POST, handleLoad);
			webServer.on("/pswConfig", HTTP_POST, handlePswConfig);
			webServer.on("/hotSpot", HTTP_POST, handleHotSpot);
			webServer.on("/webConfig", HTTP_POST, handleWebConfig);
			webServer.on("/argsConfig", HTTP_POST, handleArgsConfig);
			webServer.onNotFound(handleNotFound);
			webServer.begin();
		}
		else if (n_checkWifi > 40)
			n_checkWifi = 1;
		n_checkWifi++;
	}
	else if (configMode == CONNECTSERVER) {
		if (mqttClient.state() == MQTT_CONNECTED) {
			configMode = AUTOCONNECT;
			n_checkWifi = 0;
		}
		else if (n_checkWifi < 15)
			n_checkWifi++;
		else {
			configMode = AUTOCONNECT;
			n_checkWifi = 1;
		}
	}

	///////////////读取WIFI信号强度DB并转换为百分比///////////////
	rtData.rssi = WiFi.RSSI();
	if (rtData.rssi < 0)
	{
		n_readRSSI++;
		if (rtData.rssi >= -50)
			rtData.rssi = 100;
		else if (rtData.rssi <= -100)
			rtData.rssi = 0;
		else
			rtData.rssi = 2 * (rtData.rssi + 100);
		totData.rssi += rtData.rssi;
	}
	else rtData.rssi = 0;

	///////////////读取电压///////////////
	noInterrupts();
	u16 adc = 0, rtRMS = 0;
	u32 sumsq = 0, tm = 0;
	for (u8 i = 0; i < 40; i++) {
		tm = micros();
		analogRead(A0);
		adc = analogRead(A0);
		sumsq += adc * adc;
		while ((micros() - tm) < 500);
	}
	// ADC = Vin * 0.3 / (112.5 + 0.223) * 1023 -> Vin = 0.3673 * ADC
	// Vrms = sqrt(Σ(Vin^2)/n)	
	rtRMS = 0.05807 * sqrt(sumsq);
	totData.voltage += rtRMS;
	sumRMS += rtRMS;
	if (n_loop % 4 == 0) {
		rtData.voltage = sumRMS / 4;
		sumRMS = 0;
	}
	interrupts();

	///////////////读取温湿度传感器///////////////
	if (dht.read()) {
		n_readDHT++;
		n_readDHTFailed = 0;
		rtData.temperature = dht.readTemperature();
		rtData.humidity = dht.readHumidity();
		totData.temperature += rtData.temperature;
		totData.humidity += rtData.humidity;
		dtostrf(rtData.temperature, 2, 1, s_temperature);
		dtostrf(rtData.humidity, 2, 1, s_humidity);
	}
	else if (n_readDHTFailed > 20) {
		if (oledMode == AUTOOFF) {
			oledMode = ALWAYSON;
			display.displayOn();
		}
	}
	else
		n_readDHTFailed++;

	//Serial.print("temperature:" + String(rtData.temperature) + " humidity:" + String(rtData.humidity) + " voltage:" + String(rtData.voltage) + " wifi:" + String(rtData.rssi));

	///////////////发送数据///////////////
	if (LOOP_SPAN *(n_loop + 1) >= POST_SPAN || !f_sendOK) {//上传数据时间已到或上次未成功上传
		if (f_sendOK || (!f_sendOK && LOOP_SPAN *(n_loop + 1) >= POST_SPAN)) {
			avgData.temperature = totData.temperature / n_readDHT;
			avgData.humidity = totData.humidity / n_readDHT;
			avgData.voltage = totData.voltage / n_loop;
			avgData.rssi = totData.rssi / n_readRSSI;
			n_loop = 0;
			n_readDHT = 0;
			n_readRSSI = 0;
			totData = { 0,0,0,0 };
		}
		char msg[80];
		snprintf(msg + 3, sizeof(msg), "{\"Temp\":%.1f,\"Humi\":%.1f,\"Volt\":%d,\"RSSI\":%d,\"State\":\"online\"}",
			avgData.temperature, avgData.humidity, avgData.voltage, avgData.rssi);
		u8 streamLen = strlen(msg + 3);	//从msg[3]开始字符串长度
		msg[0] = '\x03';
		msg[1] = streamLen >> 8;
		msg[2] = streamLen & 0xFF;
		f_sendOK = mqttClient.publish(POST_TOPIC.c_str(), (u8*)msg, streamLen + 3, false);
		if (configMode == AUTOCONNECT && f_sendOK) {
			//Serial.print("\r\nPost Data Success");
			configMode = CONNECTSERVER;
		}
		else {
			//Serial.print("\r\nPost Data Failed");
		}
	}

	///////////////更新屏幕显示///////////////
	display.clear();
	//WIFI连接模式或信号强度显示
	if (n_checkWifi == 0) {
		display.setFont(mIcon_16x16);
		if (rtData.rssi == 100)
			display.drawString(0, 0, "\x05");
		else if (rtData.rssi > 66)
			display.drawString(0, 0, "\x04");
		else if (rtData.rssi > 33)
			display.drawString(0, 0, "\x03");
		else if (rtData.rssi > 0)
			display.drawString(0, 0, "\x02");
		else
			display.drawString(0, 0, "\x01");
		display.setFont(ArialMT_Plain_10);
		display.drawString(20, 5, String(rtData.rssi));
	}
	else {
		display.setFont(ArialMT_Plain_10);
		if (configMode == AUTOCONNECT) {
			display.drawString(0, 0, "AUTOCONNECT");
			for (u8 i = 0; i <= (n_checkWifi % 5); i++)
				display.setPixel(78 + i * 3, 9);
		}
		else if (configMode == SMARTCONFIG) {
			display.drawString(0, 0, "SMARTCONFIG");
			for (u8 i = 0; i <= (n_checkWifi % 5); i++)
				display.setPixel(75 + i * 3, 9);
		}
		else if (configMode == WEBCONFIG) {
			display.drawString(0, 0, "WEBCONFIG");
			for (u8 i = 0; i <= (n_checkWifi % 5); i++)
				display.setPixel(63 + i * 3, 9);
		}
		else if (configMode == CONNECTSERVER) {
			display.drawString(0, 0, "CONNECTSERV");
			for (u8 i = 0; i <= (n_checkWifi % 5); i++)
				display.setPixel(78 + i * 3, 9);
		}
	}
	//状态栏图标显示
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.setFont(mIcon_10x10);
	if (configMode == AUTOCONNECT)					//自动连接模式
		display.drawString(127, 0, "\x03");
	else if (configMode == SMARTCONFIG)				//智能配置模式
		display.drawString(127, 0, "\x04");
	else if (configMode == WEBCONFIG)
		display.drawString(127, 0, "\x05");			//WEB配置模式
	else if (configMode == CONNECTSERVER) {
		display.drawString(127, 0, "\x06");			//连接服务器/发送数据
	}
	u8 seek = 117;
	if (oledMode == AUTOOFF) {						//屏幕自动休眠
		display.drawString(seek, 0, "\x01");
		seek -= 10;
	}
	if (n_readDHTFailed > 4)						//温湿度传感器读取失败
		display.drawString(seek, 0, "\x02");
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	if (n_readDHTFailed > 20) {
		display.setFont(ArialMT_Plain_16);
		display.drawString(0, 20, "SENSOR");
		display.drawString(0, 40, "ERROR !");
	}
	else {
		display.setFont(mFont_16x16);
		display.drawString(0, 16, "\x01");			//温
		display.drawString(16, 16, "\x03");			//度
		display.drawString(0, 32, "\x02");			//湿
		display.drawString(16, 32, "\x03");			//度
		display.setFont(ArialMT_Plain_16);
		display.drawString(40, 16, String(s_temperature) + "°C");
		display.drawString(40, 32, String(s_humidity) + "%");
	}
	display.setFont(mFont_16x16);
	display.drawString(0, 48, "\x04");				//电
	display.drawString(16, 48, "\x05");				//压
	display.setFont(ArialMT_Plain_16);
	display.drawString(40, 48, String(rtData.voltage) + "V");
	display.display();

	//Serial.println("");
}

