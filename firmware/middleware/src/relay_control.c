/**
 * @file relay_control.c
 * @brief Control-plane logic for the audio relay sink (App-driven).
 */

#include "relay_control.h"
#include "bt_access_control.h"
#include "ble_control_hal.h"
#include "avrcp_hal.h"
#include "bt_classic_hal.h"
#include "nvs_hal.h"
#include "time_hal.h"
#include "inter_board_link.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

static const char *TAG = "relay_control";

/* ======================== Config ======================== */

#define NVS_NAMESPACE   "relay"
#define NVS_KEY_PASS    "passwd"
#define NVS_KEY_GRANTS  "grants"

/* ======================== State ======================== */

static bool s_inited = false;
static bool s_authenticated = false;
static bool s_tx_enabled = true;

static char s_password[RELAY_CONTROL_PASSWORD_MAX_LEN];

static char s_peer_mac[18] = {0};

static relay_grant_t s_grants[RELAY_CONTROL_MAX_GRANTS];
static uint8_t s_grant_count = 0;

/* ======================== Forward declarations ======================== */

static void on_ble_cmd(const char *line, uint16_t len, void *ctx);
static void on_conn_state(bool connected, const uint8_t *peer_bda);
static void on_avrcp_meta(const avrcp_meta_t *meta, void *ctx);
static void handle_command(const char *cmd);
static void on_inter_board_response(const char *line, void *ctx);
static void on_inter_board_event(const char *line, void *ctx);

/* ======================== Response helpers ======================== */

static void rsp(const char *fmt, ...) {
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ble_control_hal_send_rsp(buf);
    ESP_LOGI(TAG, "RSP %s", buf);
}

static void evt(const char *fmt, ...) {
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ble_control_hal_send_evt(buf);
    ESP_LOGI(TAG, "EVT %s", buf);
}

static bool is_auth(void) {
    return s_authenticated;
}

/* ======================== MAC parsing / formatting ======================== */

static bool parse_mac(const char *s, uint8_t *bda) {
    if (!s || !bda) return false;
    unsigned int v[6];
    if (sscanf(s, "%2x:%2x:%2x:%2x:%2x:%2x",
               &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) != 6) {
        return false;
    }
    for (int i = 0; i < 6; i++) {
        bda[i] = (uint8_t)v[i];
    }
    return true;
}

static void format_mac(const uint8_t *bda, char *out) {
    sprintf(out, "%02x:%02x:%02x:%02x:%02x:%02x",
            bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
}

/* ======================== Time grants ======================== */

static bool grant_find(const uint8_t *bda, int *idx) {
    for (int i = 0; i < s_grant_count; i++) {
        if (memcmp(s_grants[i].bda, bda, 6) == 0) {
            if (idx) *idx = i;
            return true;
        }
    }
    return false;
}

static bool grants_save(void) {
    if (s_grant_count == 0) {
        NvsHalEraseKey(NVS_NAMESPACE, NVS_KEY_GRANTS);
        return true;
    }
    return NvsHalSetBlob(NVS_NAMESPACE, NVS_KEY_GRANTS,
                         s_grants, s_grant_count * sizeof(relay_grant_t));
}

static void grants_load(void) {
    size_t len = sizeof(s_grants);
    if (NvsHalGetBlob(NVS_NAMESPACE, NVS_KEY_GRANTS, s_grants, &len)) {
        s_grant_count = (uint8_t)(len / sizeof(relay_grant_t));
        ESP_LOGI(TAG, "Loaded %d time grant(s)", s_grant_count);
    } else {
        s_grant_count = 0;
    }
}

/**
 * @brief Look up the (possibly expired) grant expiry for a MAC.
 * @return true if the MAC has a grant entry at all (expired or not).
 */
static bool grant_get(const uint8_t *bda, int64_t *expiry) {
    int idx;
    if (!grant_find(bda, &idx)) return false;
    if (expiry) *expiry = s_grants[idx].expiry;
    return true;
}

/**
 * @brief Current Unix time, or TIME_HAL_INVALID if not synchronized.
 */
static int64_t now_epoch(void) {
    int64_t t = TIME_HAL_INVALID;
    TimeHalGetNow(&t);
    return t;
}

/** @brief True if the peer still has usable time (no expired grant). */
static bool grant_still_valid(const uint8_t *bda) {
    int64_t expiry;
    if (!grant_get(bda, &expiry)) {
        return true;                 /* no grant -> not restricted by time */
    }
    if (expiry <= 0) return true;    /* legacy entry without time -> ignore */

    if (!TimeHalIsSynced()) {
        /* No clock: cannot prove the grant is still valid. Fail closed. */
        ESP_LOGW(TAG, "Time not synced; denying restricted peer");
        return false;
    }
    int64_t now = now_epoch();
    if (now == TIME_HAL_INVALID) return false;
    return now < expiry;
}

/* ======================== Command implementation ======================== */

static void cmd_auth(const char *arg) {
    if (arg && strcmp(arg, s_password) == 0) {
        s_authenticated = true;
        rsp("AUTH OK");
    } else {
        s_authenticated = false;
        rsp("AUTH FAIL");
    }
}

static void cmd_setpass(const char *arg) {
    if (!is_auth()) {
        rsp("ERR AUTH_REQUIRED");
        return;
    }
    if (!arg || strlen(arg) == 0 || strlen(arg) >= RELAY_CONTROL_PASSWORD_MAX_LEN) {
        rsp("ERR INVALID_PASSWORD");
        return;
    }
    snprintf(s_password, sizeof(s_password), "%s", arg);
    NvsHalSetStr(NVS_NAMESPACE, NVS_KEY_PASS, s_password);
    rsp("OK");
    evt("EVT PASSWORD_CHANGED");
}

static void cmd_tx(const char *arg) {
    if (!is_auth()) {
        rsp("ERR AUTH_REQUIRED");
        return;
    }
    if (arg && strcmp(arg, "1") == 0) {
        s_tx_enabled = true;
        rsp("OK");
        evt("EVT TX 1");
    } else if (arg && strcmp(arg, "0") == 0) {
        s_tx_enabled = false;
        rsp("OK");
        evt("EVT TX 0");
    } else {
        rsp("ERR ARG");
    }
}

static void cmd_disconnect(const char *arg) {
    (void)arg;
    if (!is_auth()) {
        rsp("ERR AUTH_REQUIRED");
        return;
    }
    bt_classic_hal_disconnect();
    rsp("OK");
}

static void cmd_wl_add(const char *arg) {
    if (!is_auth()) { rsp("ERR AUTH_REQUIRED"); return; }
    uint8_t bda[ACL_BDA_LEN];
    if (!parse_mac(arg, bda)) { rsp("ERR MAC"); return; }
    AccessControlAdd(bda, ACL_WHITELIST);
    rsp("OK");
}

static void cmd_bl_add(const char *arg) {
    if (!is_auth()) { rsp("ERR AUTH_REQUIRED"); return; }
    uint8_t bda[ACL_BDA_LEN];
    if (!parse_mac(arg, bda)) { rsp("ERR MAC"); return; }
    AccessControlAdd(bda, ACL_BLACKLIST);
    rsp("OK");
}

static void cmd_remove(const char *arg) {
    if (!is_auth()) { rsp("ERR AUTH_REQUIRED"); return; }
    uint8_t bda[ACL_BDA_LEN];
    if (!parse_mac(arg, bda)) { rsp("ERR MAC"); return; }
    AccessControlRemove(bda);
    if (grant_find(bda, NULL)) {
        int idx;
        grant_find(bda, &idx);
        memmove(&s_grants[idx], &s_grants[idx + 1],
                (s_grant_count - idx - 1) * sizeof(relay_grant_t));
        s_grant_count--;
        grants_save();
    }
    rsp("OK");
}

static void cmd_list(const char *arg) {
    (void)arg;
    /* Whitelist / blacklist are not exposed as arrays via the public
     * access-control API; list via a small snapshot. For simplicity we
     * only report membership of the current peer. */
    uint8_t peer[ACL_BDA_LEN];
    if (bt_classic_hal_get_peer_bda(peer)) {
        int64_t expiry;
        bool has_grant = grant_get(peer, &expiry);
        rsp("PEER %s WL=%d BL=%d GRANT=%d EXP=%lld",
            s_peer_mac,
            AccessControlIsWhitelisted(peer) ? 1 : 0,
            AccessControlIsBlacklisted(peer) ? 1 : 0,
            has_grant ? 1 : 0,
            has_grant && expiry > 0 ? (long long)expiry : 0LL);
    } else {
        rsp("PEER none");
    }
}

static void cmd_status(const char *arg) {
    (void)arg;
    int64_t now = now_epoch();
    rsp("STATUS conn=%d tx=%d auth=%d peer=%s time=%s now=%lld",
        bt_classic_hal_is_connected() ? 1 : 0,
        s_tx_enabled ? 1 : 0,
        s_authenticated ? 1 : 0,
        s_peer_mac[0] ? s_peer_mac : "none",
        TimeHalIsSynced() ? "synced" : "nosync",
        now != TIME_HAL_INVALID ? (long long)now : 0LL);
}

static void cmd_get_meta(const char *arg) {
    (void)arg;
    avrcp_meta_t meta;
    if (avrcp_hal_get_metadata(&meta)) {
        rsp("META title=%s artist=%s album=%s play=%d",
            meta.title[0] ? meta.title : "-",
            meta.artist[0] ? meta.artist : "-",
            meta.album[0] ? meta.album : "-",
            (int)meta.play_state);
    } else {
        rsp("ERR NO_META");
    }
}

static void cmd_sync_time(const char *arg) {
    if (!is_auth()) {
        rsp("ERR AUTH_REQUIRED");
        return;
    }
    if (!arg || !*arg) {
        rsp("ERR ARG");
        return;
    }
    char *end = NULL;
    long long epoch = strtoll(arg, &end, 10);
    if (!end || *end != '\0' || epoch <= 0) {
        rsp("ERR ARG");
        return;
    }
    if (TimeHalSync(epoch)) {
        rsp("TIME OK epoch=%lld", epoch);
        evt("EVT TIME epoch=%lld", epoch);
    } else {
        rsp("ERR TIME");
    }
}

static void cmd_grant(const char *arg) {
    if (!is_auth()) {
        rsp("ERR AUTH_REQUIRED");
        return;
    }
    if (!arg || !*arg) {
        rsp("ERR ARG");
        return;
    }
    /* Format: GRANT <AA:BB:CC:DD:EE:FF> <minutes> */
    uint8_t bda[6];
    char macstr[18];
    long minutes = 0;
    if (sscanf(arg, "%17[0-9a-fA-F:] %ld", macstr, &minutes) != 2) {
        rsp("ERR ARG");
        return;
    }
    if (!parse_mac(macstr, bda) || minutes <= 0) {
        rsp("ERR ARG");
        return;
    }
    if (!TimeHalIsSynced()) {
        rsp("ERR NO_TIME");
        return;
    }
    int64_t now = now_epoch();
    if (now == TIME_HAL_INVALID) {
        rsp("ERR NO_TIME");
        return;
    }

    int idx;
    if (grant_find(bda, &idx)) {
        s_grants[idx].expiry = now + minutes * 60;
    } else {
        if (s_grant_count >= RELAY_CONTROL_MAX_GRANTS) {
            rsp("ERR FULL");
            return;
        }
        memcpy(s_grants[s_grant_count].bda, bda, 6);
        s_grants[s_grant_count].expiry = now + minutes * 60;
        s_grant_count++;
    }
    grants_save();
    format_mac(bda, macstr);
    rsp("GRANT OK mac=%s expiry=%lld", macstr, (long long)(now + minutes * 60));
    evt("EVT GRANT mac=%s minutes=%ld", macstr, minutes);
}

static void cmd_source_request(const char *command, const char *arg, bool authenticated) {
    if (authenticated && !is_auth()) {
        rsp("ERR AUTH_REQUIRED");
        return;
    }
    char line[96];
    if (arg) snprintf(line, sizeof(line), "%s %s", command, arg);
    else snprintf(line, sizeof(line), "%s", command);
    if (!InterBoardLinkRequest(line, on_inter_board_response, NULL)) {
        rsp("ERR TARGET_TIMEOUT");
    }
}

static void cmd_scan_target(const char *arg) { (void)arg; cmd_source_request("SCAN_TARGET", NULL, true); }
static void cmd_set_target(const char *arg) { cmd_source_request("SET_TARGET", arg, true); }
static void cmd_get_target(const char *arg) { (void)arg; cmd_source_request("GET_TARGET", NULL, false); }

/* ======================== Command dispatch ======================== */

typedef struct {
    const char *name;
    void (*fn)(const char *arg);
    bool need_arg;
} cmd_entry_t;

static const cmd_entry_t s_commands[] = {
    { "AUTH",       cmd_auth,            true  },
    { "SETPASS",    cmd_setpass,         true  },
    { "TX",         cmd_tx,              true  },
    { "DISCONNECT", cmd_disconnect,      false },
    { "WL_ADD",     cmd_wl_add,          true  },
    { "BL_ADD",     cmd_bl_add,          true  },
    { "REMOVE",     cmd_remove,          true  },
    { "LIST",       cmd_list,            false },
    { "STATUS",     cmd_status,          false },
    { "GET_META",   cmd_get_meta,        false },
    { "SYNC_TIME",  cmd_sync_time,       true  },
    { "GRANT",      cmd_grant,           true  },
    { "SCAN_TARGET", cmd_scan_target,     false },
    { "SET_TARGET",  cmd_set_target,      true  },
    { "GET_TARGET",  cmd_get_target,      false },
};

static void handle_command(const char *cmd) {
    if (!cmd || !*cmd) return;

    /* First token is the command name (case-insensitive), rest is arg. */
    char name[32];
    const char *rest = NULL;
    size_t i = 0;
    while (cmd[i] && !isspace((unsigned char)cmd[i]) && i < sizeof(name) - 1) {
        name[i] = (char)toupper((unsigned char)cmd[i]);
        i++;
    }
    name[i] = '\0';
    while (cmd[i] && isspace((unsigned char)cmd[i])) i++;
    if (cmd[i]) rest = &cmd[i];

    for (size_t k = 0; k < sizeof(s_commands) / sizeof(s_commands[0]); k++) {
        if (strcmp(name, s_commands[k].name) == 0) {
            if (s_commands[k].need_arg && !rest) {
                rsp("ERR ARG");
                return;
            }
            s_commands[k].fn(rest);
            return;
        }
    }
    rsp("ERR UNKNOWN");
}

/* ======================== Callbacks ======================== */

static void on_ble_cmd(const char *line, uint16_t len, void *ctx) {
    (void)len;
    (void)ctx;
    handle_command(line);
}

static void on_conn_state(bool connected, const uint8_t *peer_bda) {
    if (connected) {
        format_mac(peer_bda, s_peer_mac);
        evt("EVT A2DP connected=1 peer=%s", s_peer_mac);
    } else {
        s_peer_mac[0] = '\0';
        evt("EVT A2DP connected=0");
    }
}

static void on_avrcp_meta(const avrcp_meta_t *meta, void *ctx) {
    (void)ctx;
    if (!meta) return;
    evt("EVT META title=%s artist=%s album=%s play=%d track_new=%d",
        meta->title[0] ? meta->title : "-",
        meta->artist[0] ? meta->artist : "-",
        meta->album[0] ? meta->album : "-",
        (int)meta->play_state,
        meta->track_changed ? 1 : 0);
}

static void on_inter_board_response(const char *line, void *ctx) {
    (void)ctx;
    rsp("%s", line);
}

static void on_inter_board_event(const char *line, void *ctx) {
    (void)ctx;
    evt("EVT %s", line);
}

/* ======================== Public API ======================== */

bool RelayControlInit(void) {
    if (s_inited) return true;

    NvsHalInit();

    /* Load or create the access password */
    char stored[RELAY_CONTROL_PASSWORD_MAX_LEN];
    if (NvsHalGetStr(NVS_NAMESPACE, NVS_KEY_PASS, stored, sizeof(stored))) {
        snprintf(s_password, sizeof(s_password), "%s", stored);
    } else {
        snprintf(s_password, sizeof(s_password), "%s", RELAY_CONTROL_PASSWORD_DEFAULT);
        NvsHalSetStr(NVS_NAMESPACE, NVS_KEY_PASS, s_password);
    }
    ESP_LOGI(TAG, "Control password loaded");

    /* Access control lists */
    AccessControlInit();

    /* Virtual clock + per-MAC time grants */
    TimeHalInit();
    grants_load();

    /* BLE control transport */
    if (!ble_control_hal_init("Audio Relay CTL", on_ble_cmd, NULL)) {
        ESP_LOGE(TAG, "BLE control init failed");
        return false;
    }
    ble_control_hal_start();

    /* AVRCP metadata */
    avrcp_hal_init(on_avrcp_meta, NULL);

    /* A2DP connection events */
    bt_classic_hal_register_conn_cb(on_conn_state);

    /* The UART reply is asynchronous, so BLE command processing never waits
     * for the second ESP32. */
    InterBoardLinkSetEventCallback(on_inter_board_event, NULL);

    s_inited = true;
    ESP_LOGI(TAG, "Relay control initialized (tx=%d)", s_tx_enabled ? 1 : 0);
    return true;
}

bool RelayControlIsTxEnabled(void) {
    return s_tx_enabled;
}

bool RelayControlIsAllowedToPlay(const uint8_t *bda) {
    if (!bda) return false;

    /* MAC layer (whitelist / blacklist) */
    if (!AccessControlIsAllowed(bda)) {
        return false;
    }

    /* Per-MAC time grant: if expired, deny even if MAC is allowed. */
    if (!grant_still_valid(bda)) {
        ESP_LOGW(TAG, "Peer blocked by expired time grant");
        return false;
    }

    return true;
}

bool RelayControlSetPassword(const char *password) {
    if (!password || strlen(password) == 0 ||
        strlen(password) >= RELAY_CONTROL_PASSWORD_MAX_LEN) {
        return false;
    }
    snprintf(s_password, sizeof(s_password), "%s", password);
    return NvsHalSetStr(NVS_NAMESPACE, NVS_KEY_PASS, s_password);
}

/*==================[end of file]============================================*/
