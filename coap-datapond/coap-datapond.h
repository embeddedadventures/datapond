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

#ifndef __coap-datapond_h
#define __coap-datapond_h

#include "ESP8266WiFi.h"
#include "coap-packet.h"
#include "coap-protocol.h"
#include "WiFiUdp.h"

#define		TOKENID_LENGTH		1
#define		TOKENID_BUFFER_SIZE	10

//Callback codes
#define		LOGIN_CODE			0x01
#define		CREATE_DROPLET		0x10
#define		READ_DROPLET		0x20
#define		UPDATE_DROPLET		0x30
#define		DELETE_DROPLET		0x40
#define		CREATE_STREAM		0x11
#define		READ_STREAM			0x21
#define		UPDATE_STREAM		0x31
#define		DELETE_STREAM		0x41
#define		CREATE_POND			0x12
#define		READ_POND			0x22
#define		UPDATE_POND			0x32
#define		DELETE_POND			0x42			

typedef	void (*login_fnPtr)(bool i);	
typedef void (*create_request_ptr)(uns8 tkn, bool status);
typedef void (*read_request_ptr)(uns8 tkn, bool status, String data);


typedef struct {
	uns8 	token_id = 0;
	uns8	callback_code = 0;
	uns8	response_code = 0x00;
	bool	in_use = false;
} token_buffer_struct; 

class CoapDatapond : public CoapProtocol{
	
private:
	CoapPacket 			packet;
	
	String				pondUsername;
	String				pondPassword;
	const char*			pondIPAddress;	
	int					serverPort;
	int					localPort;
	
	uns16				messageID;
	String				cookie = "";
	String				payload;
	String				body;
	String				url;
	
	//Token buffer variables
	token_buffer_struct	tokenBuffer[TOKENID_BUFFER_SIZE];
	uns8				current_token_id;
	
	//Token buffer functions
	void	insertTokenEntry(uns8 callbackCode);
	void	markEntryResponse(uns8 tokenID, uns8 response);
	
	//Packet info collection
	void	collectCookie();
	void	collectPayload();
	
	//Callback handling
	void 	retrievePacket(uns8* pkt, int pktLen);
	void 	confirmFailed(uns8* packet, int packetLength);
	void 	confirmSuccess(uns16 id);
	void 	responseTimedOut(uns8* packet, int packetLength);
	
	//Datapond callback handling
	inline void	loginHandler(bool rstatus);
	inline void	createDropletHandler(uns8 tkn, bool status);
	inline void	readDropletHandler(uns8 tkn, bool status, String data);
	inline void	createStreamhandler(uns8 tkn, bool status);
	inline void	readStreamHandler(uns8 tkn, bool status, String data);
	
	//Coap Protocol Callback functions
	packetReturn_callback	_txSuccess = NULL;
	packetReturn_callback 	_txFailure = NULL;
	packetReturn_callback 	_packetAvailable = NULL;
	packetReturn_callback 	_responseTimeout = NULL;
	
	//Datapond callback functions
	login_fnPtr				_loggedIn;
	create_request_ptr		_createDropletFn = NULL;
	create_request_ptr		_createStreamFn = NULL;
	read_request_ptr		_readDropletFn = NULL;
	read_request_ptr		_readStreamFn = NULL;

public:
	CoapDatapond(const char* ip, int thisport, int remoteport);
	void	begin(String user, String pass, uns16 id);
	
	//Internal admin functions
	void	run();
	void	emptyQueue();
	void	processed(int x);

	//Packet data accessors
	String	getPayload();
	uns8*	getPacket();
	int		getPacketLength();
	String	getCookie();
	
	//Datapond server transactions
	int	login();
	int	createDroplet(int stream_id, double data);
	int	createDroplet(int stream_id, String data);
	int	getLastDroplet(int stream_id);
	int	getStatsToday(int stream_id);
	int	getStream(int stream_id);

	//Callback sets/handlers
	void	setProtocolHandlers(packetReturn_callback packetAvailable = NULL, 
							packetReturn_callback txSuccess = NULL,
							packetReturn_callback txFailure = NULL,
							packetReturn_callback responseTimeout = NULL);
	void	txSuccessHandler(uns8* pkt, int pktLen);
	void	txFailureHandler(uns8* pkt, int pktLen);
	void	availablePacketHandler(uns8* pkt, int pktLen);
	void	responseTimeoutHandler(uns8* pkt, int pktLen);
	
	void	setLoginHandler(login_fnPtr loggedIn);
	void	setCreateDropletHandler(create_request_ptr handler);
	void	setReadDropletHandler(read_request_ptr handler);
	void	setCreateStreamHandler(create_request_ptr handler);
	void	setReadStreamHandler(read_request_ptr handler);
	void	setHandlers(create_request_ptr handler1, 
					read_request_ptr handler2, create_request_ptr handler3,
					read_request_ptr handler4);
		
};


#endif