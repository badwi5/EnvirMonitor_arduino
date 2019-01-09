String generateJson() {
	char jsonString[800];
	snprintf(jsonString, sizeof(jsonString),
		"{"
		"\"devInf\":{"
		"\"chipId\":\"%s\","
		"\"webIp\":\"%s\","
		"\"hwVer\":\"%s\","
		"\"swVer\":\"%s\","
		"\"apid\":\"%s\""
		"},"
		"\"psw\":\"%s\","
		"\"netCfg\":{"
		"\"wifi\":{"
		"\"ssid\":\"%s\","
		"\"psw\":\"%s\","
		"\"sIP\":{"
		"\"ip\":\"%s\","
		"\"gtw\":\"%s\","
		"\"sbn\":\"%s\","
		"\"dns\":\"%s\""
		"}"
		"},"
		"\"serv\":{"
		"\"ip\":\"%s\","
		"\"port\":%d,"
		"\"prodID\":\"%s\","
		"\"devID\":\"%s\","
		"\"auth\":\"%s\","
		"\"topic\":\"%s\""
		"}"
		"},"
		"\"args\":{"
		"\"loopTm\":%u,"
		"\"postTm\":%lu,"
		"\"sda\":%u,"
		"\"scl\":%u,"
		"\"dht\":%u,"
		"\"btn\":%u,"
		"\"led\":%u"
		"}"
		"}", CHIP_ID.c_str(), WEB_IP.toString().c_str(), HARD_VER.c_str(), SOFT_VER.c_str(), AP_NAME.c_str(), WEB_PASSWORD.c_str(), STA_SSID.c_str(), STA_PASSWORD.c_str(),
		STATIC_IP.toString().c_str(), STATIC_GATEWAY.toString().c_str(), STATIC_SUBNET.toString().c_str(), STATIC_DNS.toString().c_str(),
		SERVER_IP.c_str(), SERVER_PORT, PRODUCT_ID.c_str(), DEV_ID.c_str(), AUTH_INFO.c_str(), POST_TOPIC.c_str(),
		LOOP_SPAN, POST_SPAN, PIN_SDA, PIN_SCL, PIN_DHT, PIN_BTN, PIN_LED
	);
	//Serial.println(jsonString);
	return jsonString;
}