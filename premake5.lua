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
    
  -- *** JSON *** 
  project "external_json"
    json_dir = bifrost_getenv("BIFROST_JSON_DIR")

    kind "None"
    files { json_dir .. "/**" }
    includedirs(json_dir)

    function bifrost_add_external_json()
      includedirs(json_dir)
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
    
    pchheader "bifrost/core/common.h"
    pchsource "source/bifrost/core/common.cpp"
    
    files { "source/bifrost/core/*.cpp", "source/bifrost/core/*.h" }
    disablewarnings { "4267", "4146" }
    
    bifrost_add_external_json()

    function bifrost_add_bifrost_core()
      includedirs "source" 
      links "bifrost_core"
      bifrost_add_external_json()
    end

--  -- *** Shared ***
--  project "bifrost_shared"
--    kind "SharedLib"
--    defines { "BIFROST_SHARED_EXPORTS" }
--    includedirs { "source" }
--    disablewarnings { "4251", "4146", "4244" }
    
--    files "source/bifrost/shared/**" 
--    removefiles { "**_test.cpp", "**_test.h" }
    
--    pchheader "bifrost/shared/common.h"
--    pchsource "source/bifrost/shared/common.cpp"
    
--    bifrost_add_bifrost_core()
    
--    function bifrost_add_bifrost_shared()
--      includedirs "source"
--      links "bifrost_shared"
--    end
    
--  -- *** Loader ***
--  project "bifrost_loader"
--    kind "SharedLib"
--    defines { "BIFROST_LOADER_EXPORTS" }
--    includedirs { "source" }
    
--    files "source/bifrost/loader/**" 
--    removefiles { "**_test.cpp", "**_test.h" }
    
--    pchheader "bifrost/loader/common.h"
--    pchsource "source/bifrost/loader/common.cpp"
    
--    bifrost_add_bifrost_core()
    
--    function bifrost_add_bifrost_loader()
--      includedirs "source" 
--      links "bifrost_loader"
--    end
    
--  -- *** Injector ***
--  project "bifrost_injector"
--    kind "SharedLib"
--    defines { "BIFROST_INJECTOR_EXPORTS" }
--    includedirs { "source" }
    
--    files "source/bifrost/injector/**"
--    removefiles { "**_test.cpp", "**_test.h", "*/cli/*" }

--    pchheader "bifrost/injector/common.h"
--    pchsource "source/bifrost/injector/common.cpp"
    
--    bifrost_add_bifrost_core()
--    bifrost_add_external_spdlog()
    
--    dependson { "bifrost_shared", "bifrost_loader" }
    
--    function bifrost_add_bifrost_injector()
--      includedirs "source" 
--      links "bifrost_injector"
--      bifrost_add_external_spdlog()
--    end
    
--  project "bifrost_injector_cli"
--    kind "ConsoleApp"
--    files "source/bifrost/injector/cli/main.cpp"
--    targetname "bifrost-injector"
    
--    bifrost_add_bifrost_injector()
--    bifrost_add_external_args()
	
  -- *****
  -- *** Test ***
  -- *****
  
  -- *** Test Util ***
--  project "bifrost_test_util"
--    kind "StaticLib"
--    includedirs { "source" }
    
--    pchheader "bifrost/test_util/common.h"
--    pchsource "source/bifrost/test_util/common.cpp"
    
--    files "source/bifrost/test_util/**" 
    
--    bifrost_add_bifrost_core()
--    bifrost_add_external_gtest()
    
--    function bifrost_add_bifrost_test_util()
--      includedirs "source"
--      links "bifrost_test_util"
      
--      bifrost_add_bifrost_core()
--      bifrost_add_external_gtest()
--    end
  
  -- *** Shared Test ***
--  project "bifrost_shared_test"
--    kind "ConsoleApp"
--    includedirs { "source" }
--    targetname "test-bifrost-shared"
--    disablewarnings { "4251", "4146" }
    
--    files "source/bifrost/shared/*_test.*"
	
--    bifrost_add_bifrost_test_util()
--    bifrost_add_bifrost_shared()
    
  -- *** Core Test (mock executable) ***
  project "bifrost_core_test_mock_executable"
    kind "ConsoleApp"
    targetname "test-bifrost-core-mock-executable"
    files { "source/bifrost/core/test/data/mock_executable.cpp" }


  -- *** Core Test ***
  project "bifrost_core_test"
    kind "ConsoleApp"
    includedirs { "source" }
    targetname "test-bifrost-core"
    
    pchheader "bifrost/core/test/test.h"
    pchsource "source/bifrost/core/test/test.cpp"
    
    files { "source/bifrost/core/test/*" }
	
    bifrost_add_external_gtest()
    bifrost_add_bifrost_core()
    dependson { "bifrost_core_test_mock_executable" } 
    
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
    
    