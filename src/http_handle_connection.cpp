#include "http_handle_connection.h"


boolean connectToHttpServer(HTTPClient& httpClient, const IPAddress& ip, int port, const String& path) {
    String url = "http://";
    url += ip.toString();
    url += ":";
    url += String(port);
    url += path;

    Serial.print("Connecting to HTTP server: " + url + " ... \n");

    if (!httpClient.begin(url)) {
        Serial.println("Failed to connect");
        return false;
    }
    else {
        Serial.println("Connected to HTTP server");
        return true;
    }
}


void closeConnectionToHttpServer(HTTPClient& httpClient) {
    httpClient.end();
    delay(1000);
}

int sendHttpPost(HTTPClient& httpClient, const String& payload)
{
    String data = "message=" + payload;
    httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
    httpClient.addHeader("Content-Length", String(data.length()));

    int httpCode = httpClient.POST(data);

    if (httpCode == HTTP_CODE_OK) {
        String response = httpClient.getString();
        return httpCode;
        
    } else {
        Serial.println("Failed to send data");
        return -1;
    }

}


