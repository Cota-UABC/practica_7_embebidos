idf_component_register(SRCS "main.c"
                    SRCS "web_server.c"
                    SRCS "gpio.c"
                    SRCS "i2c.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "view.html")