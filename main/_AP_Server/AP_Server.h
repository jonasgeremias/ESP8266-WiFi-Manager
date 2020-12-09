#ifdef DEBUG_AP_SERVER
const char *LOG_SERVER = "SERVER";
#endif

// Tipos de retornos ao receber um post
enum
{
    CONFIG_OK = 0,
    CONFIG_ERROR_PASS = -1,
    CONFIG_ERROR_NAME = -2,
    CONFIG_MEMORY_ERROR = -3,
    CONFIG_ERROR_PASS_IN = -4,
    CONFIG_ERROR_CONFIG_IP = -5,
    CONFIG_ERROR_DATA_INVALID = -6
};

httpd_handle_t server = NULL;

// Carrega os arquivos da memoria do ESP32
#define ARRAY_SIZE_OF(a) (sizeof(a) / sizeof(a[0]))

extern const unsigned char index_html_start[] asm("_binary_index_html_start");
extern const unsigned char index_html_end[] asm("_binary_index_html_end");
// Style and frameworks
extern const unsigned char styles_css_start[] asm("_binary_styles_css_start");
extern const unsigned char styles_css_end[] asm("_binary_styles_css_end");
extern const unsigned char mini_css_start[] asm("_binary_mini_css_start");
extern const unsigned char mini_css_end[] asm("_binary_mini_css_end");
extern const unsigned char favicon_png_start[] asm("_binary_favicon_png_start");
extern const unsigned char favicon_png_end[] asm("_binary_favicon_png_end");
// Tela configuração do Wi-Fi
extern const unsigned char configwifi_html_start[] asm("_binary_configwifi_html_start");
extern const unsigned char configwifi_html_end[] asm("_binary_configwifi_html_end");
extern const unsigned char configwifi_js_start[] asm("_binary_configwifi_js_start");
extern const unsigned char configwifi_js_end[] asm("_binary_configwifi_js_end");
// Tela configuração do módulo
extern const unsigned char configmodulo_html_start[] asm("_binary_configmodulo_html_start");
extern const unsigned char configmodulo_html_end[] asm("_binary_configmodulo_html_end");
extern const unsigned char configmodulo_js_start[] asm("_binary_configmodulo_js_start");
extern const unsigned char configmodulo_js_end[] asm("_binary_configmodulo_js_end");
// Tela realtime
extern const unsigned char realtime_js_start[] asm("_binary_realtime_js_start");
extern const unsigned char realtime_js_end[] asm("_binary_realtime_js_end");
extern const unsigned char realtime_html_start[] asm("_binary_realtime_html_start");
extern const unsigned char realtime_html_end[] asm("_binary_realtime_html_end");
// Arquivo Manifest
extern const unsigned char manifest_json_start[] asm("_binary_manifest_json_start");
extern const unsigned char manifest_json_end[] asm("_binary_manifest_json_end");
// imagens de potencia de Wi-Fi
extern const unsigned char iconwifi_lg_png_start[] asm("_binary_iconwifi_lg_png_start");
extern const unsigned char iconwifi_lg_png_end[] asm("_binary_iconwifi_lg_png_end");
extern const unsigned char iconwifi_md_png_start[] asm("_binary_iconwifi_md_png_start");
extern const unsigned char iconwifi_md_png_end[] asm("_binary_iconwifi_md_png_end");
extern const unsigned char iconwifi_sm_png_start[] asm("_binary_iconwifi_sm_png_start");
extern const unsigned char iconwifi_sm_png_end[] asm("_binary_iconwifi_sm_png_end");

const char * restart_code_reload = 
"<html><head><script type='text/javascript'>\
var t = 15;function timedCount(){if (t > 0){document.getElementById('txt').innerText = 'Aguarde o módulo reiniciar. Faltam: ' + t + ' segundos.';\
setTimeout('timedCount()', 1000);t = t - 1;}else location.href = './';}</script></head><body onload='timedCount()'> <h1 id='txt'></h1></body></html>";

// Strutura para verifiar arquivos estáticos
typedef struct static_content
{
    const char *path;
    const unsigned char *data_start;
    const unsigned char *data_end;
    const char *content_type;
    bool is_gzip;
} static_content_t;

static static_content_t content_list_get_files[] = {
    {"/", index_html_start, index_html_end, "text/html", false},
    {"/index.html", index_html_start, index_html_end, "text/html", false},
    {"/mini.css", mini_css_start, mini_css_end, "text/css", false},
    {"/styles.css", styles_css_start, styles_css_end, "text/css", false},
    {"/configwifi.html", configwifi_html_start, configwifi_html_end, "text/html", false},
    {"/configwifi.js", configwifi_js_start, configwifi_js_end, "text/javascript", false},
    {"/favicon.png", favicon_png_start, favicon_png_end, "image/x-icon", false},
    {"/manifest.json", manifest_json_start, manifest_json_end, "text/json", false},
    {"/configmodulo.html", configmodulo_html_start, configmodulo_html_end, "text/html", false},
    {"/configmodulo.js", configmodulo_js_start, configmodulo_js_end, "text/javascript", false},
    {"/realtime.html", realtime_html_start, realtime_html_end, "text/html", false},
    {"/realtime.js", realtime_js_start, realtime_js_end, "text/javascript", false},
    {"/iconwifi_lg.png", iconwifi_lg_png_start, iconwifi_lg_png_end, "image", false},
    {"/iconwifi_md.png", iconwifi_md_png_start, iconwifi_md_png_end, "image", false},
    {"/iconwifi_sm.png", iconwifi_sm_png_start, iconwifi_sm_png_end, "image", false},
    {"/jsonconfigwifi.json", NULL, NULL, NULL, false},
    {"/jsonconfigmod.json", NULL, NULL, NULL, false},
    {"/scanwifi.json", NULL, NULL, NULL, false},
    {"/jsonrealtime.json", NULL, NULL, NULL, false},
    {"/restart_module", NULL, NULL, NULL, false}
    };

static static_content_t content_list_post_files[] = {
    {"/jsonconfigwifi.json", NULL, NULL, NULL, false},
    {"/jsonconfigmod.json", NULL, NULL, NULL, false}};

// Método OPTIONS -------------------------------------------------------------
esp_err_t http_handle_options(httpd_req_t *req);

// Método GET -----------------------------------------------------------------
esp_err_t http_handle_get(httpd_req_t *req);

// Recebe WIFI ----------------------------------------------------------------
static esp_err_t recebe_config_wifi(char *struct_json);

// Recebe Configuração --------------------------------------------------------
static int recebe_config_modulo(char *struct_json);

// Método POST ----------------------------------------------------------------
esp_err_t http_handle_post(httpd_req_t *req);

// Registros de estruturar de eventos -----------------------------------------

httpd_uri_t http_uri_options = {
    .uri = "/",
    .method = HTTP_OPTIONS,
    .handler = http_handle_options,
    .user_ctx = NULL};

// Função de inicio da aplicaçõo de servidor
void start_webserver(void);