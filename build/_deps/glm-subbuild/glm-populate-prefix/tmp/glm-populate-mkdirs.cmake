# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/31009/Desktop/OffScreenRender/extern/glm")
  file(MAKE_DIRECTORY "C:/Users/31009/Desktop/OffScreenRender/extern/glm")
endif()
file(MAKE_DIRECTORY
  "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-build"
  "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix"
  "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix/tmp"
  "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp"
  "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix/src"
  "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/31009/Desktop/OffScreenRender/build/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
