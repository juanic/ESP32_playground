#ifndef PHOTO_REPOSITORY_H
#define PHOTO_REPOSITORY_H

/** @defgroup middleware Middleware
 *  @brief Reusable middleware services.
 *  @{
 *  @defgroup photo_repository Photo repository
 *  @brief SD card layout for captured photos, detections and daily CSV logs.
 *  @{
 *
 * @section genDesc General Description
 *
 * Stores the files produced by the people counter on the SD card:
 *
 * @code
 * /sdcard/photos/YYYY-MM-DD/HH-MM-SS.jpg    original capture
 * /sdcard/photos/YYYY-MM-DD/HH-MM-SS.det    detection result (JSON)
 * /sdcard/data/YYYY-MM-DD.csv               one row per capture
 * @endcode
 *
 * The CSV columns are:
 * `timestamp,archivo,personas,confianza_media,confianza_min,confianza_max,procesado`
 *
 * All the paths are relative to a configurable root, normally `/sdcard`.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 09/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
/*==================[macros]=================================================*/

/** Maximum number of detections stored per photo. */
#define PHOTO_REPO_MAX_BOXES		32

/*==================[typedef]================================================*/

/**
 * @brief Summary of the detections of one photo.
 */
typedef struct {
	uint32_t count;				/**< Number of people above the threshold */
	float confidence_avg;		/**< Mean confidence, 0..1 */
	float confidence_min;		/**< Lowest confidence, 0..1 */
	float confidence_max;		/**< Highest confidence, 0..1 */
} photo_repo_detection_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Create the directory tree under the repository root.
 *
 * @param root Absolute mount point, normally "/sdcard".
 * @return ESP_OK on success or the error reported by mkdir().
 */
esp_err_t photo_repo_init(const char *root);

/**
 * @brief Build the file paths for a capture timestamp.
 *
 * @param ts      Unix time of the capture.
 * @param jpg     Output JPEG path.
 * @param jpg_len JPEG buffer size.
 * @param det     Output detection path (may be NULL).
 * @param det_len Detection buffer size.
 * @param csv     Output CSV path (may be NULL).
 * @param csv_len CSV buffer size.
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_build_paths(int64_t ts, char *jpg, size_t jpg_len,
								 char *det, size_t det_len,
								 char *csv, size_t csv_len);

/**
 * @brief Save a JPEG capture.
 *
 * Creates the day directory if it does not exist.
 *
 * @param path Destination path built with photo_repo_build_paths().
 * @param data JPEG bytes.
 * @param len  JPEG length.
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_save_jpeg(const char *path, const uint8_t *data, size_t len);

/**
 * @brief Save the detection result as JSON.
 *
 * @param path Destination path (the `.det` file).
 * @param json Null-terminated JSON document.
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_save_detection(const char *path, const char *json);

/**
 * @brief Append one row to the daily CSV, creating it with a header if needed.
 *
 * @param csv_path CSV path built with photo_repo_build_paths().
 * @param ts       Capture Unix time.
 * @param jpg_path Path of the photo, used to derive the file name.
 * @param det      Detection summary (may be NULL if not processed yet).
 * @param processed true when the photo was analysed by the detector.
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_append_csv(const char *csv_path, int64_t ts,
								const char *jpg_path,
								const photo_repo_detection_t *det,
								bool processed);

/**
 * @brief Insert or update the CSV row of one photo.
 *
 * The file is rewritten through a temporary copy. If a row for the photo
 * already exists (for example the "not processed yet" row written at
 * capture time) it is replaced, otherwise a new row is appended.
 *
 * @param csv_path CSV path built with photo_repo_build_paths().
 * @param ts       Capture Unix time.
 * @param jpg_path Path of the photo, used to derive the file name.
 * @param det      Detection summary with valid confidence values.
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_upsert_csv(const char *csv_path, int64_t ts,
								const char *jpg_path,
								const photo_repo_detection_t *det);

/**
 * @brief Check whether a file exists.
 */
bool photo_repo_exists(const char *path);

/**
 * @brief Find the most recent capture.
 *
 * @param jpg_path Output path.
 * @param len      Output buffer size.
 * @param ts       Output capture time (may be NULL).
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if there are no photos.
 */
esp_err_t photo_repo_find_latest(char *jpg_path, size_t len, int64_t *ts);

/**
 * @brief Find the oldest capture that has no `.det` file yet.
 *
 * @param jpg_path Output path.
 * @param len      Output buffer size.
 * @param ts       Output capture time (may be NULL).
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if everything was processed.
 */
esp_err_t photo_repo_find_pending(char *jpg_path, size_t len, int64_t *ts);

/**
 * @brief Build a JSON listing of the stored days.
 *
 * Format: `{"days":[{"day":"YYYY-MM-DD","files":12}]}`
 *
 * @param buf Output buffer.
 * @param len Output buffer size.
 * @return ESP_OK on success, ESP_ERR_INVALID_SIZE if the listing was truncated.
 */
esp_err_t photo_repo_list_days_json(char *buf, size_t len);

/**
 * @brief Build a JSON listing of the files of one day.
 *
 * Format: `{"day":"YYYY-MM-DD","files":[{"name":"HH-MM-SS.jpg","size":123,"det":1}]}`
 *
 * @param day Day in `YYYY-MM-DD` format.
 * @param buf Output buffer.
 * @param len Output buffer size.
 * @return ESP_OK on success, ESP_ERR_INVALID_SIZE if the listing was truncated.
 */
esp_err_t photo_repo_list_day_json(const char *day, char *buf, size_t len);

/**
 * @brief Read a detection JSON file into memory.
 *
 * @param det_path Path of the `.det` file.
 * @param buf      Output buffer.
 * @param len      Output buffer size.
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if the file does not exist.
 */
esp_err_t photo_repo_read_detection(const char *det_path, char *buf, size_t len);

/**
 * @brief Delete a single capture, its detection file, and its CSV row.
 *
 * @param day       Day in YYYY-MM-DD format.
 * @param file_name File name (e.g. HH-MM-SS.jpg).
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_delete_photo(const char *day, const char *file_name);

/**
 * @brief Delete an entire day: all photos, detections, day folder and CSV.
 *
 * @param day Day in YYYY-MM-DD format.
 * @return ESP_OK on success.
 */
esp_err_t photo_repo_delete_day(const char *day);

/** @} */
/** @} */

#endif /* #ifndef PHOTO_REPOSITORY_H */

/*==================[end of file]============================================*/
