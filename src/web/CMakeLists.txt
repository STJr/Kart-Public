add_custom_command(

    OUTPUT dist
    DEPENDS SRB2SDL2
)

file(COPY package.json DESTINATION .)
file(COPY index.html DESTINATION .)
file(COPY js DESTINATION .)
file(COPY elm DESTINATION .)
file(COPY elm.json DESTINATION .)
file(COPY webpack.config.js DESTINATION .)

add_custom_target(
    WEB ALL
    COMMAND npm install && OUTPUT_PATH="${CMAKE_BINARY_DIR}/bin" npx webpack
    DEPENDS webpack.config.js package.json elm.json
    BYPRODUCTS dist/*
)
