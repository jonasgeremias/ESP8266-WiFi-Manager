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

#ifdef SENSORES
// Definições para o sensor DHT11 ----------
#include <dht.h>
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
#define DHT11_GPIO 16

// Sensor ultrassonico ---------------------
#include <ultrasonic.h>
#define MAX_DISTANCE_CM 500 // 5m max
#define TRIGGER_GPIO 4
#define ECHO_GPIO 5

int sensores[3] = {0, 0, 0}; // temp, hum, ultra

static const char *LOG_DHT = "DHT";
static const char *LOG_ULTRASONIC = "ULTRASONIC";
#endif

static void atualiza_pacote_tempo_real(bool realtime)
{
    bzero(json_tempo_real, sizeof(json_tempo_real));

    char *p = (char *)&json_tempo_real;
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
        p += sprintf(p, "\"Contagem\":%d,", contagem_teste);
#ifdef SENSORES
        p += sprintf(p, "\"Temp\":%d,", sensores[0]);
        p += sprintf(p, "\"Hum\":%d,", sensores[1]);
        p += sprintf(p, "\"Ultra\":%d,", sensores[2]);
#endif
        for (int i = 0; i < 10; i++)
        {
            p += sprintf(p, "\"TESTE%d\":\"TESTE-%d\",", i, i);
        }
        p += sprintf(p - 1, "}");
    }
}
#ifdef SENSORES

static void ultrasonic_task(void *pvParamters)
{
    ESP_LOGI(LOG_ULTRASONIC, "Init");
    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO};

    ultrasonic_init(&sensor);

    // Estabilizar o sensor após ligar o pullup
    vTaskDelay(50 / portTICK_PERIOD_MS);
    //uint32_t req = 0;
    char res[128];
    uint32_t distance;
    while (true)
    {
        esp_err_t ret = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
        if (ret != ESP_OK)
        {
            switch (ret)
            {
            case ESP_ERR_ULTRASONIC_PING:
                sprintf(res, "%s", "Cannot ping (device is in invalid state)\r\n");
                ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                break;
            case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                sprintf(res, "%s", "Ping timeout (no device found)\r\n");
                ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                break;
            case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                sprintf(res, "%s", "Echo timeout (i.e. distance too big)\r\n");
                ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                break;
            default:
                sprintf(res, "Erro não tratado:%u\r\n", ret);
                ESP_LOGE(LOG_ULTRASONIC, "%s", res);
            }
            sensores[2] = 0;
        }
        else
        {
            sprintf(res, "Distance: %d cm\r\n", distance);
            ESP_LOGE(LOG_ULTRASONIC, "%s", res);
            sensores[2] = distance;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void dht_task(void *pvParameters)
{
    ESP_LOGI(LOG_DHT, "Init");
    int16_t temperature = 0;
    int16_t humidity = 0;

    // DHT sensors that come mounted on a PCB generally have
    // pull-up resistors on the data pin.  It is recommended
    // to provide an external pull-up resistor otherwise...
    gpio_set_pull_mode(DHT11_GPIO, GPIO_PULLUP_ONLY);

    // Estabilizar o sensor após ligar o pullup
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // uint32_t req = 0;
    char res[128];

    while (1)
    {
        if (dht_read_data(sensor_type, DHT11_GPIO, &humidity, &temperature) == ESP_OK)
        {
            sprintf(res, "Humidity: %d%% Temp: %d°C\r\n", humidity / 10, temperature / 10);
            ESP_LOGI(LOG_DHT, "%s", res);
            sensores[0] = temperature / 10;
            sensores[1] = humidity / 10;
        }
        else
        {
            sprintf(res, "Could not read data from sensor dht\r\n");
            ESP_LOGE(LOG_DHT, "%s", res);
            sensores[0] = -100;
            sensores[1] = 0;
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

#endif
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

#ifdef SENSORES
    xTaskCreate(&dht_task, "dht_task", configMINIMAL_STACK_SIZE * 2, NULL, 5, NULL);
    xTaskCreate(&ultrasonic_task, "ultrasonic_task", configMINIMAL_STACK_SIZE * 2, NULL, 5, NULL);
#endif

    wifi_app_init(); // Serviço wi-fi
    xTaskResumeAll();
}
