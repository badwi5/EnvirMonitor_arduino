/****************WEB配置部分****************/

String getContentType(String filename) {
	if (webServer.hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

/** Handle root or redirect to captive portal */
void handleRoot() {
	//Serial.println("handleRoot");
	File file = SPIFFS.open("/8266Login.html", "r");
	webServer.streamFile(file, "text/html");
	file.close();
}

void handleLogin() {
	//Serial.println("handleLogin");
	if (webServer.hasArg("psw")) {
		if (webServer.arg("psw") == WEB_PASSWORD) {
			//Serial.println("navigate to config page");
			webServer.send(200, "text/plain", "success");
		}
		else {
			//Serial.println("Authentication failed");
			webServer.send(200, "text/plain", "Authentication failed");
		}
	}
	else {
		//Serial.println("Wrong arguments");
	}
}

void handleLoad() {
	//Serial.println("handleLoad");
	webServer.send(200, "text/json", generateJson());
}

void handlePswConfig() {
	//Serial.println("handlePswConfig");
	if (webServer.hasArg("psw")) {
		WEB_PASSWORD = webServer.arg("psw");
		webServer.send(200, "text/plain", "success");
	}
	else webServer.send(200, "text/plain", "Missing arguments");
}

void handleHotSpot() {
	//Serial.println("handleHotSpot");
	String args[] = { "ssid","psw","sIp","sGtw","sSbn","sDns" };
	for (u8 i = 0; i < 6; i++) {
		if (!webServer.hasArg(args[i])) {
			webServer.send(200, "text/plain", "Missing arguments");
			return;
		}
	}
	STA_SSID = webServer.arg("ssid");
	STA_PASSWORD = webServer.arg("psw");
	
	//Serial.println("ssid:" + STA_SSID);
	//Serial.println(" password:" + STA_PASSWORD);
	
	STATIC_IP.fromString(webServer.arg("sIp"));
	//Serial.println(" sIp:" + STATIC_IP.toString());

	STATIC_GATEWAY.fromString(webServer.arg("sGtw"));
	//Serial.println(" sGtw:" + STATIC_GATEWAY.toString());

	STATIC_SUBNET.fromString(webServer.arg("sSbn"));
	//Serial.println(" sSbn:" + STATIC_SUBNET.toString());

	STATIC_DNS.fromString(webServer.arg("sDns"));
	//Serial.println(" sDns:" + STATIC_DNS.toString());

	webServer.send(200, "text/plain", "success");
}

void handleWebConfig() {
	//Serial.println("handleWebConfig");
	String args[] = { "ip","port","prodId","devId","auth","topic" };
	for (u8 i = 0; i < 6; i++) {
		if (!webServer.hasArg(args[i])) {
			webServer.send(200, "text/plain", "Missing arguments");
			return;
		}
	}
	SERVER_IP = webServer.arg("ip");
	SERVER_PORT = webServer.arg("port").toInt();
	PRODUCT_ID = webServer.arg("prodId");
	DEV_ID = webServer.arg("devId");
	AUTH_INFO = webServer.arg("auth");
	POST_TOPIC = webServer.arg("topic");

	webServer.send(200, "text/plain", "success");
}

void handleArgsConfig() {
	//Serial.println("handleArgsConfig");
	if (webServer.hasArg("tm")) {
		POST_SPAN = webServer.arg("tm").toInt();
		webServer.send(200, "text/plain", "success");
	}
	else
		webServer.send(200, "text/plain", "Missing arguments");
}

void handleNotFound() {
	//if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
	//	return;
	//}
	String path = webServer.uri();
	String contentType = getContentType(path);
	//Serial.println(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
		webServer.streamFile(file, contentType);
		file.close();
		return;
	}
	//Serial.println("file not found");
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += webServer.uri();
	message += "\nMethod: ";
	message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += webServer.args();
	message += "\n";

	for (uint8_t i = 0; i < webServer.args(); i++) {
		message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
	}
	webServer.send(404, "text/plain", message);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
//boolean captivePortal() {
//	String header = webServer.hostHeader();
	//Android captive portal:generate_204;Microsoft captive portal:fwlink;IOS:captive.apple.com
//	if (!IPAddress::isValid(header)) {
//		//Serial.println("redirect to homepage");
//		webServer.sendHeader("Location", String("http://" + WEB_IP.toString()), true);
//		webServer.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
//		webServer.client().stop(); // Stop is needed because we sent no content length
//		return true;
//	}
//	return false;
//}