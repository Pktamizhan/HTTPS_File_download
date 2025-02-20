#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

// Replace with your Wi-Fi credentials
const char* ssid = "OPPO A55";
const char* password = "pk123456789";

// Sample file URL for testing (Replace with your own file URL if needed)
const char* fileUrl = "https://raw.githubusercontent.com/seladb/PcapPlusPlus/master/LICENSE";

// File path in SPIFFS
const char* filePath = "/downloaded_file.txt";

// Function to download a file
bool downloadFile(const char* url, const char* path) {
    HTTPClient http;
    http.begin(url); // Specify the URL

    int httpCode = http.GET(); // Send HTTP GET request
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    // Open the file in SPIFFS for writing
    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        http.end();
        return false;
    }

    // Get the length of the file
    int contentLength = http.getSize();
    Serial.printf("File size: %d bytes\n", contentLength);

    // Create a buffer for reading
    uint8_t buffer[1024];
    int totalBytes = 0;
    unsigned long startTime = millis();

    // Read the file in chunks and write to SPIFFS
    WiFiClient* stream = http.getStreamPtr();
    while (http.connected() && totalBytes < contentLength) {
        size_t bytesRead = stream->readBytes(buffer, sizeof(buffer));
        if (bytesRead > 0) {
            file.write(buffer, bytesRead);
            totalBytes += bytesRead;
            Serial.printf("Downloaded %d bytes (%.2f%%)\n", totalBytes, (totalBytes * 100.0) / contentLength);
        }
    }

    // Close the file and HTTP connection
    file.close();
    http.end();

    unsigned long endTime = millis();
    float speed = (totalBytes / 1024.0) / ((endTime - startTime) / 1000.0); // Speed in KBps
    Serial.printf("Download speed: %.2f KBps\n", speed);

    if (totalBytes == contentLength) {
        Serial.println("Download complete");
        return true;
    } else {
        Serial.println("Download incomplete");
        return false;
    }
}

// Function to read the downloaded file
void readFile(const char* path) {
    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.println("File content:");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi");

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");

    // Download the file
    if (downloadFile(fileUrl, filePath)) {
        Serial.println("File downloaded successfully. Verifying...");
        readFile(filePath); // Read and print the file content
    } else {
        Serial.println("Failed to download file");
    }
}

void loop() {
    // Nothing to do here
}