
zephyr_include_directories_ifdef(CONFIG_FRAMEWORK include)
zephyr_sources_ifdef(CONFIG_FRAMEWORK
  source/BufferPool.c
  source/Framework.c
  source/FrameworkMsg.c
  source/FrameworkStubs.c
)

zephyr_sources_ifdef(CONFIG_BUFFER_POOL_SHELL
  source/BufferPoolShell.c
)

if((CONFIG_FRAMEWORK) AND(CONFIG_FWK_AUTO_GENERATE_FILES))
  if((CONFIG_FWK_SENSOR))
    add_fwk_msgcode_file(${CMAKE_CURRENT_SOURCE_DIR}/framework/sensor_msgcodes.h)
  endif()

  include(cmake/framework_gen.cmake)
endif()
