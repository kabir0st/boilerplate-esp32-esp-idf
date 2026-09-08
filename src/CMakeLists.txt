# Not named "main", so component dependencies must be listed explicitly.
# freertos and log are implicit (common requirements); gpio is not.
idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio)
