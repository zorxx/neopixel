set(priv_requires)
# Starting from esp-idf v5.3, the peripheral drivers are in separate components
if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.3")
    list(APPEND priv_requires "esp_driver_i2s")
else()
    list(APPEND priv_requires "driver")
endif()
idf_component_register(SRCS "neopixel.c" INCLUDE_DIRS "." PRIV_REQUIRES ${priv_requires})