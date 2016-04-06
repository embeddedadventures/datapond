/*
Copyright (c) 2016, Embedded Adventures
All rights reserved.
Contact us at source [at] embeddedadventures.com
www.embeddedadventures.com
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
- Neither the name of Embedded Adventures nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
*/

// Arduino library for communicating with Embeddded Adventures datapond server using HTTP
// Written originally by Embedded Adventures


#include "Arduino.h"
#include "http-datapond.h"
#include "ESP8266HTTPClient.h"

HttpDatapond::HttpDatapond(const char* ip, int port) {
	pondIPAddress = ip;
	serverPort = port;
}

int	HttpDatapond::login(const char* username, const char* password) {
	url = "/user/login";
	HTTPClient::begin(pondIPAddress, serverPort, url);
	//body = "{\"email\": \"soon@along.com\",";
	//body += " \"password\": \"blankevent\"}\r\n";
	body = "{\"email\": \"" + (String)username;
	body += "\",";
	body += " \"password\": \"" + (String)password;
	body += "\"}\r\n";
	return HTTPClient::POST(body);
}

void HttpDatapond::collectCookie() {
	payload = HTTPClient::getString();
	int cookieStart, cookieEnd;
	cookieStart = payload.indexOf("session") + 10;
	cookieEnd = payload.indexOf('}') - 1;
	cookie = "session=" + payload.substring(cookieStart, cookieEnd);
	Serial.print("\nCookie: ");
	Serial.println(cookie);
	Serial.println();
}

int HttpDatapond::getLastDroplet(int stream_id) {
	url = "/droplet/last?stream=" + (String)stream_id;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::createDroplet(int stream_id, double data) {
	url = "/droplet?stream=";
	url += stream_id;
	
	body = "{\"value\":" + (String)data;
	body += ",\"stream_id\":";
	body += (String)stream_id;
	body += "}\r\n";
	
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	
	int code = HTTPClient::POST(body);
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::createDroplet(int stream_id, String data) {
	url = "/droplet?stream=";
	url += stream_id;
	
	body = "{\"value\":" + data;
	body += ",\"stream_id\":";
	body += (String)stream_id;
	body += "}\r\n";
	
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	
	int code = HTTPClient::POST(body);
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getStatsToday(int stream_id) {
	url = "/stream/stats/today/" + (String)stream_id;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getStatsFrom(String from, String towards, int stream_id) {
	url = "/stream/stats/range/" + (String)stream_id + "?from=" + from + "&to=" + towards;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

String HttpDatapond::getPayload() {
	return payload;
}

int HttpDatapond::getPond(int pond_id) {
	url = "/pond/" + (String)pond_id;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getPondCount() {
	url = "/pond/count";
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getStream(int stream_id) {
	url = "/stream/"+ (String)stream_id;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getStreamsInPond(int pond_id) {
	url = "/stream?pond="+ (String)pond_id;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getStreamCountInPond(int pond_id) {
	url = "/stream/count?pond="+ (String)pond_id;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

String HttpDatapond::getCookie() {
	return cookie;
}

//Currently unsupported
int HttpDatapond::getCountries() {
	url = "/countries/";
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}
	
int HttpDatapond::getCountries(String query) {
	url = "/countries?startswith=" + query;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}

int HttpDatapond::getTimeZones(int countryCode) {
	url = "/timezones?country=" + (String)countryCode;
	HTTPClient::setReuse(true);
	HTTPClient::begin(pondIPAddress, serverPort, url);
	HTTPClient::addHeader((String)"Cookie", cookie, false);
	int code = HTTPClient::GET();
	payload = HTTPClient::getString();
	return code;
}







