#include "espaxa.h"

CustomAXA2RemoteUART::CustomAXA2RemoteUART(UARTComponent *parent) : UARTDevice(parent) {}

void CustomAXA2RemoteUART::setup() {
    lastread = 0;
    ESP_LOGCONFIG("espaxa", "Setting up AXA UART...");
}

void CustomAXA2RemoteUART::loop() {
    unsigned long now = millis();

    if (now - lastread > DELAY_MS || lastread == 0) {
        lastread = now;
        while (available()) { // empty UART input buffer
            dummy = read();
        }
        write_str("\r\n");  // send dummy character
        delay(100);
        write_str("STATUS\r\n");  // ask status
        delay(100);
        if (available()) {
            i = 0;
            while (available()) {
                buff[i++] = read(); // get return string
                if (i >= 29) {
                    break;
                }
            }
            buff[i] = 0;
            axa_status = 100 * (buff[0] - '0') + 10 * (buff[1] - '0') + (buff[2] - '0');  // calculate status code from first three digits
            i = i - 2;
            buff[i] = 0; // remove 0D 0A linebreak
            ESP_LOGD("espaxa", "%d %s-> %d", i, buff, axa_status);
            if (axa_status == AXA_CLOSED) {
                axa_window->publish_state(COVER_CLOSED); // only AXA_CLOSED will return CLOSED
            } else if (axa_status == AXA_OPENED) {
                axa_window->publish_state(COVER_OPEN);  // only AXA_OPENED will return OPEN
            }
            // no messages will be published on any other state (ie. 502 Command not implemented)
        }
    }
}