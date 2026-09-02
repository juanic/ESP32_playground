#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

/** @defgroup middleware Middleware
 *  @brief Layer of intermediate logical services.
 *  @{
 *  @defgroup relay_control Relay Control Middleware
 *  @brief Control-plane logic for the audio relay sink (App-driven).
 *  @{
 *
 * @section genDesc General Description
 *
 * Implements the control-plane of the audio relay: a remote App connects
 * over BLE (GATT control service) and sends text commands to manage the
 * relay — enable/disable audio transmission, filter MACs, disconnect
 * the A2DP peer, view metadata, and change the access password.
 *
 * Security model: mutating commands require the App to authenticate
 * first with a password ("AUTH <password>"). Read-only queries
 * ("STATUS", "GET_META", "LIST") are allowed without authentication.
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

#define RELAY_CONTROL_PASSWORD_MAX_LEN   32
#define RELAY_CONTROL_PASSWORD_DEFAULT   "relay1234"

/** Max number of concurrent time-granted MAC entries. */
#define RELAY_CONTROL_MAX_GRANTS         8

/*==================[typedef]================================================*/

/**
 * @brief A time grant for a specific Bluetooth MAC.
 *
 * While a grant is active, the MAC may play; once the grant expires the
 * MAC is treated as denied regardless of whitelist/blacklist membership.
 */
typedef struct {
    uint8_t  bda[6];   /**< Bluetooth Device Address */
    int64_t  expiry;   /**< Unix time (s) when the grant expires */
} relay_grant_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize the relay control subsystem.
 *
 * Loads the access password from NVS (or stores the default), hooks the
 * BLE control transport, AVRCP metadata and the A2DP connection callback.
 *
 * @return true if successful.
 */
bool RelayControlInit(void);

/**
 * @brief True if audio transmission to the output is currently enabled.
 *
 * The sink data path must check this before forwarding audio.
 *
 * @return true if transmission is enabled.
 */
bool RelayControlIsTxEnabled(void);

/**
 * @brief Set/overwrite the access password.
 *
 * @param password  New password (max RELAY_CONTROL_PASSWORD_MAX_LEN-1).
 * @return true if stored successfully.
 */
bool RelayControlSetPassword(const char *password);

/**
 * @brief Decide whether a connected peer is allowed to play audio.
 *
 * Combines the existing MAC access control (whitelist/blacklist) with
 * the per-MAC time grant:
 *   - If the peer is not allowed by the MAC list -> deny.
 *   - Else if the peer has a time grant that has already expired -> deny.
 *   - Else allow.
 *
 * A peer with no grant is governed purely by the MAC list.
 *
 * @param bda  Bluetooth Device Address (6 bytes).
 * @return true if the peer may play.
 */
bool RelayControlIsAllowedToPlay(const uint8_t *bda);

/** @} */
/** @} */

#endif /* #ifndef RELAY_CONTROL_H */

/*==================[end of file]============================================*/
