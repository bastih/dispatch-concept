include "toolset.lua"
include "clang.lua"

solution "Dispatch"
  configurations { "ReleaseFLTO", "Release", "ReleaseClang" }
  language "c++"
  p = path.getdirectory("")
  p1 = path.getdirectory("thirdparty/catch/single_include/")
  includedirs { p, p1 }
  buildoptions { "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter", "-ggdb" }
  linkoptions { "-ggdb" }
  location "."

  local proc = os.outputof("papi_avail | grep Yes")
  if string.len(proc) > 0 then
    defines {"USE_PAPI_TRACE"}
    libdirs {"/usr/local/lib"}
    links {"papi"}
    printf("%s", "Detected available PAPI counters")
  end

  configuration "ReleaseFLTO"
    toolset "flto"

  configuration "ReleaseClang"
    toolset "clang34"

  configuration "*Release*"
    linkoptions { "-O3", "-fwhole-program", "-march=native"}
    buildoptions { "-O3" }

  project "dispatch-lib"
    kind "StaticLib"
    files { "dispatch/*.cpp"}
    location "build"

  project "dispatch-test"
    kind "ConsoleApp"
    files {"dispatch_test/*.cpp"}
    links {"dispatch-lib"}
    location "build"
    targetdir "."

  project "storage-lib"
    files {"storage/*.cpp"}
    kind "StaticLib"
    location "build"

  project "access-lib"
    files {"access/*.cpp"}
    kind "StaticLib"
    location "build"

  project "storage-perf2"
    kind "ConsoleApp"
    files {"performance_test/*.cpp"}
    links {"access-lib", "storage-lib", "dispatch-lib"}
    location "build"
    targetdir "."
