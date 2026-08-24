/**
 * @file alarm_manager.c
 * @brief [P2-M7] [P2-M8] Alarm Manager with Atomic Write config persistence and Buzzer Pulsing Controller
 * @author PHUC TAI
 */

#include "alarm_manager.h"
#include "../../include/smartclock_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define DEFAULT_ALARM_HOUR      7
#define DEFAULT_ALARM_MINUTE    0

/* Track last triggered minute to prevent repeated firing in the same minute */
static int s_last_triggered_minute = -1;

int alarm_manager_load_config(const char *config_path, alarm_config_t *out_cfg)
{
    if (!config_path || !out_cfg)
        return -EINVAL;

    FILE *fp = fopen(config_path, "r");
    if (!fp) {
        /* If file does not exist, initialize with default disabled values */
        out_cfg->hour = DEFAULT_ALARM_HOUR;
        out_cfg->minute = DEFAULT_ALARM_MINUTE;
        out_cfg->enabled = false;
        return -ENOENT;
    }

    char line[64];
    int hour = DEFAULT_ALARM_HOUR, min = DEFAULT_ALARM_MINUTE, en = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "hour=%d", &hour) == 1) continue;
        if (sscanf(line, "minute=%d", &min) == 1) continue;
        if (sscanf(line, "enabled=%d", &en) == 1) continue;
    }

    fclose(fp);

    out_cfg->hour = (hour >= 0 && hour <= 23) ? hour : DEFAULT_ALARM_HOUR;
    out_cfg->minute = (min >= 0 && min <= 59) ? min : DEFAULT_ALARM_MINUTE;
    out_cfg->enabled = (en != 0);

    printf("[alarm_manager] Loaded config: %02d:%02d (Enabled: %d)\n",
           out_cfg->hour, out_cfg->minute, out_cfg->enabled);

    return 0;
}

/* [P2-M7] Save alarm configuration to persistent storage using Atomic File Write Pattern (.tmp -> fsync -> rename) */
int alarm_manager_save_config_atomic(const char *config_path, const alarm_config_t *cfg)
{
    if (!config_path || !cfg)
        return -EINVAL;

    /* 1. Open temporary file */
    FILE *fp = fopen(ALARM_CONFIG_TMP_FILE, "w");
    if (!fp) {
        perror("alarm_manager: Failed to open temporary config file");
        return -errno;
    }

    /* 2. Write key-value configuration */
    fprintf(fp, "hour=%d\nminute=%d\nenabled=%d\n",
            cfg->hour, cfg->minute, cfg->enabled ? 1 : 0);

    /* 3. Flush userspace buffer and commit to physical storage */
    fflush(fp);
    if (fsync(fileno(fp)) < 0) {
        perror("alarm_manager: fsync failed");
        fclose(fp);
        unlink(ALARM_CONFIG_TMP_FILE);
        return -errno;
    }

    fclose(fp);

    /* 4. Atomic Rename to replace active config */
    if (rename(ALARM_CONFIG_TMP_FILE, config_path) < 0) {
        perror("alarm_manager: Failed to atomic rename config file");
        unlink(ALARM_CONFIG_TMP_FILE);
        return -errno;
    }

    printf("[alarm_manager] Successfully saved alarm config atomically to %s\n", config_path);
    return 0;
}

/* [P2-M8] Check current system time against configured alarm and trigger buzzer */
void alarm_manager_check(const struct tm *tm_info)
{
    if (!tm_info) return;

    pthread_mutex_lock(&g_state_mutex);
    alarm_config_t cfg = g_system_state.alarm_config;
    bool is_currently_ringing = g_system_state.alarm_ringing;
    pthread_mutex_unlock(&g_state_mutex);

    /* Reset trigger tracker when minute changes */
    if (tm_info->tm_min != s_last_triggered_minute) {
        s_last_triggered_minute = -1;
    }

    if (!cfg.enabled || is_currently_ringing) {
        return;
    }

    /* Check if current hour & minute match alarm schedule */
    if (tm_info->tm_hour == cfg.hour && tm_info->tm_min == cfg.minute) {
        if (s_last_triggered_minute != tm_info->tm_min) {
            s_last_triggered_minute = tm_info->tm_min;

            pthread_mutex_lock(&g_state_mutex);
            g_system_state.alarm_ringing = true;
            pthread_mutex_unlock(&g_state_mutex);

            printf("[alarm_manager] ALARM TRIGGERED! Time: %02d:%02d:00\n",
                   tm_info->tm_hour, tm_info->tm_min);
        }
    }
}

void *buzzer_thread_func(void *arg)
{
    (void)arg;
    int bz_fd = -1;

    printf("[buzzer_thread] Buzzer controller thread started.\n");

    while (1) {
        pthread_mutex_lock(&g_state_mutex);
        bool is_running = g_system_state.running;
        bool should_ring = g_system_state.alarm_ringing;
        pthread_mutex_unlock(&g_state_mutex);

        if (!is_running) {
            break;
        }

        if (should_ring) {
            /* Open character driver if not already opened */
            if (bz_fd < 0) {
                bz_fd = open(BUZZER_DEV_PATH, O_WRONLY);
                if (bz_fd < 0) {
                    perror("buzzer_thread: Failed to open buzzer driver");
                    usleep(500 * 1000);
                    continue;
                }
            }

            /* Beep Pattern: 500ms ON / 500ms OFF */
            if (write(bz_fd, "1", 1) < 0) perror("buzzer write on");
            usleep(500 * 1000);

            /* Check again before sleeping low to respond quickly to button press */
            pthread_mutex_lock(&g_state_mutex);
            should_ring = g_system_state.alarm_ringing;
            pthread_mutex_unlock(&g_state_mutex);

            if (should_ring) {
                if (write(bz_fd, "0", 1) < 0) perror("buzzer write off");
                usleep(500 * 1000);
            }
        } else {
            /* Turn off buzzer and close file descriptor when idle */
            if (bz_fd >= 0) {
                if (write(bz_fd, "0", 1) < 0) perror("buzzer write off");
                close(bz_fd);
                bz_fd = -1;
            }
            /* Sleep idle cycle */
            usleep(200 * 1000);
        }
    }

    if (bz_fd >= 0) {
        if (write(bz_fd, "0", 1) < 0) perror("buzzer write off");
        close(bz_fd);
    }

    printf("[buzzer_thread] Thread safely terminated.\n");
    return NULL;
}