#ifndef __APP_SYSTEM_H
#define __APP_SYSTEM_H

#define PORRSTF_MASK  (1ul<<27)
#define SFTRST_MASK   (1ul<<28)
#define IWDGRST_MASK  (1ul<<29)
#define WWDGRST_MASK  (1ul<<30)
#define LPWRRSTF_MASK (1ul<<31)

void app_system_LedOn(void);
void app_system_CheckID(void);
void app_system_UpdataBle(void);
void app_system_NetLedToggle(void);

void connect_again(void);
void app_system_Start(void);
void app_system_NetPublic(void);
void app_system_MqttConnect(uint8_t socket);

#endif

