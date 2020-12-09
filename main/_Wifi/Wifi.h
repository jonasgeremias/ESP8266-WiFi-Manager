#ifdef DEBUG_WIFI
const char *LOG_WIFI = "APP WIFI";
#endif

#define WIFI_QUANTIDADE_DE_PROCURAS MAX_LOGIN_WIFI // Procura essa quantidade de vezes antes de colcoar o timeout abaixo 
#define WIFI_TEMPO_RETORNO_DE_PROCURA 60000 // 1 min

// Protótipos de funções ----------------------------------------------------

// Trata Os eventos do wifi.
static esp_err_t event_handler(void *ctx, system_event_t *event);

// Configura o ponto de acesso.
static esp_err_t wifi_ap_config();

// Configura o IP do ponto de acesso como fixo.
static void config_tcpip_ap_fix();

// Configura o login e senha para conectar o ESP (STA) em um Wi-Fi disponivel.
static esp_err_t wifi_sta_config(int id);

// Faz a procura de Wifis e compara se existe um Wi-Fi conhecido para conectar.
static int wifi_app_start_scan();

// Inicia o modulo Wi-Fi do ESP no modo AP-STA.
static void wifi_init();

// Função para desconectar do Wi-Fi e desabilitar a reconexão.
static void desconectar_do_wifi();

// Função deve ser adicionada ao um timer de 1ms para contar corretamente.
static void wifi_timer_ms();

// Função que verifique periodicamente os status do wifi e tarfas relacionadas.
static void wifi_app_control(void *pvParameters);

// Inicia tarefa de cntole do Wi-Fi .
static void wifi_app_init();

// JSONs
static char *gera_json_scanwifi();
static cJSON *gera_json_configwifi();
