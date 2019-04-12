-- BIFROST
--
-- This file is distributed under the MIT License (MIT).
-- See LICENSE.txt for details.

local os = require "os"
local io = require "io"

-- Report an error message
function bifrost_error(msg)
  io.stderr:write("Error: " .. msg)
  os.exit(1)
end

-- Get an environment variable or fail
function bifrost_getenv(variable)
  var = os.getenv(variable)
  if var == nil or var == "" then
    bifrost_error("Environment variabe '" .. variable .. "' is not defined\n")
  end
  return var
end

-- Set the output directory
build_dir = bifrost_getenv("BIFROST_BUILD_DIR")

workspace "bifrost" 
  location(build_dir)
  language "C++"
  architecture "x86_64"
  cppdialect "C++17"
  
  flags { "MultiProcessorCompile" }

  -- Configurations
  configurations { "Debug", "Release" }
      
  filter { "configurations:Release" }
    optimize "On"
    
  -- *****
  -- *** External ***
  -- *
  
  -- *** MinHook ***
  project "external_minhook"
    minhook_dir = bifrost_getenv("BIFROST_MINHOOK_DIR")

    kind "StaticLib"
    files(minhook_dir .. "/src/**")
    includedirs(minhook_dir .. "/include")

    function bifrost_add_external_minhook()
      includedirs(minhook_dir .. "/include")
      links "external_minhook"
    end
    
  -- *** SpdLog *** 
  project "external_spdlog"
    spdlog_dir = bifrost_getenv("BIFROST_SPDLOG_DIR")

    kind "None"
    files { spdlog_dir .. "/**" }
    includedirs(spdlog_dir .. "/include")

    function bifrost_add_external_spdlog()
      includedirs(spdlog_dir .. "/include")
    end
    
  -- *** Args *** 
  project "external_args"
    args_dir = bifrost_getenv("BIFROST_ARGS_DIR")

    kind "None"
    files { args_dir .. "/**" }
    includedirs(args_dir)

    function bifrost_add_external_args()
      includedirs(args_dir)
    end
	
  -- *** GTest *** 
  project "external_gtest"
    gtest_dir = bifrost_getenv("BIFROST_GTEST_DIR")

    kind "StaticLib"
    files { gtest_dir .. "/**" }
    includedirs { gtest_dir .. "/include", gtest_dir }
    removefiles { "**gtest-all.cc", "**gtest_main.cc" }

    function bifrost_add_external_gtest()
      includedirs(gtest_dir .. "/include")
      links "external_gtest"
    end
    
  -- *****
  -- *** Bifrost ***
  -- *
    
  -- *** Core ***
  project "bifrost_core"
    kind "StaticLib"
    includedirs { "source" }
    pchheader "bifrost_core/common.h"
    pchsource "source/bifrost_core/common.cpp"
    
    files "source/bifrost_core/**"
    removefiles "**_test.cpp"
    disablewarnings { "4267" }

    function bifrost_add_bifrost_core()
      includedirs "source" 
      links "bifrost_core"
    end

  -- *** Shared ***
  project "bifrost_shared"
    kind "SharedLib"
    defines { "BIFROST_SHARED_EXPORTS" }
    includedirs { "source" }
    disablewarnings { "4251", "4146", "4244" }
    
    files "source/bifrost_shared/**" 
    removefiles "**_test.cpp"
    
    pchheader "bifrost_shared/common.h"
    pchsource "source/bifrost_shared/common.cpp"
    
    bifrost_add_bifrost_core()
    
    function bifrost_add_bifrost_shared()
      includedirs "source"
      links "bifrost_shared"
    end
    
  -- *** Loader ***
  project "bifrost_loader"
    kind "SharedLib"
    defines { "BIFROST_LOADER_EXPORTS" }
    includedirs { "source" }
    
    files "source/bifrost_loader/**" 
    removefiles "**_test.cpp"
    
    pchheader "bifrost_loader/common.h"
    pchsource "source/bifrost_loader/common.cpp"
    
    bifrost_add_bifrost_core()
    
    function bifrost_add_bifrost_loader()
      includedirs "source" 
      links "bifrost_loader"
    end
    
  -- *** Injector ***
  project "bifrost_injector"
    kind "SharedLib"
    defines { "BIFROST_INJECTOR_EXPORTS" }
    includedirs { "source" }
    
    files "source/bifrost_injector/**"
    removefiles "**_test.cpp"

    pchheader "bifrost_injector/common.h"
    pchsource "source/bifrost_injector/common.cpp"
    
    bifrost_add_bifrost_core()
    bifrost_add_external_spdlog()
    
    dependson { "bifrost_shared", "bifrost_loader" }
    
    function bifrost_add_bifrost_injector()
      includedirs "source" 
      links "bifrost_injector"
      bifrost_add_external_spdlog()
    end
    
  project "bifrost_injector_cli"
    kind "ConsoleApp"
    files "source/bifrost_injector_cli/main.cpp"
    targetname "bifrost-injector"
    
    bifrost_add_bifrost_injector()
    bifrost_add_external_args()
	
  -- *****
  -- *** Test ***
  -- *****
  
  -- *** Test Util ***
  project "bifrost_test_util"
    kind "StaticLib"
    includedirs { "source" }
    
    files "source/bifrost_test_util/**" 
    
    pchheader "bifrost_test_util/common.h"
    pchsource "source/bifrost_test_util/common.cpp"
    
    bifrost_add_bifrost_core()
    bifrost_add_external_gtest()
    
    function bifrost_add_bifrost_test_util()
      includedirs "source"
      links "bifrost_test_util"
      
      bifrost_add_bifrost_core()
      bifrost_add_external_gtest()
    end
  
  -- *** Shared Test ***
  project "bifrost_shared_test"
    kind "ConsoleApp"
    includedirs { "source" }
    targetname "test-bifrost-shared"
    disablewarnings { "4251", "4146" }
    
    files "source/bifrost_shared/*_test.cpp"
	
    bifrost_add_bifrost_test_util()
    bifrost_add_bifrost_shared()
    
  -- *** Core Test ***
  project "bifrost_core_test"
    kind "ConsoleApp"
    includedirs { "source" }
    targetname "test-bifrost-core"
    
    files "source/bifrost_core/*_test.cpp"
	
    bifrost_add_bifrost_test_util()
    bifrost_add_bifrost_core()
    
  -- *****
  -- *** Example ***
  -- *
    
  -- *** Hello World ***
  project "01-hello-world"
    kind "ConsoleApp"
    files { "example/01-hello-world/hello_world.h", "example/01-hello-world/hello_world.cpp" }
  
  --project "01-hello-world-plugin"
    --kind "SharedLib"
    --files "example/01-hello-world/hello-world-plugin.cpp" 
    
    