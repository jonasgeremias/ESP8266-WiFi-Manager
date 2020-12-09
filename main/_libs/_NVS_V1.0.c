#include "nvs_flash.h"
#include "nvs.h"

#define memoria "nvs"

#ifdef DEBUG_NVS
const char *TAG_NVS = "NVS";
#endif

// Variáveis de controle
typedef uint32_t nvs_handle_t;
static SemaphoreHandle_t mutex_nvs;

esp_err_t app_nvs_init()
{
   mutex_nvs = xSemaphoreCreateMutex();
   esp_err_t err;

   // Configurações da aplicação ----------------------------------------------
   err = nvs_flash_init();
   ESP_ERROR_CHECK(err);

   return err;
}

esp_err_t nvs_app_read_config_wifi()
{
   xSemaphoreTake(mutex_nvs, portMAX_DELAY);
   nvs_handle_t nvs_handle;
   esp_err_t err;
   err = nvs_open(memoria, NVS_READONLY, &nvs_handle); // Abrir
   if (err != ESP_OK)
      goto fim;

   size_t size = sizeof(login_wifi);
   err = nvs_get_blob(nvs_handle, "login_wifi", &login_wifi, &size); // Ler

#ifdef DEBUG_NVS
   for (int i = 0; i < MAX_LOGIN_WIFI; i++)
   {
      printf("NVI->ssid %i:'%s',pass:'%s',IP:%d.%d.%d.%d\r\n",
             i, login_wifi[i].ssid, login_wifi[i].pass,
             MAKE8(login_wifi[i].net_ip4.ip, 0),
             MAKE8(login_wifi[i].net_ip4.ip, 1),
             MAKE8(login_wifi[i].net_ip4.ip, 2),
             MAKE8(login_wifi[i].net_ip4.ip, 3));
   }
#endif

fim:
   nvs_close(nvs_handle); // Fechar
   xSemaphoreGive(mutex_nvs);
   return err;
}

esp_err_t nvs_app_write_config_module()
{
   xSemaphoreTake(mutex_nvs, portMAX_DELAY);
   nvs_handle_t nvs_handle;
   esp_err_t err;

   err = nvs_open(memoria, NVS_READWRITE, &nvs_handle); // Abrir
   if (err != ESP_OK)
      goto fim;

   err = nvs_set_blob(nvs_handle, "modulo", &modulo, sizeof(modulo)); // seta valores
   if (err != ESP_OK)
      goto fim;

   err = nvs_commit(nvs_handle); // Publica

fim:
   nvs_close(nvs_handle); // Fechar
   xSemaphoreGive(mutex_nvs);
   return err;
}

esp_err_t nvs_app_write_config_wifi()
{
   xSemaphoreTake(mutex_nvs, portMAX_DELAY);
   nvs_handle_t nvs_handle;
   esp_err_t err;

   err = nvs_open(memoria, NVS_READWRITE, &nvs_handle); // Abrir
   if (err != ESP_OK)
      goto fim;

   err = nvs_set_blob(nvs_handle, "login_wifi", &login_wifi, sizeof(login_wifi));
   if (err != ESP_OK)
      goto fim;

   err = nvs_commit(nvs_handle); // Garante que gravou corretamente*/

fim:
   nvs_close(nvs_handle); // Fechar
   xSemaphoreGive(mutex_nvs);
   return err;
}

esp_err_t carrega_configuracao_default()
{
   memset(&login_wifi, 0, sizeof(login_wifi_t));
   sprintf((char *)login_wifi[0].ssid, "WiFi-Manager");
   sprintf((char *)login_wifi[0].pass, "teste123");
   login_wifi[0].ip4_fix = 1;
   login_wifi[0].net_ip4.ip = MAKE32(192, 168, 1, 230);
   login_wifi[0].net_ip4.gw = MAKE32(192, 168, 1, 1);
   login_wifi[0].net_ip4.netmask = MAKE32(255, 255, 255, 0);
   login_wifi[0].net_ip4.dns1 = MAKE32(8, 8, 8, 8);
   login_wifi[0].net_ip4.dns2 = MAKE32(4, 4, 4, 4);

   // Configura valores padrão
   sprintf(modulo.nome, "%s", "WiFi-Manager-AP");
   sprintf(modulo.passwifi, "%s", "wifimanager");
   sprintf(modulo.passconfig, "%s", "1234");
   sprintf(modulo.versao_sw, "%s", "1.0.00");

   xSemaphoreTake(mutex_nvs, portMAX_DELAY);
   nvs_handle_t nvs_handle;
   esp_err_t err;

   err = nvs_open(memoria, NVS_READWRITE, &nvs_handle); // Abrir
   if (err != ESP_OK)
      goto fim;

   err = nvs_set_blob(nvs_handle, "modulo", &modulo, sizeof(modulo)); // seta valores modulç
#ifdef DEBUG_NVS
   if (err == ESP_OK)
      printf("Gravou as configurações do módulo.\r\n");
   else
      printf("Erro as gravar as configurações do módulo.\r\n");
#endif

   err = nvs_set_blob(nvs_handle, "login_wifi", &login_wifi, sizeof(login_wifi)); // Seta valores wifi
#ifdef DEBUG_NVS
   if (err == ESP_OK)
      printf("Gravou as configurações do Wi-Fi.\r\n");
   else
      printf("Erro as gravar as configurações do Wi-Fi.\r\n");
#endif

   err = nvs_commit(nvs_handle); // Garante que gravou corretamente*/
#ifdef DEBUG_NVS
   if (err == ESP_OK)
      printf("Gravado com sucesso\r\n");
   else
      printf("Erro ao gravar na NVS.\r\n");
#endif
fim:
   nvs_close(nvs_handle); // Fechar
   xSemaphoreGive(mutex_nvs);
   return err;
}

// Leitura NVS ----------------------------------------------------------------
esp_err_t nvs_app_read_module()
{
   xSemaphoreTake(mutex_nvs, portMAX_DELAY);
   nvs_handle_t nvs_handle;
   esp_err_t err;

   err = nvs_open(memoria, NVS_READONLY, &nvs_handle); // Abrir
   if (err != ESP_OK)
      goto fim;

   size_t size = sizeof(modulo);
   err = nvs_get_blob(nvs_handle, "modulo", &modulo, &size);

#ifdef DEBUG_NVS
   printf("%s\r\n", modulo.passwifi);
   printf("%s\r\n", modulo.nome);
   printf("%s\r\n", modulo.passconfig);
   printf("%s\r\n", modulo.versao_sw);
#endif

fim:
   nvs_close(nvs_handle); // Fechar
   xSemaphoreGive(mutex_nvs);

   // Reset inicial -----------------------------------------------------------
   if (strstr(modulo.versao_sw, "1.0.00") == NULL)
   { // Pendente alterar
      err = carrega_configuracao_default();
   }

   return err;
}