project(SRB2
	VERSION 1.6)

add_custom_target(SRB2WEB
	COMMAND webpack
)

install(TARGETS SRB2WEB RUNTIME DESTINATION .)
