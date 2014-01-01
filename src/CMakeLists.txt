add_executable(test_vec4 test_vec4.c)

add_executable(test_mat4 test_mat4.c)

add_executable(test_gl test_gl.c)
target_link_libraries(test_gl ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})

install(TARGETS test_vec4 test_mat4 test_gl DESTINATION "${SSE_Stuff_SOURCE_DIR}/bin")

