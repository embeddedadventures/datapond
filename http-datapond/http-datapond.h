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

#ifndef __HTTP-DATAPOND_h
#define __HTTP-DATAPOND_h
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"


class HttpDatapond : public HTTPClient{
	private:
		int 			serverPort;
		const char*		pondIPAddress;
		
		String	cookie;
		String	body;
		String	url;
		String 	payload;
		
	public:
		HttpDatapond(const char* ip, int port);
		
		int 	login(const char* username, const char* password);
		void	collectCookie();
		String	getPayload();
		String	getCookie();
		int		getLastDroplet(int stream_id);
		int 	getStatsToday(int stream_id);
		int		getStatsFrom(String from, String towards, int stream_id);
		int		createDroplet(int stream_id, double data);
		int		createDroplet(int stream_id, String data);
		int		getPond(int pond_id);
		int		getPondCount();
		int		getStream(int stream_id);
		int		getStreamsInPond(int pond_id);
		int		getStreamCountInPond(int pond_id);
		
		//Currently Unsupported
		int		getCountries();
		int		getCountries(String query);
		int		getTimeZones(int countryCode);
};

#endif