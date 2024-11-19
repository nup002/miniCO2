idf_component_register(SRCS "constants.cpp" "measurement.cpp" "advertisement.cpp"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES bt mbedtls)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra)
