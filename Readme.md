
Add your WiFi SSID/password and the SmartVita API token to `include/secrets.h` and do not commit that file (already in gitignore).

```
#pragma once
const char WIFI_SSID[] = "YOUR_WIFI_SSID";
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
const char ACCESS_TOKEN[] = "YOUR_SMARTVITA_ACCESS_TOKEN";
```

# Known Errors
## No Console logs
Restart PC