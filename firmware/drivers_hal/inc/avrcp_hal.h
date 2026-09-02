#ifndef AVRCP_HAL_H
#define AVRCP_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup avrcp_hal AVRCP HAL
 *  @brief AVRCP Controller (CT) — media metadata and playback events.
 *  @{
 *
 * @section genDesc General Description
 *
 * Exposes AVRCP Controller functionality so the sink can query the
 * connected A2DP source (a phone) for "now playing" metadata (title,
 * artist, album, cover art handle) and subscribe to playback events
 * (track changed, play/pause status).
 *
 * @note Requires CONFIG_BT_AVRCP_ENABLED, which is auto-selected by
 * CONFIG_BT_A2DP_ENABLE on Bluedroid.
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

#define AVRCP_HAL_TITLE_MAX_LEN   128
#define AVRCP_HAL_ARTIST_MAX_LEN  128
#define AVRCP_HAL_ALBUM_MAX_LEN   128

/*==================[typedef]================================================*/

/**
 * @brief Playback status.
 */
typedef enum {
    AVRCP_PLAY_STATE_STOPPED = 0,
    AVRCP_PLAY_STATE_PLAYING,
    AVRCP_PLAY_STATE_PAUSED,
    AVRCP_PLAY_STATE_FWD_SEEK,
    AVRCP_PLAY_STATE_REV_SEEK,
    AVRCP_PLAY_STATE_ERROR,
} avrcp_play_state_t;

/**
 * @brief Now-playing metadata l.
 */
typedef struct {
    char title[AVRCP_HAL_TITLE_MAX_LEN];
    char artist[AVRCP_HAL_ARTIST_MAX_LEN];
    char album[AVRCP_HAL_ALBUM_MAX_LEN];
    avrcp_play_state_t play_state;
    bool track_changed;
} avrcp_meta_t;

/**
 * @brief Callback when metadata or playback status changed.
 *
 * @param meta  Metadata snapshot (valid until callback returns).
 * @param ctx   User context.
 */
typedef void (*avrcp_meta_cb_t)(const avrcp_meta_t *meta, void *ctx);

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize AVRCP Controller.
 *
 * @param cb  Callback on metadata/playback change.
 * @param ctx User context.
 * @return true if successful.
 */
bool avrcp_hal_init(avrcp_meta_cb_t cb, void *ctx);

/**
 * @brief Get the last known metadata snapshot.
 *
 * @param meta  Output snapshot.
 * @return true if a snapshot was filled.
 */
bool avrcp_hal_get_metadata(avrcp_meta_t *meta);

/** @} */
/** @} */

#endif /* #ifndef AVRCP_HAL_H */

/*==================[end of file]============================================*/
