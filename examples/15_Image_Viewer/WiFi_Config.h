/*
  WiFi Configuration

  WICHTIG: Diese Datei enthält deine WiFi-Zugangsdaten!
  Füge sie zu .gitignore hinzu wenn du den Code veröffentlichst!
*/

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// WiFi Credentials - HIER DEINE DATEN EINTRAGEN!
const char* WIFI_SSID = "DEIN_WIFI_NAME";
const char* WIFI_PASSWORD = "DEIN_WIFI_PASSWORT";

// Alternative: Mehrere WiFi-Netzwerke (automatische Auswahl)
/*
struct WiFiCredentials {
  const char* ssid;
  const char* password;
};

WiFiCredentials wifiNetworks[] = {
  {"Home_WiFi", "password123"},
  {"Office_WiFi", "office456"},
  {"Mobile_Hotspot", "mobile789"},
};

const int WIFI_NETWORK_COUNT = sizeof(wifiNetworks) / sizeof(WiFiCredentials);
*/

// Timeout-Einstellungen
const int WIFI_CONNECT_TIMEOUT = 15000;  // 15 Sekunden
const int HTTP_TIMEOUT = 10000;          // 10 Sekunden

#endif
