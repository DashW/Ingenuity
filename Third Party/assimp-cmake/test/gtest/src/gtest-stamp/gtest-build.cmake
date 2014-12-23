

set(command "C:/Program Files (x86)/CMake/bin/cmake.exe;--build;C:/Users/Richard/Documents/IngenuityGitHub/Third Party/assimp-cmake/test/gtest/src/gtest-build;--config;${config}")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "C:/Users/Richard/Documents/IngenuityGitHub/Third Party/assimp-cmake/test/gtest/src/gtest-stamp/gtest-build-out.log"
  ERROR_FILE "C:/Users/Richard/Documents/IngenuityGitHub/Third Party/assimp-cmake/test/gtest/src/gtest-stamp/gtest-build-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  C:/Users/Richard/Documents/IngenuityGitHub/Third Party/assimp-cmake/test/gtest/src/gtest-stamp/gtest-build-*.log\n")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "gtest build command succeeded.  See also C:/Users/Richard/Documents/IngenuityGitHub/Third Party/assimp-cmake/test/gtest/src/gtest-stamp/gtest-build-*.log\n")
  message(STATUS "${msg}")
endif()
