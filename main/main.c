// Bibliotecas ----------------------------------------------------------------
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/gpio.h"
#include <cJSON.h>
#include "freertos/semphr.h"
#include <esp_http_server.h>

#define DEBUG // Pendente
#ifdef DEBUG
#define DEBUG_MAIN
#define DEBUG_NVS
#include "esp_log.h"
#endif
#ifdef DEBUG_MAIN
const char *LOG_APP = "Main";
#endif

// Definições da aplicação ----------------------------------------------------
#include "main.h"

static void atualiza_pacote_tempo_real(bool realtime)
{
    bzero(json_tempo_real, sizeof(json_tempo_real));

    char *p = (char *)& json_tempo_real;
    if (!realtime)
    {   
        // Configuração
        p += sprintf(p, "{");
        p += sprintf(p, "\"mac\":\"%02X-%02X-%02X-%02X-%02X-%02X\",", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
        p += sprintf(p, "\"vsw\":\"%s\",", modulo.versao_sw);
        p += sprintf(p, "\"key\":\"%s\",", modulo.key);
        p += sprintf(p, "\"nome\":\"%s\"", modulo.nome);
        p += sprintf(p, "}");
    }
    else
    {
        // Tempo real
        p += sprintf(p, "{");
        p += sprintf(p, "\"Contagem\":%d,",contagem_teste);
        
        for (int i = 0; i < 10; i++)
        {
            p += sprintf(p, "\"TESTE%d\":\"TESTE-%d\",",i,i);
        }
        p += sprintf(p-1, "}");

    }
}

// Adicionar código da aplicação
static void app_controller(void *pvParameters)
{
#ifdef DEBUG_MAIN
    ESP_LOGW(LOG_APP, "Init app_controller");
#endif
    int count = 0;
    while (1)
    {
        controle_led_status(LED_PISCA_LENTO);

        if (++count >= 100)
        {
            ESP_LOGI(LOG_APP, "App run-> contagem = %d", ++contagem_teste);
            count = 0;

        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void inicia_configuracoes_aplicacao()
{
    // Configura LEd Status como saída
    // Configura o timer.
    init_timer_us(APP_COUNT_TIMER_MS);
    led_status_configurar_pinos(); // Configura pinos do LED bicolor
    controle_led_status(LED_LIGADO);
    app_event_group = xEventGroupCreate();
    mutex_pacote_tempo_real = xSemaphoreCreateMutex();

    // Leitura das variáveis da memória NVS embarcada
    memset(&login_wifi, 0, sizeof(login_wifi));
    nvs_app_read_module();
    nvs_app_read_config_wifi();

    // Monta nome do wifi do AP, tratamento pois nao te configuração na primeira inicialização.
    if (strlen((char *)modulo.nome) >= 4) // Proteção apenas
    {                                     // nome do Wi-Fi deve ser maior que 3.
        sprintf(wifi_ap.ssid, "%s", (char *)modulo.nome);
        sprintf(wifi_ap.pass, "%s", modulo.passwifi);
    }
    else
    { // Se os dados não são válidos
        sprintf(wifi_ap.ssid, "WiFi-Manager");
        sprintf(wifi_ap.pass, "%s", "teste123");
    }
}

// Timer ----------------------------------------------------------------------
void timer_isr_app()
{
    led_status_timer();
    wifi_timer_ms();
}

void app_main()
{
    app_nvs_init();
    ESP_LOGI(LOG_APP, "Init app.");
    inicia_configuracoes_aplicacao();

    vTaskSuspendAll();
    xTaskCreate(app_controller, "app_controller", 15000, NULL, 1, &taskhandle_app_controller);
    wifi_app_init(); // Serviço wi-fi
    xTaskResumeAll();
}
