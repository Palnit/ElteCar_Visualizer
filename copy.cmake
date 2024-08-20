get_filename_component(dir ${file} DIRECTORY)
get_filename_component(name ${file} NAME)
message("Copying ${name}")
file(COPY ${SOURCE_DIR}/${file} DESTINATION ${DESTINATION_DIR}/${dir})
