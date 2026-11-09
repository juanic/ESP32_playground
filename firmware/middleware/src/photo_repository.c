/*! @file photo_repository.c
 *  @brief SD card photo storage and daily CSV log.
 *  @defgroup photo_repository impl.
 *  @{
 *
 * Implementa el almacenamiento del contador de personas sobre FATFS:
 * `photos/YYYY-MM-DD/HH-MM-SS.jpg` para las capturas, un `.det` JSON por foto
 * y un CSV por día con una fila por captura.
 */

/*==================[inclusions]=============================================*/
#include "photo_repository.h"
#include "esp_log.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
/*==================[macros and definitions]=================================*/

#define PHOTO_REPO_MAX_DAYS		64
#define PHOTO_REPO_MAX_FILES	128
#define PHOTO_REPO_PATH_MAX		192

/*==================[internal data definition]===============================*/

static const char *TAG = "photo_repo";
static char s_root[32] = "/sdcard";

/*==================[internal functions declaration]=========================*/

static esp_err_t ensure_dir(const char *path);
static bool is_day_name(const char *name);
static bool is_jpg_name(const char *name);
static bool parse_ts(const char *day, const char *name, int64_t *ts);
static void format_timestamp(int64_t ts, char *buf, size_t len);
static const char *base_name(const char *path);
static int cmp_str_desc(const void *a, const void *b);

/*==================[internal functions definition]==========================*/

static esp_err_t ensure_dir(const char *path) {
	if (mkdir(path, 0777) == 0 || errno == EEXIST) {
		return ESP_OK;
	}
	ESP_LOGE(TAG, "No se pudo crear '%s' (errno=%d)", path, errno);
	return ESP_FAIL;
}

static bool is_day_name(const char *name) {
	return strlen(name) == 10 && name[4] == '-' && name[7] == '-';
}

static bool is_jpg_name(const char *name) {
	/* Captures are always named "HH-MM-SS.jpg". */
	return strlen(name) == 12 && strcmp(name + 8, ".jpg") == 0;
}

static void copy_fixed(char *dst, const char *src, size_t count) {
	memcpy(dst, src, count);
	dst[count] = '\0';
}

static bool parse_ts(const char *day, const char *name, int64_t *ts) {
	int year, month, mday, hour, minute, second;
	if (sscanf(day, "%d-%d-%d", &year, &month, &mday) != 3) {
		return false;
	}
	if (sscanf(name, "%d-%d-%d", &hour, &minute, &second) != 3) {
		return false;
	}

	struct tm tm = {0};
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = mday;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;
	tm.tm_isdst = -1;

	time_t value = mktime(&tm);
	if (value == (time_t)-1) {
		return false;
	}
	*ts = (int64_t)value;
	return true;
}

static void format_timestamp(int64_t ts, char *buf, size_t len) {
	time_t value = (time_t)ts;
	struct tm tm;
	localtime_r(&value, &tm);
	strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tm);
}

static const char *base_name(const char *path) {
	const char *slash = strrchr(path, '/');
	return slash != NULL ? slash + 1 : path;
}

static int cmp_str_desc(const void *a, const void *b) {
	return strcmp((const char *)b, (const char *)a);
}

/*==================[external functions definition]==========================*/

esp_err_t photo_repo_init(const char *root) {
	if (root == NULL || root[0] == '\0') {
		return ESP_ERR_INVALID_ARG;
	}

	snprintf(s_root, sizeof(s_root), "%s", root);

	char path[PHOTO_REPO_PATH_MAX];
	esp_err_t err = ensure_dir(s_root);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(path, sizeof(path), "%s/photos", s_root);
	err = ensure_dir(path);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(path, sizeof(path), "%s/data", s_root);
	err = ensure_dir(path);
	if (err != ESP_OK) {
		return err;
	}

	ESP_LOGI(TAG, "Repositorio listo en %s", s_root);
	return ESP_OK;
}

esp_err_t photo_repo_build_paths(int64_t ts, char *jpg, size_t jpg_len,
								 char *det, size_t det_len,
								 char *csv, size_t csv_len) {
	if (jpg == NULL || jpg_len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	time_t value = (time_t)ts;
	struct tm tm;
	localtime_r(&value, &tm);

	char day[16];
	char clock[16];
	strftime(day, sizeof(day), "%Y-%m-%d", &tm);
	strftime(clock, sizeof(clock), "%H-%M-%S", &tm);

	int written = snprintf(jpg, jpg_len, "%s/photos/%s/%s.jpg", s_root, day, clock);
	if (written < 0 || (size_t)written >= jpg_len) {
		return ESP_ERR_INVALID_SIZE;
	}
	if (det != NULL && det_len > 0) {
		snprintf(det, det_len, "%s/photos/%s/%s.det", s_root, day, clock);
	}
	if (csv != NULL && csv_len > 0) {
		snprintf(csv, csv_len, "%s/data/%s.csv", s_root, day);
	}
	return ESP_OK;
}

esp_err_t photo_repo_save_jpeg(const char *path, const uint8_t *data, size_t len) {
	if (path == NULL || data == NULL || len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	char dir[PHOTO_REPO_PATH_MAX];
	snprintf(dir, sizeof(dir), "%s", path);
	char *slash = strrchr(dir, '/');
	if (slash != NULL) {
		*slash = '\0';
		if (ensure_dir(dir) != ESP_OK) {
			return ESP_FAIL;
		}
	}

	FILE *file = fopen(path, "wb");
	if (file == NULL) {
		ESP_LOGE(TAG, "No se pudo abrir '%s' para escritura", path);
		return ESP_FAIL;
	}

	size_t written = fwrite(data, 1, len, file);
	fclose(file);

	if (written != len) {
		ESP_LOGE(TAG, "Escritura incompleta en '%s' (%u/%u)",
				 path, (unsigned)written, (unsigned)len);
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t photo_repo_save_detection(const char *path, const char *json) {
	if (path == NULL || json == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	FILE *file = fopen(path, "w");
	if (file == NULL) {
		ESP_LOGE(TAG, "No se pudo abrir '%s' para escritura", path);
		return ESP_FAIL;
	}

	size_t len = strlen(json);
	size_t written = fwrite(json, 1, len, file);
	fclose(file);
	return written == len ? ESP_OK : ESP_FAIL;
}

esp_err_t photo_repo_append_csv(const char *csv_path, int64_t ts,
								const char *jpg_path,
								const photo_repo_detection_t *det,
								bool processed) {
	if (csv_path == NULL || jpg_path == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	char dir[PHOTO_REPO_PATH_MAX];
	snprintf(dir, sizeof(dir), "%s", csv_path);
	char *slash = strrchr(dir, '/');
	if (slash != NULL) {
		*slash = '\0';
		if (ensure_dir(dir) != ESP_OK) {
			return ESP_FAIL;
		}
	}

	struct stat st;
	bool exists = (stat(csv_path, &st) == 0);

	FILE *file = fopen(csv_path, "a");
	if (file == NULL) {
		ESP_LOGE(TAG, "No se pudo abrir '%s' para añadir", csv_path);
		return ESP_FAIL;
	}

	if (!exists) {
		fprintf(file, "timestamp,archivo,personas,confianza_media,"
					  "confianza_min,confianza_max,procesado\n");
	}

	char timestamp[24];
	format_timestamp(ts, timestamp, sizeof(timestamp));
	const char *name = base_name(jpg_path);

	if (processed && det != NULL) {
		fprintf(file, "%s,%s,%u,%.1f,%.1f,%.1f,1\n",
				timestamp, name, (unsigned)det->count,
				det->confidence_avg * 100.0f,
				det->confidence_min * 100.0f,
				det->confidence_max * 100.0f);
	} else {
		fprintf(file, "%s,%s,0,,,,0\n", timestamp, name);
	}

	fclose(file);
	return ESP_OK;
}

esp_err_t photo_repo_upsert_csv(const char *csv_path, int64_t ts,
								const char *jpg_path,
								const photo_repo_detection_t *det) {
	if (csv_path == NULL || jpg_path == NULL || det == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	char dir[PHOTO_REPO_PATH_MAX];
	snprintf(dir, sizeof(dir), "%s", csv_path);
	char *slash = strrchr(dir, '/');
	if (slash != NULL) {
		*slash = '\0';
		if (ensure_dir(dir) != ESP_OK) {
			return ESP_FAIL;
		}
	}

	char timestamp[24];
	format_timestamp(ts, timestamp, sizeof(timestamp));
	const char *name = base_name(jpg_path);

	char row[160];
	int row_len = snprintf(row, sizeof(row), "%s,%s,%u,%.1f,%.1f,%.1f,1\n",
						   timestamp, name, (unsigned)det->count,
						   det->confidence_avg * 100.0f,
						   det->confidence_min * 100.0f,
						   det->confidence_max * 100.0f);
	if (row_len < 0 || (size_t)row_len >= sizeof(row)) {
		return ESP_ERR_INVALID_SIZE;
	}

	char tmp_path[PHOTO_REPO_PATH_MAX + 8];
	if (snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", csv_path) >=
		(int)sizeof(tmp_path)) {
		return ESP_ERR_INVALID_SIZE;
	}

	FILE *out = fopen(tmp_path, "w");
	if (out == NULL) {
		ESP_LOGE(TAG, "No se pudo abrir '%s' para escritura", tmp_path);
		return ESP_FAIL;
	}

	bool header_written = false;
	bool replaced = false;
	char line[256];

	FILE *in = fopen(csv_path, "r");
	if (in != NULL) {
		while (fgets(line, sizeof(line), in) != NULL) {
			header_written = true;
			if (!replaced && strstr(line, name) != NULL) {
				fputs(row, out);
				replaced = true;
			} else {
				fputs(line, out);
			}
		}
		fclose(in);
	}

	if (!header_written) {
		fputs("timestamp,archivo,personas,confianza_media,"
			  "confianza_min,confianza_max,procesado\n", out);
	}
	if (!replaced) {
		fputs(row, out);
	}

	if (fclose(out) != 0) {
		remove(tmp_path);
		return ESP_FAIL;
	}

	remove(csv_path);
	if (rename(tmp_path, csv_path) != 0) {
		ESP_LOGE(TAG, "No se pudo reemplazar '%s'", csv_path);
		return ESP_FAIL;
	}
	return ESP_OK;
}

bool photo_repo_exists(const char *path) {
	if (path == NULL) {
		return false;
	}
	struct stat st;
	return stat(path, &st) == 0;
}

esp_err_t photo_repo_read_detection(const char *det_path, char *buf, size_t len) {
	if (det_path == NULL || buf == NULL || len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	FILE *file = fopen(det_path, "r");
	if (file == NULL) {
		return ESP_ERR_NOT_FOUND;
	}

	size_t read = fread(buf, 1, len - 1, file);
	fclose(file);
	buf[read] = '\0';
	return ESP_OK;
}

static esp_err_t scan_photos(bool latest, char *jpg_path, size_t len, int64_t *ts) {
	char photos_dir[48];
	snprintf(photos_dir, sizeof(photos_dir), "%s/photos", s_root);

	DIR *root = opendir(photos_dir);
	if (root == NULL) {
		return ESP_ERR_NOT_FOUND;
	}

	char best_name[24] = "";
	bool found = false;

	struct dirent *day_entry;
	while ((day_entry = readdir(root)) != NULL) {
		if (!is_day_name(day_entry->d_name)) {
			continue;
		}

		char day_name[11];
		copy_fixed(day_name, day_entry->d_name, 10);

		char day_dir[64];
		snprintf(day_dir, sizeof(day_dir), "%s/%s", photos_dir, day_name);
		DIR *day = opendir(day_dir);
		if (day == NULL) {
			continue;
		}

		struct dirent *file_entry;
		while ((file_entry = readdir(day)) != NULL) {
			if (!is_jpg_name(file_entry->d_name)) {
				continue;
			}

			char file_name[13];
			copy_fixed(file_name, file_entry->d_name, 12);

			char candidate[24];
			snprintf(candidate, sizeof(candidate), "%s/%s",
					 day_name, file_name);

			if (!latest) {
				char det_path[96];
				snprintf(det_path, sizeof(det_path), "%s/%s", day_dir,
						 file_name);
				char *dot = strrchr(det_path, '.');
				if (dot != NULL) {
					snprintf(dot, sizeof(det_path) - (size_t)(dot - det_path), ".det");
				}
				struct stat st;
				if (stat(det_path, &st) == 0) {
					continue;
				}
			}

			const char *best = found ? best_name : NULL;
			int order = best == NULL ? 0 : strcmp(candidate, best);
			bool take = found ? (latest ? order > 0 : order < 0) : true;
			if (take) {
				found = true;
				snprintf(best_name, sizeof(best_name), "%s", candidate);
			}
		}
		closedir(day);
	}
	closedir(root);

	/* best_name is "YYYY-MM-DD/HH-MM-SS.jpg" */
	if (!found) {
		return ESP_ERR_NOT_FOUND;
	}

	char day[16];
	char name[32];
	if (sscanf(best_name, "%15[^/]/%31s", day, name) != 2) {
		return ESP_ERR_INVALID_STATE;
	}

	snprintf(jpg_path, len, "%s/photos/%s", s_root, best_name);
	if (ts != NULL && !parse_ts(day, name, ts)) {
		*ts = 0;
	}
	return ESP_OK;
}

esp_err_t photo_repo_find_latest(char *jpg_path, size_t len, int64_t *ts) {
	return scan_photos(true, jpg_path, len, ts);
}

esp_err_t photo_repo_find_pending(char *jpg_path, size_t len, int64_t *ts) {
	return scan_photos(false, jpg_path, len, ts);
}

esp_err_t photo_repo_list_days_json(char *buf, size_t len) {
	if (buf == NULL || len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	char photos_dir[48];
	snprintf(photos_dir, sizeof(photos_dir), "%s/photos", s_root);

	DIR *root = opendir(photos_dir);
	if (root == NULL) {
		snprintf(buf, len, "{\"days\":[]}");
		return ESP_OK;
	}

	char days[PHOTO_REPO_MAX_DAYS][16];
	size_t day_count = 0;
	struct dirent *entry;
	while ((entry = readdir(root)) != NULL && day_count < PHOTO_REPO_MAX_DAYS) {
		if (is_day_name(entry->d_name)) {
			copy_fixed(days[day_count], entry->d_name, 10);
			day_count++;
		}
	}
	closedir(root);

	qsort(days, day_count, sizeof(days[0]), cmp_str_desc);

	size_t used = 0;
	bool truncated = false;
	int written = snprintf(buf, len, "{\"days\":[");
	if (written < 0 || (size_t)written >= len) {
		return ESP_ERR_INVALID_SIZE;
	}
	used = (size_t)written;

	for (size_t i = 0; i < day_count; i++) {
		char day_dir[64];
		snprintf(day_dir, sizeof(day_dir), "%s/%s", photos_dir, days[i]);

		unsigned files = 0;
		DIR *day = opendir(day_dir);
		if (day != NULL) {
			struct dirent *file_entry;
			while ((file_entry = readdir(day)) != NULL) {
				if (is_jpg_name(file_entry->d_name)) {
					files++;
				}
			}
			closedir(day);
		}

		written = snprintf(buf + used, len - used,
						   "%s{\"day\":\"%s\",\"files\":%u}",
						   i > 0 ? "," : "", days[i], files);
		if (written < 0 || (size_t)written >= len - used) {
			truncated = true;
			break;
		}
		used += (size_t)written;
	}

	if (!truncated) {
		snprintf(buf + used, len - used, "]}");
		return ESP_OK;
	}

	snprintf(buf, len, "{\"days\":[],\"truncated\":true}");
	return ESP_ERR_INVALID_SIZE;
}

esp_err_t photo_repo_list_day_json(const char *day, char *buf, size_t len) {
	if (day == NULL || !is_day_name(day) || buf == NULL || len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	char day_dir[64];
	snprintf(day_dir, sizeof(day_dir), "%s/photos/%s", s_root, day);

	DIR *dir = opendir(day_dir);
	if (dir == NULL) {
		snprintf(buf, len, "{\"day\":\"%s\",\"files\":[]}", day);
		return ESP_OK;
	}

	size_t used = 0;
	bool first = true;
	bool truncated = false;
	int written = snprintf(buf, len, "{\"day\":\"%s\",\"files\":[", day);
	if (written < 0 || (size_t)written >= len) {
		closedir(dir);
		return ESP_ERR_INVALID_SIZE;
	}
	used = (size_t)written;

	struct dirent *file_entry;
	while ((file_entry = readdir(dir)) != NULL) {
		if (!is_jpg_name(file_entry->d_name)) {
			continue;
		}

		char file_name[13];
		copy_fixed(file_name, file_entry->d_name, 12);

		char jpg_path[PHOTO_REPO_PATH_MAX];
		snprintf(jpg_path, sizeof(jpg_path), "%s/%s", day_dir, file_name);
		struct stat st;
		long size = (stat(jpg_path, &st) == 0) ? (long)st.st_size : 0;

		char det_path[PHOTO_REPO_PATH_MAX];
		snprintf(det_path, sizeof(det_path), "%s", jpg_path);
		char *dot = strrchr(det_path, '.');
		bool has_det = false;
		if (dot != NULL) {
			snprintf(dot, sizeof(det_path) - (size_t)(dot - det_path), ".det");
			has_det = photo_repo_exists(det_path);
		}

		written = snprintf(buf + used, len - used,
						   "%s{\"name\":\"%s\",\"size\":%ld,\"det\":%d}",
						   first ? "" : ",", file_name, size,
						   has_det ? 1 : 0);
		if (written < 0 || (size_t)written >= len - used) {
			truncated = true;
			break;
		}
		used += (size_t)written;
		first = false;
	}
	closedir(dir);

	if (truncated) {
		snprintf(buf, len, "{\"day\":\"%s\",\"files\":[],\"truncated\":true}", day);
		return ESP_ERR_INVALID_SIZE;
	}

	snprintf(buf + used, len - used, "]}");
	return ESP_OK;
}

esp_err_t photo_repo_delete_photo(const char *day, const char *file_name) {
	if (day == NULL || !is_day_name(day) || file_name == NULL || !is_jpg_name(file_name)) {
		return ESP_ERR_INVALID_ARG;
	}

	char path[PHOTO_REPO_PATH_MAX];
	snprintf(path, sizeof(path), "%s/photos/%s/%s", s_root, day, file_name);
	remove(path);

	char det_path[PHOTO_REPO_PATH_MAX];
	snprintf(det_path, sizeof(det_path), "%s/photos/%s/%s", s_root, day, file_name);
	char *dot = strrchr(det_path, '.');
	if (dot != NULL) {
		snprintf(dot, sizeof(det_path) - (size_t)(dot - det_path), ".det");
		remove(det_path);
	}

	char csv_path[PHOTO_REPO_PATH_MAX];
	snprintf(csv_path, sizeof(csv_path), "%s/data/%s.csv", s_root, day);
	char tmp_path[PHOTO_REPO_PATH_MAX + 8];
	snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", csv_path);

	FILE *in = fopen(csv_path, "r");
	if (in != NULL) {
		FILE *out = fopen(tmp_path, "w");
		if (out != NULL) {
			char line[256];
			while (fgets(line, sizeof(line), in) != NULL) {
				if (strstr(line, file_name) == NULL) {
					fputs(line, out);
				}
			}
			fclose(out);
			fclose(in);
			remove(csv_path);
			rename(tmp_path, csv_path);
		} else {
			fclose(in);
		}
	}

	char day_dir[64];
	snprintf(day_dir, sizeof(day_dir), "%s/photos/%s", s_root, day);
	rmdir(day_dir);

	return ESP_OK;
}

esp_err_t photo_repo_delete_day(const char *day) {
	if (day == NULL || !is_day_name(day)) {
		return ESP_ERR_INVALID_ARG;
	}

	char day_dir[64];
	snprintf(day_dir, sizeof(day_dir), "%s/photos/%s", s_root, day);

	DIR *dir = opendir(day_dir);
	if (dir != NULL) {
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.') {
				continue;
			}
			char file_path[320];
			snprintf(file_path, sizeof(file_path), "%s/%s", day_dir, entry->d_name);
			remove(file_path);
		}
		closedir(dir);
		rmdir(day_dir);
	}

	char csv_path[PHOTO_REPO_PATH_MAX];
	snprintf(csv_path, sizeof(csv_path), "%s/data/%s.csv", s_root, day);
	remove(csv_path);

	return ESP_OK;
}

/*==================[end of file]============================================*/
