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

// CoAP Arduino library for communicating with Embeddded Adventures datapond server
// Written originally by Embedded Adventures

#include "Arduino.h"
#include "coap-datapond.h"
#include "WiFiUdp.h"
#include "coap-protocol.h"

CoapDatapond::CoapDatapond(const char* ip, int thisport, int remoteport) {
	pondIPAddress = ip;
	localPort = thisport;
	serverPort = remoteport;
}

void CoapDatapond::begin(String user, String pass, uns16 id) {
	CoapProtocol::begin();
	CoapProtocol::setDestination(pondIPAddress, serverPort);
	messageID = id;
	pondUsername = user;
	pondPassword = pass;
	//oap(pondIPAddress);
	//oap(serverPort);	
	
	current_token_id = 0x00;
}

/*	Receives a packet address and extracts the payload from it	*/
void CoapDatapond::collectPayload() {
	uns8* payPtr = packet.getPayloadAddr();
	payload = "";
	if (payPtr == NULL)
		return;
	while (*payPtr != '\0') {
		payload += (char)*payPtr++;
	}
}

/*	Extracts the cookie from payload	*/
void CoapDatapond::collectCookie() {
	int cookieStart = payload.indexOf("n\":\"") + 4;
	int cookieEnd = payload.indexOf("=") + 1;
	cookie = "session=";
	cookie += payload.substring(cookieStart, cookieEnd);
}

/*	Processes the coap queue. Takes the callback functions from main program as args	*/
void CoapDatapond::run() {
	int packetSize = CoapProtocol::parseUDPPacket();
	if (packetSize) {
		int index = CoapProtocol::receivePacket();
	}	
	CoapProtocol::process_rx_queue();
	CoapProtocol::process_tx_queue();
}

/*	Clears both TX and RX queues	*/
void CoapDatapond::emptyQueue() {
	CoapProtocol::clearQueue(RX);
	CoapProtocol::clearQueue(TX);
}


////////////////////////////////////////////////////////
////			Datapond Transactions			 	////
////////////////////////////////////////////////////////

/*	Create login packet and add it to txBuffer. Token_id = 0	*/
int CoapDatapond::login() {
	body = "{\"email\":\"soon@along.com\",\"password\":\"blankevent\"}";
	
	packet.begin();
	packet.addHeader(TYPE_CON, COAP_POST, messageID++);
	packet.addTokens(TOKENID_LENGTH, &current_token_id);
	packet.addOption(OPT_URI_PATH, 4, "user");
	packet.addOption(OPT_URI_PATH, 5, "login");
	packet.addPayload(body.length(), body.c_str());
	
	insertTokenEntry(LOGIN_CODE);
	return CoapProtocol::addToTX(packet.getPacket(), packet.getPacketLength());
}

/*	Create new droplet in stream stream_id	*/
int CoapDatapond::createDroplet(int stream_id, String data) {
	url = "stream=" + (String)stream_id;
	body = "value=" + data;

	packet.begin();
	packet.addHeader(TYPE_CON, COAP_POST, messageID++);
	packet.addTokens(TOKENID_LENGTH, &current_token_id);
	packet.addOption(OPT_URI_PATH, 7, "droplet");
	packet.addOption(OPT_URI_QUERY, url.length(), url.c_str());
	packet.addOption(OPT_URI_QUERY, body.length(), body.c_str());
	packet.addOption(OPT_URI_QUERY, cookie.length(), cookie.c_str());
	
	insertTokenEntry(CREATE_DROPLET);
	return CoapProtocol::addToTX(packet.getPacket(), packet.getPacketLength());
}

/*	Create new droplet in stream stream_id	*/
int CoapDatapond::createDroplet(int stream_id, double data) {
	url = "stream=" + (String)stream_id;
	body = "value=" + (String)data;

	packet.begin();
	packet.addHeader(TYPE_CON, COAP_POST, messageID++);
	packet.addTokens(TOKENID_LENGTH, &current_token_id);
	packet.addOption(OPT_URI_PATH, 7, "droplet");
	packet.addOption(OPT_URI_QUERY, url.length(), url.c_str());
	packet.addOption(OPT_URI_QUERY, body.length(), body.c_str());
	packet.addOption(OPT_URI_QUERY, cookie.length(), cookie.c_str());

	insertTokenEntry(CREATE_DROPLET);
	return CoapProtocol::addToTX(packet.getPacket(), packet.getPacketLength());	
}

/*	Get last created droplet in stream_id	*/
int CoapDatapond::getLastDroplet(int stream_id) {
	url = "stream=" + (String)stream_id;
	
	packet.begin();
	packet.addHeader(TYPE_CON, COAP_GET, messageID++);
	packet.addTokens(TOKENID_LENGTH, &current_token_id);
	packet.addOption(OPT_URI_PATH, 7, "droplet");
	packet.addOption(OPT_URI_PATH, 4, "last");
	packet.addOption(OPT_URI_QUERY, url.length(), url.c_str());
	packet.addOption(OPT_URI_QUERY, cookie.length(), cookie.c_str());
	
	insertTokenEntry(READ_DROPLET);
	return CoapProtocol::addToTX(packet.getPacket(), packet.getPacketLength());
	
}

/*	Get stats of the day from stream stream_id	*/
int CoapDatapond::getStatsToday(int stream_id) {
	url = (String)stream_id;
	
	packet.begin();
	packet.addHeader(TYPE_CON, COAP_GET, messageID++);
	packet.addTokens(TOKENID_LENGTH, &current_token_id);
	packet.addOption(OPT_URI_PATH, 6, "stream");
	packet.addOption(OPT_URI_PATH, 5, "stats");
	packet.addOption(OPT_URI_PATH, 5, "today");
	packet.addOption(OPT_URI_PATH, url.length(), url.c_str());
	packet.addOption(OPT_URI_QUERY, cookie.length(), cookie.c_str());
	
	insertTokenEntry(READ_STREAM);
	return CoapProtocol::addToTX(packet.getPacket(), packet.getPacketLength());
}

int CoapDatapond::getStream(int stream_id) {
	url = (String)stream_id;
	
	packet.begin();
	packet.addHeader(TYPE_CON, COAP_GET, messageID++);
	packet.addTokens(TOKENID_LENGTH, &current_token_id);
	packet.addOption(OPT_URI_PATH, 6, "stream");
	packet.addOption(OPT_URI_PATH, url.length(), url.c_str());
	packet.addOption(OPT_URI_QUERY, cookie.length(), cookie.c_str());
	
	CoapProtocol::addToTX(packet.getPacket(), packet.getPacketLength());
	
	insertTokenEntry(READ_STREAM);
}


////////////////////////////////////////////////////////
////			Callback Functions				 	////
////////////////////////////////////////////////////////

void CoapDatapond::retrievePacket(uns8* pkt, int pktLen) {
	
	bool rStatus = false;
	
	packet.begin();
	packet.copyPacket(pkt, pktLen);
	packet.parsePacket();
	collectPayload();

	if ((packet.getResponseCode() == CODE_CREATED) ||(packet.getResponseCode() == CODE_CONTENT))
		rStatus = true;
	
	for (int i = 0; i < TOKENID_BUFFER_SIZE; i++) {
		printTokenEntry(i);
		//If it's not in use, it doesn't have an entry
		if (!tokenBuffer[i].in_use) 
			continue;
		
		if (tokenBuffer[i].token_id == *packet.getTokens()) {
			switch (tokenBuffer[i].callback_code) {
				case LOGIN_CODE:
					printTokenEntry(i);
					loginHandler(rStatus);
					break;
				case CREATE_DROPLET: 
					printTokenEntry(i);
					createDropletHandler(tokenBuffer[i].token_id, rStatus);
					break;
				case READ_DROPLET:
					printTokenEntry(i);
					readDropletHandler(tokenBuffer[i].token_id, rStatus, payload);
					break;
				case READ_STREAM:
					printTokenEntry(i);
					readStreamHandler(tokenBuffer[i].token_id, rStatus, payload);
					break;
			}
			tokenBuffer[i].in_use = false;
			printTokenEntry(i);
			break;
		}
	}	
}

void CoapDatapond::loginHandler(bool rstatus) {
	if (rstatus) {
		collectCookie();
		_loggedIn(true);
	}
	else {
		_loggedIn(false);
	}
}

void CoapDatapond::createDropletHandler(uns8 tkn, bool status) {
	if (_createDropletFn != NULL) {
		_createDropletFn(tkn, status);
	}
}

void CoapDatapond::readDropletHandler(uns8 tkn, bool status, String data) {
	if (_readDropletFn != NULL) {
		_readDropletFn(tkn, status, data);
	}
}

void CoapDatapond::createStreamhandler(uns8 tkn, bool status) {
	if (_createStreamFn != NULL) {
		_createStreamFn(tkn, status);
	}
}

void CoapDatapond::readStreamHandler(uns8 tkn, bool status, String data) {
	if (_readStreamFn != NULL) {
		_readStreamFn(tkn, status, data);
	}
}

void CoapDatapond::txSuccessHandler(uns8* pkt, int pktLen) {
	if (_txSuccess != NULL) {
		CoapProtocol::txSuccessHandler(pkt, pktLen);
	}
	else {
		retrievePacket(pkt, pktLen);
	}
}

void CoapDatapond::txFailureHandler(uns8* pkt, int pktLen) {
	if (_txSuccess != NULL) {
		CoapProtocol::txFailureHandler(pkt, pktLen);
	}
	else {
		packet.begin();
		packet.copyPacket(pkt, pktLen);
		packet.parsePacket();
		//Remove entry from token_buffer
		for (int i = 0; i < TOKENID_BUFFER_SIZE; i++) {
			if (tokenBuffer[i].token_id == *packet.getTokens()) {
				tokenBuffer[i].in_use = false;
				break;
			}
		}
	}
}

void CoapDatapond::availablePacketHandler(uns8* pkt, int pktLen) {
	if (_txSuccess != NULL) {
		CoapProtocol::availablePacketHandler(pkt, pktLen);
	}
	else {
		retrievePacket(pkt, pktLen);
	}
}

void CoapDatapond::responseTimeoutHandler(uns8* pkt, int pktLen) {
	if (_txSuccess != NULL) {
		CoapProtocol::responseTimeoutHandler(pkt, pktLen);
	}
	else {
		retrievePacket(pkt, pktLen);
	}
}
	
	
////////////////////////////////////////////////////////////
////			Set Callback Functions				 	////
////////////////////////////////////////////////////////////	
	
void CoapDatapond::setLoginHandler(login_fnPtr loggedIn) {
	_loggedIn = loggedIn;
}	
	
void CoapDatapond::setCreateDropletHandler(create_request_ptr handler) {
	_createDropletFn = handler;
}

void CoapDatapond::setReadDropletHandler(read_request_ptr handler) {
	_readDropletFn = handler;
}

void CoapDatapond::setCreateStreamHandler(create_request_ptr handler) {
	_createStreamFn = handler;
}

void CoapDatapond::setReadStreamHandler(read_request_ptr handler) {
	_readStreamFn = handler;
}

void CoapDatapond::setHandlers(create_request_ptr handler1, 
					read_request_ptr handler2, create_request_ptr handler3,
					read_request_ptr handler4) {
						
	_createDropletFn = handler1;
	_readDropletFn = handler2;
	_createStreamFn = handler3;
	_readStreamFn = handler4;
}


void CoapDatapond::setProtocolHandlers(packetReturn_callback packetAvailable, 
					packetReturn_callback txSuccess, packetReturn_callback txFailure,
					packetReturn_callback responseTimeout)
{
	_txSuccess = txSuccess;
	_txFailure = txFailure;
	_packetAvailable = packetAvailable;
	_responseTimeout = responseTimeout;				
}


////////////////////////////////////////////////////////////
////			Data Accessor Functions				 	////
////////////////////////////////////////////////////////////	

/*	Returns the current payload data  */
String CoapDatapond::getPayload() {
	return this->payload;
}


String CoapDatapond::getCookie() {
	return cookie;
}

/*	Returns address of first byte in packet	*/
uns8* CoapDatapond::getPacket() {
	return packet.getPacket();
}

/*	Returns packet length	*/
int CoapDatapond::getPacketLength() {
	return packet.getPacketLength();
}


////////////////////////////////////////////////////////////
////			Token Buffer Functions				 	////
////////////////////////////////////////////////////////////	
	
void CoapDatapond::insertTokenEntry(uns8 callbackCode) {
	int tokenCursor = 0;
	for (int tokenCursor = 0; tokenCursor < TOKENID_BUFFER_SIZE; tokenCursor++) {
		if (tokenBuffer[tokenCursor].in_use)
			continue;
		else {
			tokenBuffer[tokenCursor].token_id = current_token_id;
			tokenBuffer[tokenCursor].callback_code = callbackCode;
			tokenBuffer[tokenCursor].in_use = true;
			break;
		}
		printTokenEntry(tokenCursor);
	}

	if (current_token_id <= 0xFF)
		current_token_id++;
	else
		current_token_id = 0x01;
}    

void CoapDatapond::markEntryResponse(uns8 tokenID, uns8 response) {
	for (int i = 0; i < TOKENID_BUFFER_SIZE; i++) {
		if (tokenID == tokenBuffer[i].token_id) {
			tokenBuffer[i].response_code = response;
			break;
		}
	}
}


