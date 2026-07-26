idf_component_register(SRCS "app.c"
                                   "utils/dotenv.c"
                                   "utils/wifi_connect.c"
                                   "utils/web_util.c"
                                   "utils/web_API.c"
                    INCLUDE_DIRS "."
                                "utils"
                    EMBED_FILES "web/index.html"
                                "web/style.css"
                                "web/app.js"
                    REQUIRES nvs_flash esp_wifi esp_netif esp_event esp_http_server cjson)

# EMBED_FILES skips dotfiles — embed .env manually with objcopy
set(dotenv_obj "${CMAKE_CURRENT_BINARY_DIR}/dotenv.o")
add_custom_command(
    OUTPUT ${dotenv_obj}
    COMMAND ${CMAKE_OBJCOPY}
            -I binary -O elf32-xtensa-le -B xtensa
            --rename-section .data=.rodata.dotenv,contents,alloc,load,readonly,data
            .env ${dotenv_obj}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.env"
    COMMENT "Embedding .env -> dotenv.o"
)
target_sources(${COMPONENT_LIB} PRIVATE ${dotenv_obj})
