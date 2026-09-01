idf_component_register(SRCS "main.c" "Classifier.c"
                    INCLUDE_DIRS "."
                    REQUIRES board_support drivers_hal esp_timer)
