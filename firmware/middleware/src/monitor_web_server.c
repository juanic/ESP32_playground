/*! @file monitor_web_server.c
 *  @brief Web UI and REST API for the people counter.
 *  @defgroup monitor_web_server impl.
 *  @{
 *
 * Sirve una página única con la última captura (con las detecciones
 * superpuestas por el navegador), los controles de imagen, el umbral de
 * confianza y la descarga de los archivos guardados en la SD.
 */

/*==================[inclusions]=============================================*/
#include "monitor_web_server.h"
#include "camera_hal.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "photo_repository.h"
#include "sdkconfig.h"
#include "wifi_hal.h"
#if CONFIG_SD_HAL_ENABLED
#include "sd_hal.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
/*==================[macros and definitions]=================================*/

#define MONITOR_STACK_SIZE		10240
#define MONITOR_MAX_SOCKETS		4
#define MONITOR_QUERY_MAX		256
#define MONITOR_DAYS_BUF		4096
#define MONITOR_FILES_BUF		24576
#define MONITOR_LATEST_BUF		4096
#define MONITOR_PATH_MAX		192
/*==================[internal data definition]===============================*/

static const char *TAG = "monitor_web";
static httpd_handle_t s_server = NULL;
static monitor_web_server_config_t s_config;

static const char s_index_html[] =
	"<!doctype html>\n"
	"<html lang='es'>\n"
	"<head>\n"
	"<meta charset='utf-8'>\n"
	"<meta name='viewport' content='width=device-width, initial-scale=1'>\n"
	"<title>Contador de personas</title>\n"
	"<style>\n"
	":root{color-scheme:dark}\n"
	"*{box-sizing:border-box}\n"
	"body{margin:0;background:#0d1117;color:#e6edf3;font-family:system-ui,sans-serif}\n"
	"header{display:flex;flex-wrap:wrap;gap:10px;align-items:center;padding:12px 20px;"
	"background:#161b22;border-bottom:1px solid #21262d}\n"
	"h1{font-size:1.05rem;margin:0 12px 0 0;font-weight:600}\n"
	"h2{font-size:.95rem;margin:0 0 8px;font-weight:600}\n"
	".pill{background:#21262d;border-radius:999px;padding:4px 12px;font-size:.78rem}\n"
	"main{display:grid;grid-template-columns:minmax(0,1fr) 340px;gap:20px;padding:20px;"
	"align-items:start}\n"
	"@media(max-width:900px){main{grid-template-columns:1fr}}\n"
	".viewer{position:relative;background:#000;border-radius:10px;overflow:hidden}\n"
	".viewer img{width:100%;display:block}\n"
	".viewer canvas{position:absolute;inset:0;width:100%;height:100%}\n"
	"section{background:#161b22;border:1px solid #21262d;border-radius:10px;"
	"padding:16px;margin-bottom:16px}\n"
	"label{display:flex;justify-content:space-between;gap:8px;font-size:.82rem;"
	"margin:10px 0 2px;color:#9da7b3}\n"
	"label span{color:#e6edf3}\n"
	"input[type=range]{width:100%;accent-color:#2f81f7}\n"
	"input[type=checkbox]{accent-color:#2f81f7}\n"
	"select{width:100%;background:#0d1117;color:#e6edf3;border:1px solid #30363d;"
	"border-radius:6px;padding:6px}\n"
	"button{background:#238636;border:0;color:#fff;padding:10px 16px;border-radius:6px;"
	"font-size:.88rem;cursor:pointer;margin-top:12px;width:100%}\n"
	"button.secondary{background:#30363d}\n"
	"a{color:#58a6ff;text-decoration:none}\n"
	"ul{list-style:none;padding:0;margin:0;max-height:260px;overflow:auto}\n"
	"li{display:flex;justify-content:space-between;align-items:center;gap:8px;padding:6px 0;"
	"border-bottom:1px solid #21262d;font-size:.82rem}\n"
	".muted{color:#8b949e;font-size:.78rem}\n"
	".act{display:inline-flex;align-items:center;gap:6px}\n"
	".ibtn{background:none;border:none;color:#8b949e;padding:2px 4px;margin:0;width:auto;"
	"cursor:pointer;display:inline-flex;align-items:center;border-radius:4px;text-decoration:none}\n"
	".ibtn:hover{color:#58a6ff;background:#21262d}\n"
	".ibtn.del:hover{color:#f85149;background:rgba(248,81,73,.15)}\n"
	".btn-csv{font-size:.72rem;font-weight:600;color:#58a6ff;background:#161b22;border:1px solid #30363d;"
	"padding:2px 6px;border-radius:4px;display:inline-flex;align-items:center;gap:4px;text-decoration:none;"
	"cursor:pointer;line-height:1.2}\n"
	".btn-csv:hover{background:#21262d;border-color:#58a6ff;color:#79c0ff}\n"
	"</style>\n"
	"</head>\n"
	"<body>\n"
	"<header>\n"
	"<h1>Contador de personas</h1>\n"
	"<span class='pill' id='st-ip'>IP ...</span>\n"
	"<span class='pill' id='st-time'>--</span>\n"
	"<span class='pill' id='st-sd'>SD ...</span>\n"
	"<span class='pill' id='st-count'>Personas: -</span>\n"
	"<span class='pill' id='st-msg'></span>\n"
	"</header>\n"
	"<main>\n"
	"<div>\n"
	"<div class='viewer'><img id='photo' alt='Ultima captura'><canvas id='overlay'></canvas></div>\n"
	"<p class='muted' id='photo-name'></p>\n"
	"</div>\n"
	"<div>\n"
	"<section>\n"
	"<h2>Ajustes de imagen</h2>\n"
	"<label>Brillo <span id='v-brightness'>0</span></label>\n"
	"<input id='brightness' type='range' min='-2' max='2' step='1' value='0'>\n"
	"<label>Contraste <span id='v-contrast'>0</span></label>\n"
	"<input id='contrast' type='range' min='-2' max='2' step='1' value='0'>\n"
	"<label>Saturacion <span id='v-saturation'>0</span></label>\n"
	"<input id='saturation' type='range' min='-2' max='2' step='1' value='0'>\n"
	"<label>Balance de blancos</label>\n"
	"<select id='wb'><option value='0'>Auto</option><option value='1'>Soleado</option>"
	"<option value='2'>Nublado</option><option value='3'>Oficina</option>"
	"<option value='4'>Hogar</option></select>\n"
	"<label>Exposicion automatica</label>\n"
	"<input id='exposure_auto' type='checkbox' checked>\n"
	"<label>Nivel de exposicion <span id='v-exposure_level'>0</span></label>\n"
	"<input id='exposure_level' type='range' min='0' max='1200' step='10' value='0'>\n"
	"<label>Ganancia automatica</label>\n"
	"<input id='gain_auto' type='checkbox' checked>\n"
	"<label>Techo de ganancia <span id='v-gain_ceiling'>0</span></label>\n"
	"<input id='gain_ceiling' type='range' min='0' max='6' step='1' value='0'>\n"
	"<label>Espejo horizontal</label>\n"
	"<input id='hmirror' type='checkbox'>\n"
	"<label>Voltear vertical</label>\n"
	"<input id='vflip' type='checkbox'>\n"
	"<label>Calidad JPEG (menor = mejor) <span id='v-quality'>10</span></label>\n"
	"<input id='quality' type='range' min='4' max='40' step='1' value='10'>\n"
	"<label>Umbral de confianza (%) <span id='v-threshold'>50</span></label>\n"
	"<input id='threshold' type='range' min='5' max='95' step='5' value='50'>\n"
	"<button onclick='applySettings()'>Aplicar ajustes</button>\n"
	"<button class='secondary' onclick='captureNow()'>Capturar ahora</button>\n"
	"</section>\n"
	"<section>\n"
	"<h2>Dias guardados</h2>\n"
	"<ul id='days'></ul>\n"
	"<div style='display:flex;justify-content:space-between;align-items:center;margin-top:16px'>\n"
	"<h2 id='day-title' style='margin:0'>Archivos</h2>\n"
	"<a id='day-csv' class='btn-csv' style='display:none' href='#'></a>\n"
	"</div>\n"
	"<ul id='files'></ul>\n"
	"</section>\n"
	"</div>\n"
	"</main>\n"
	"<script>\n"
	"const $=id=>document.getElementById(id);\n"
	"let synced=false;\n"
	"function fmtSize(n){if(n>=1048576)return (n/1048576).toFixed(1)+' MB';"
	"if(n>=1024)return Math.round(n/1024)+' KB';return n+' B';}\n"
	"function flash(m){$('st-msg').textContent=m;setTimeout(()=>{"
	"if($('st-msg').textContent===m)$('st-msg').textContent='';},4000);}\n"
	"function bindRange(id){const el=$(id),out=$('v-'+id);if(!el||!out)return;"
	"const upd=()=>{out.textContent=el.value;};el.addEventListener('input',upd);upd();}\n"
	"['brightness','contrast','saturation','exposure_level','gain_ceiling','quality',"
	"'threshold'].forEach(bindRange);\n"
	"async function loadStatus(){try{const s=await(await fetch('/api/status')).json();"
	"$('st-ip').textContent=s.ip||'sin IP';"
	"$('st-time').textContent=s.synced?s.time:'hora sin sincronizar';"
	"$('st-sd').textContent=s.sd?('SD '+s.free_mb+' MB libres'):'SD sin montar';"
	"if(!synced&&s.image){const i=s.image;"
	"$('brightness').value=i.brightness;$('contrast').value=i.contrast;"
	"$('saturation').value=i.saturation;$('wb').value=i.wb;"
	"$('exposure_auto').checked=!!i.exposure_auto;$('exposure_level').value=i.exposure_level;"
	"$('gain_auto').checked=!!i.gain_auto;$('gain_ceiling').value=i.gain_ceiling;"
	"$('hmirror').checked=!!i.hmirror;$('vflip').checked=!!i.vflip;"
	"$('quality').value=i.quality;$('threshold').value=i.threshold;synced=true;"
	"['brightness','contrast','saturation','exposure_level','gain_ceiling','quality',"
	"'threshold'].forEach(id=>$('v-'+id).textContent=$(id).value);}}catch(e){}}\n"
	"async function applySettings(){const q=new URLSearchParams({"
	"brightness:$('brightness').value,contrast:$('contrast').value,"
	"saturation:$('saturation').value,wb:$('wb').value,"
	"exposure_auto:$('exposure_auto').checked?1:0,exposure_level:$('exposure_level').value,"
	"gain_auto:$('gain_auto').checked?1:0,gain_ceiling:$('gain_ceiling').value,"
	"hmirror:$('hmirror').checked?1:0,vflip:$('vflip').checked?1:0,"
	"quality:$('quality').value,threshold:$('threshold').value});"
	"try{const r=await fetch('/api/settings?'+q.toString(),{method:'POST'});"
	"flash(r.ok?'Ajustes aplicados':'Error al aplicar');}catch(e){flash('Error al aplicar');}}\n"
	"async function captureNow(){try{await fetch('/api/capture',{method:'POST'});"
	"flash('Capturando...');setTimeout(loadLatest,5000);}catch(e){}}\n"
	"function drawBoxes(det){const img=$('photo'),cv=$('overlay'),ctx=cv.getContext('2d');"
	"cv.width=img.naturalWidth||1600;cv.height=img.naturalHeight||1200;"
	"ctx.clearRect(0,0,cv.width,cv.height);if(!det||!det.boxes)return;"
	"const lw=Math.max(4,Math.round(cv.width/240));ctx.lineWidth=lw;"
	"ctx.font=Math.max(16,Math.round(cv.width/38))+'px sans-serif';"
	"det.boxes.forEach((b,i)=>{const x=b[0]*cv.width,y=b[1]*cv.height,"
	"w=b[2]*cv.width,h=b[3]*cv.height;ctx.strokeStyle='#00e676';ctx.strokeRect(x,y,w,h);"
	"const label='#'+(i+1)+' '+(b[4]*100).toFixed(0)+'%';"
	"const tw=ctx.measureText(label).width+12,th=Math.max(24,Math.round(cv.width/38)+8);"
	"ctx.fillStyle='rgba(0,230,118,.9)';ctx.fillRect(x,Math.max(0,y-th),tw,th);"
	"ctx.fillStyle='#001b0c';ctx.fillText(label,x+6,Math.max(16,y-8));});}\n"
	"async function loadLatest(){try{const d=await(await fetch('/api/latest')).json();"
	"if(!d.photo)return;const img=$('photo');img.onload=()=>drawBoxes(d.det);"
	"img.src=d.photo+'?t='+Date.now();"
	"$('photo-name').textContent=d.day+' '+d.name;"
	"$('st-count').textContent='Personas: '+(d.det?d.det.count:'-')"
	"+(d.det&&d.det.avg?' ('+Math.round(d.det.avg*100)+'%)':'');"
	"}catch(e){}}\n"
	"async function showPhoto(url,name,detUrl){const img=$('photo');let det=null;"
	"try{det=await(await fetch(detUrl)).json();}catch(e){}"
	"$('photo-name').textContent=name;"
	"$('st-count').textContent='Personas: '+(det?det.count:'-')"
	"+(det&&det.avg?' ('+Math.round(det.avg*100)+'%)':'');"
	"img.onload=()=>drawBoxes(det);img.src=url+'?t='+Date.now();}\n"
	"const ICO_DL=\"<svg width='13' height='13' viewBox='0 0 16 16' fill='currentColor'><path d='M2.75 14A1.75 1.75 0 0 1 1 12.25v-2.5a.75.75 0 0 1 1.5 0v2.5c0 .138.112.25.25.25h10.5a.25.25 0 0 0 .25-.25v-2.5a.75.75 0 0 1 1.5 0v2.5A1.75 1.75 0 0 1 13.25 14Z'/><path d='M7.25 1.75a.75.75 0 0 1 1.5 0v7.69l2.72-2.72a.75.75 0 1 1 1.06 1.06l-4 4a.75.75 0 0 1-1.06 0l-4-4a.75.75 0 1 1 1.06-1.06l2.72 2.72Z'/></svg>\";\n"
	"const ICO_DEL=\"<svg width='13' height='13' viewBox='0 0 16 16' fill='currentColor'><path d='M11 1.75V3h2.25a.75.75 0 0 1 0 1.5H2.75a.75.75 0 0 1 0-1.5H5V1.75C5 .784 5.784 0 6.75 0h2.5C10.216 0 11 .784 11 1.75ZM4.496 6.675l.66 6.6a1.75 1.75 0 0 0 1.744 1.575h2.2c.928 0 1.693-.722 1.744-1.575l.66-6.6a.75.75 0 1 0-1.496-.15l-.66 6.6a.25.25 0 0 1-.248.225h-2.2a.25.25 0 0 1-.248-.225l-.66-6.6a.75.75 0 0 0-1.496.15Z'/></svg>\";\n"
	"let curDay='';\n"
	"async function deleteDay(day){\n"
	"if(!confirm('¿Borrar todo el día '+day+' y sus fotos?'))return;\n"
	"try{const r=await fetch('/api/delete?day='+day,{method:'POST'});\n"
	"if(r.ok){flash('Día '+day+' borrado');if(curDay===day){curDay='';$('day-title').textContent='Archivos';const cb=$('day-csv');if(cb)cb.style.display='none';$('files').innerHTML='';}loadDays();loadLatest();}\n"
	"else{flash('Error al borrar');}}catch(e){flash('Error al borrar');}}\n"
	"async function deletePhoto(day,file){\n"
	"if(!confirm('¿Borrar la foto '+file+'?'))return;\n"
	"try{const r=await fetch('/api/delete?day='+day+'&file='+file,{method:'POST'});\n"
	"if(r.ok){flash('Foto borrada');loadDay(day);loadDays();loadLatest();}\n"
	"else{flash('Error al borrar');}}catch(e){flash('Error al borrar');}}\n"
	"async function loadDays(){try{const d=await(await fetch('/api/days')).json();"
	"const ul=$('days');ul.innerHTML='';(d.days||[]).forEach(x=>{"
	"const li=document.createElement('li');const a=document.createElement('a');"
	"a.href='#';a.textContent=x.day;a.onclick=e=>{e.preventDefault();loadDay(x.day);};"
	"const act=document.createElement('div');act.className='act';"
	"const sp=document.createElement('span');sp.className='muted';sp.textContent=x.files+' fotos';"
	"const dl=document.createElement('a');dl.className='btn-csv';dl.href='/files/data/'+x.day+'.csv?dl=1';dl.title='Descargar CSV '+x.day;dl.innerHTML='CSV '+ICO_DL;"
	"const del=document.createElement('button');del.className='ibtn del';del.title='Borrar día';del.innerHTML=ICO_DEL;del.onclick=()=>deleteDay(x.day);"
	"act.append(sp,dl,del);li.append(a,act);ul.appendChild(li);});"
	"}catch(e){}}\n"
	"async function loadDay(day){curDay=day;try{const d=await(await fetch('/api/files?day='+day)).json();"
	"$('day-title').textContent='Archivos de '+day;"
	"const cb=$('day-csv');if(cb){cb.href='/files/data/'+day+'.csv?dl=1';cb.innerHTML='Descargar CSV '+ICO_DL;cb.style.display='inline-flex';}"
	"const ul=$('files');ul.innerHTML='';"
	"(d.files||[]).sort((a,b)=>b.name.localeCompare(a.name)).forEach(f=>{"
	"const li=document.createElement('li');const base=f.name.replace('.jpg','');"
	"const a=document.createElement('a');a.href='#';a.textContent=base;"
	"a.onclick=e=>{e.preventDefault();showPhoto('/files/photos/'+day+'/'+f.name,"
	"day+' '+base,'/files/photos/'+day+'/'+base+'.det');};"
	"const act=document.createElement('div');act.className='act';"
	"const sp=document.createElement('span');sp.className='muted';sp.textContent=fmtSize(f.size);"
	"const dl=document.createElement('a');dl.className='ibtn';dl.href='/files/photos/'+day+'/'+f.name+'?dl=1';dl.title='Descargar JPG';dl.innerHTML=ICO_DL;"
	"const del=document.createElement('button');del.className='ibtn del';del.title='Borrar captura';del.innerHTML=ICO_DEL;del.onclick=()=>deletePhoto(day,f.name);"
	"act.append(sp,dl,del);li.append(a,act);ul.appendChild(li);});"
	"}catch(e){}}\n"
	"loadStatus();loadLatest();loadDays();\n"
	"setInterval(loadStatus,15000);\n"
	"</script>\n"
	"</body>\n"
	"</html>\n";

/*==================[internal functions declaration]=========================*/

static esp_err_t index_get_handler(httpd_req_t *req);
static esp_err_t status_get_handler(httpd_req_t *req);
static esp_err_t latest_get_handler(httpd_req_t *req);
static esp_err_t capture_post_handler(httpd_req_t *req);
static esp_err_t settings_post_handler(httpd_req_t *req);
static esp_err_t days_get_handler(httpd_req_t *req);
static esp_err_t files_api_get_handler(httpd_req_t *req);
static esp_err_t files_get_handler(httpd_req_t *req);
static bool query_int(const char *query, const char *key, int *out);
static const char *content_type_for(const char *path);

/*==================[internal functions definition]==========================*/

static bool query_int(const char *query, const char *key, int *out) {
	if (query == NULL) {
		return false;
	}
	char value[16];
	if (httpd_query_key_value(query, key, value, sizeof(value)) != ESP_OK) {
		return false;
	}
	*out = atoi(value);
	return true;
}

static const char *content_type_for(const char *path) {
	const char *dot = strrchr(path, '.');
	if (dot == NULL) {
		return "application/octet-stream";
	}
	if (strcmp(dot, ".jpg") == 0) {
		return "image/jpeg";
	}
	if (strcmp(dot, ".csv") == 0) {
		return "text/csv; charset=utf-8";
	}
	if (strcmp(dot, ".det") == 0 || strcmp(dot, ".json") == 0) {
		return "application/json";
	}
	return "application/octet-stream";
}

static esp_err_t index_get_handler(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/html; charset=utf-8");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	return httpd_resp_send(req, s_index_html, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t status_get_handler(httpd_req_t *req) {
	char ip[16] = "0.0.0.0";
	if (wifi_hal_get_ip(ip, sizeof(ip)) != ESP_OK) {
		snprintf(ip, sizeof(ip), "sin IP");
	}

	bool synced = wifi_hal_time_synced();
	char timebuf[24] = "--";
	if (synced) {
		time_t now = time(NULL);
		struct tm tm;
		localtime_r(&now, &tm);
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);
	}

	uint32_t free_mb = 0;
	bool sd_ready = false;
#if CONFIG_SD_HAL_ENABLED
	sd_ready = sd_hal_is_mounted();
	free_mb = (uint32_t)(sd_hal_free_bytes() / (1024 * 1024));
#endif

	const char *sensor = camera_hal_sensor_name();
	const camera_hal_image_settings_t *img = s_config.image;

	char body[768];
	snprintf(body, sizeof(body),
			 "{\"ip\":\"%s\",\"sd\":%d,\"free_mb\":%u,\"sensor\":\"%s\","
			 "\"synced\":%d,\"time\":\"%s\",\"image\":{"
			 "\"brightness\":%d,\"contrast\":%d,\"saturation\":%d,\"wb\":%u,"
			 "\"exposure_auto\":%u,\"exposure_level\":%d,\"gain_auto\":%u,"
			 "\"gain_ceiling\":%u,\"hmirror\":%u,\"vflip\":%u,"
			 "\"quality\":%u,\"threshold\":%u}}",
			 ip, sd_ready ? 1 : 0, (unsigned)free_mb,
			 sensor != NULL ? sensor : "desconocido",
			 synced ? 1 : 0, timebuf,
			 img != NULL ? img->brightness : 0,
			 img != NULL ? img->contrast : 0,
			 img != NULL ? img->saturation : 0,
			 img != NULL ? img->wb_mode : 0,
			 img != NULL ? img->exposure_auto : 1,
			 img != NULL ? img->exposure_level : 0,
			 img != NULL ? img->gain_auto : 1,
			 img != NULL ? img->gain_ceiling : 0,
			 img != NULL ? img->hmirror : 0,
			 img != NULL ? img->vflip : 0,
			 img != NULL ? img->quality : 10,
			 s_config.threshold_pct != NULL ? *s_config.threshold_pct : 50);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	return httpd_resp_sendstr(req, body);
}

static void split_path(const char *jpg_path, char *day, size_t day_len,
					   char *name, size_t name_len) {
	day[0] = '\0';
	name[0] = '\0';

	const char *slash = strrchr(jpg_path, '/');
	if (slash == NULL) {
		snprintf(name, name_len, "%s", jpg_path);
		return;
	}
	snprintf(name, name_len, "%s", slash + 1);

	char parent[MONITOR_PATH_MAX];
	snprintf(parent, sizeof(parent), "%.*s", (int)(slash - jpg_path), jpg_path);
	const char *day_slash = strrchr(parent, '/');
	if (day_slash != NULL) {
		snprintf(day, day_len, "%s", day_slash + 1);
	}
}

static esp_err_t latest_get_handler(httpd_req_t *req) {
	char jpg[192];
	int64_t ts = 0;

	if (photo_repo_find_latest(jpg, sizeof(jpg), &ts) != ESP_OK) {
		httpd_resp_set_type(req, "application/json");
		return httpd_resp_sendstr(req, "{\"photo\":null,\"det\":null}");
	}

	char day[16];
	char name[32];
	split_path(jpg, day, sizeof(day), name, sizeof(name));

	char det_path[192];
	snprintf(det_path, sizeof(det_path), "%s", jpg);
	char *dot = strrchr(det_path, '.');
	if (dot != NULL) {
		snprintf(dot, sizeof(det_path) - (size_t)(dot - det_path), ".det");
	}

	char det_json[2048];
	bool has_det = photo_repo_read_detection(det_path, det_json, sizeof(det_json)) == ESP_OK;

	const char *mount = "/sdcard";
#if CONFIG_SD_HAL_ENABLED
	if (sd_hal_mount_point() != NULL) {
		mount = sd_hal_mount_point();
	}
#endif
	const char *url = jpg;
	if (strncmp(jpg, mount, strlen(mount)) == 0) {
		url = jpg + strlen(mount);
	}

	char timebuf[24];
	time_t value = (time_t)ts;
	struct tm tm;
	localtime_r(&value, &tm);
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

	char body[MONITOR_LATEST_BUF];
	snprintf(body, sizeof(body),
			 "{\"photo\":\"/files%s\",\"name\":\"%s\",\"day\":\"%s\","
			 "\"time\":\"%s\",\"det\":%s}",
			 url, name, day, timebuf, has_det ? det_json : "null");

	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	return httpd_resp_sendstr(req, body);
}

static esp_err_t capture_post_handler(httpd_req_t *req) {
	if (s_config.request_capture != NULL) {
		s_config.request_capture();
	}
	httpd_resp_set_type(req, "application/json");
	return httpd_resp_sendstr(req, "{\"ok\":true}");
}

static esp_err_t settings_post_handler(httpd_req_t *req) {
	char query[MONITOR_QUERY_MAX];
	if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
		return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Sin parametros");
	}

	int value = 0;
	if (s_config.image != NULL) {
		if (query_int(query, "brightness", &value)) {
			s_config.image->brightness = (int8_t)value;
		}
		if (query_int(query, "contrast", &value)) {
			s_config.image->contrast = (int8_t)value;
		}
		if (query_int(query, "saturation", &value)) {
			s_config.image->saturation = (int8_t)value;
		}
		if (query_int(query, "wb", &value)) {
			s_config.image->wb_mode = (uint8_t)value;
		}
		if (query_int(query, "exposure_auto", &value)) {
			s_config.image->exposure_auto = (uint8_t)(value != 0);
		}
		if (query_int(query, "exposure_level", &value)) {
			s_config.image->exposure_level = (int16_t)value;
		}
		if (query_int(query, "gain_auto", &value)) {
			s_config.image->gain_auto = (uint8_t)(value != 0);
		}
		if (query_int(query, "gain_ceiling", &value)) {
			s_config.image->gain_ceiling = (uint8_t)value;
		}
		if (query_int(query, "hmirror", &value)) {
			s_config.image->hmirror = (uint8_t)(value != 0);
		}
		if (query_int(query, "vflip", &value)) {
			s_config.image->vflip = (uint8_t)(value != 0);
		}
		if (query_int(query, "quality", &value)) {
			s_config.image->quality = (uint8_t)value;
		}
		camera_hal_apply_settings(s_config.image);
	}

	if (s_config.threshold_pct != NULL && query_int(query, "threshold", &value)) {
		if (value < 1) {
			value = 1;
		}
		if (value > 99) {
			value = 99;
		}
		*s_config.threshold_pct = (uint8_t)value;
	}

	httpd_resp_set_type(req, "application/json");
	return httpd_resp_sendstr(req, "{\"ok\":true}");
}

static esp_err_t days_get_handler(httpd_req_t *req) {
	char *buffer = malloc(MONITOR_DAYS_BUF);
	if (buffer == NULL) {
		return httpd_resp_send_500(req);
	}

	photo_repo_list_days_json(buffer, MONITOR_DAYS_BUF);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	esp_err_t err = httpd_resp_sendstr(req, buffer);
	free(buffer);
	return err;
}

static esp_err_t files_api_get_handler(httpd_req_t *req) {
	char query[MONITOR_QUERY_MAX];
	char day[16];
	if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK ||
		httpd_query_key_value(query, "day", day, sizeof(day)) != ESP_OK) {
		return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Falta day");
	}

	char *buffer = malloc(MONITOR_FILES_BUF);
	if (buffer == NULL) {
		return httpd_resp_send_500(req);
	}

	photo_repo_list_day_json(day, buffer, MONITOR_FILES_BUF);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	esp_err_t err = httpd_resp_sendstr(req, buffer);
	free(buffer);
	return err;
}

static esp_err_t files_get_handler(httpd_req_t *req) {
	const char *mount = "/sdcard";
#if CONFIG_SD_HAL_ENABLED
	if (sd_hal_mount_point() != NULL) {
		mount = sd_hal_mount_point();
	}
#endif

	/* req->uri may include the query string: copy only the path part. */
	char uri[MONITOR_PATH_MAX];
	size_t uri_len = strcspn(req->uri, "?");
	if (uri_len >= sizeof(uri)) {
		return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Ruta muy larga");
	}
	memcpy(uri, req->uri, uri_len);
	uri[uri_len] = '\0';

	if (strstr(uri, "..") != NULL) {
		return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Ruta invalida");
	}

	/* uri is "/files/<relative path>" */
	const char *relative = uri + strlen("/files");
	char path[256];
	snprintf(path, sizeof(path), "%s%s", mount, relative);

	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "No existe");
	}

	httpd_resp_set_type(req, content_type_for(path));

	char query[MONITOR_QUERY_MAX];
	char dl[8];
	if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK &&
		httpd_query_key_value(query, "dl", dl, sizeof(dl)) == ESP_OK &&
		atoi(dl) != 0) {
		const char *name = strrchr(path, '/');
		char header[128];
		snprintf(header, sizeof(header), "attachment; filename=\"%s\"",
				 name != NULL ? name + 1 : "archivo");
		httpd_resp_set_hdr(req, "Content-Disposition", header);
	}

	char buffer[2048];
	size_t read = 0;
	while ((read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		if (httpd_resp_send_chunk(req, buffer, read) != ESP_OK) {
			fclose(file);
			return ESP_FAIL;
		}
	}
	fclose(file);

	return httpd_resp_send_chunk(req, NULL, 0);
}

static esp_err_t delete_api_handler(httpd_req_t *req) {
	char query[MONITOR_QUERY_MAX];
	char day[16] = {0};
	char file[32] = {0};

	if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK ||
		httpd_query_key_value(query, "day", day, sizeof(day)) != ESP_OK) {
		return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Falta day");
	}

	esp_err_t err;
	if (httpd_query_key_value(query, "file", file, sizeof(file)) == ESP_OK && file[0] != '\0') {
		err = photo_repo_delete_photo(day, file);
	} else {
		err = photo_repo_delete_day(day);
	}

	if (err != ESP_OK) {
		return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error al borrar");
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	return httpd_resp_sendstr(req, "{\"ok\":true}");
}

/*==================[external functions definition]==========================*/

esp_err_t monitor_web_server_start(const monitor_web_server_config_t *config) {
	if (config == NULL) {
		return ESP_ERR_INVALID_ARG;
	}
	if (s_server != NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	s_config = *config;

	httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
	http_config.server_port = config->port;
	http_config.ctrl_port = config->port + 1;
	http_config.max_open_sockets = MONITOR_MAX_SOCKETS;
	http_config.stack_size = MONITOR_STACK_SIZE;
	http_config.max_uri_handlers = 12;
	http_config.lru_purge_enable = true;
	http_config.uri_match_fn = httpd_uri_match_wildcard;

	esp_err_t err = httpd_start(&s_server, &http_config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo iniciar el servidor HTTP: %s", esp_err_to_name(err));
		s_server = NULL;
		return err;
	}

	const httpd_uri_t index_uri = {
		.uri = "/", .method = HTTP_GET, .handler = index_get_handler,
	};
	const httpd_uri_t status_uri = {
		.uri = "/api/status", .method = HTTP_GET, .handler = status_get_handler,
	};
	const httpd_uri_t latest_uri = {
		.uri = "/api/latest", .method = HTTP_GET, .handler = latest_get_handler,
	};
	const httpd_uri_t capture_uri = {
		.uri = "/api/capture", .method = HTTP_POST, .handler = capture_post_handler,
	};
	const httpd_uri_t settings_uri = {
		.uri = "/api/settings", .method = HTTP_POST, .handler = settings_post_handler,
	};
	const httpd_uri_t days_uri = {
		.uri = "/api/days", .method = HTTP_GET, .handler = days_get_handler,
	};
	const httpd_uri_t files_api_uri = {
		.uri = "/api/files", .method = HTTP_GET, .handler = files_api_get_handler,
	};
	const httpd_uri_t files_uri = {
		.uri = "/files/*", .method = HTTP_GET, .handler = files_get_handler,
	};
	const httpd_uri_t delete_uri = {
		.uri = "/api/delete", .method = HTTP_POST, .handler = delete_api_handler,
	};

	httpd_register_uri_handler(s_server, &index_uri);
	httpd_register_uri_handler(s_server, &status_uri);
	httpd_register_uri_handler(s_server, &latest_uri);
	httpd_register_uri_handler(s_server, &capture_uri);
	httpd_register_uri_handler(s_server, &settings_uri);
	httpd_register_uri_handler(s_server, &days_uri);
	httpd_register_uri_handler(s_server, &files_api_uri);
	httpd_register_uri_handler(s_server, &files_uri);
	httpd_register_uri_handler(s_server, &delete_uri);

	ESP_LOGI(TAG, "Servidor web escuchando en el puerto %u", (unsigned)config->port);
	return ESP_OK;
}

esp_err_t monitor_web_server_stop(void) {
	if (s_server == NULL) {
		return ESP_ERR_INVALID_STATE;
	}
	esp_err_t err = httpd_stop(s_server);
	s_server = NULL;
	return err;
}

/*==================[end of file]============================================*/
