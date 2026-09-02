#ifndef BT_ACCESS_CONTROL_H
#define BT_ACCESS_CONTROL_H

/** @defgroup middleware Middleware
 *  @brief Layer of intermediate logical services.
 *  @{
 *  @defgroup bt_access_control_middleware BT Access Control Middleware
 *  @brief Bluetooth MAC whitelist/blacklist access control.
 *  @{
 *
 * @section genDesc General Description
 *
 * Manages a whitelist and blacklist of Bluetooth MAC addresses (BDAs).
 * Persisted in NVS via nvs_hal. Business logic only — no hardware
 * dependencies beyond NVS storage.
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

#define ACL_MAX_ENTRIES  16
#define ACL_BDA_LEN      6

/*==================[typedef]================================================*/

/**
 * @brief Access control list type.
 */
typedef enum {
    ACL_WHITELIST = 0,  /**< Only listed BDAs are allowed */
    ACL_BLACKLIST       /**< Listed BDAs are denied */
} acl_list_type_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize access control subsystem.
 *        Loads persisted lists from NVS.
 *
 * @return true if successful.
 */
bool AccessControlInit(void);

/**
 * @brief Check if a BDA is allowed to connect.
 *
 * Logic:
 * - If whitelist is empty and BDA not in blacklist → allowed
 * - If whitelist has entries and BDA in whitelist → allowed
 * - Otherwise → denied
 *
 * @param bda  Bluetooth Device Address (6 bytes).
 * @return true if connection is allowed.
 */
bool AccessControlIsAllowed(const uint8_t *bda);

/**
 * @brief Add a BDA to the specified list.
 *
 * @param bda        Bluetooth Device Address (6 bytes).
 * @param list_type  Whitelist or blacklist.
 * @return true if successful.
 */
bool AccessControlAdd(const uint8_t *bda, acl_list_type_t list_type);

/**
 * @brief Remove a BDA from both lists.
 *
 * @param bda  Bluetooth Device Address (6 bytes).
 * @return true if the BDA was found and removed.
 */
bool AccessControlRemove(const uint8_t *bda);

/**
 * @brief Check if a BDA is in the blacklist.
 *
 * @param bda  Bluetooth Device Address (6 bytes).
 * @return true if blacklisted.
 */
bool AccessControlIsBlacklisted(const uint8_t *bda);

/**
 * @brief Check if a BDA is in the whitelist.
 *
 * @param bda  Bluetooth Device Address (6 bytes).
 * @return true if whitelisted.
 */
bool AccessControlIsWhitelisted(const uint8_t *bda);

/** @} */
/** @} */

#endif /* #ifndef BT_ACCESS_CONTROL_H */

/*==================[end of file]============================================*/
