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
    
  -- *** Bifrost Core ***
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
    
  -- *** Bifrost Debugger ***
  project "bifrost_debugger"
    kind "StaticLib"
    includedirs { "source" }
    
    files { "source/bifrost/debugger/*.cpp", "source/bifrost/debugger/*.h" }
    disablewarnings { "4267", "4146" }
    
    pchheader "bifrost/debugger/common.h"
    pchsource "source/bifrost/debugger/common.cpp"
    
    function bifrost_add_bifrost_debugger()
      includedirs "source" 
      links "bifrost_debugger"
    end
    
  -- *** Bifrost Template ***
  project "bifrost_template"
    kind "None"
    includedirs { "source" }
    files { "source/bifrost/template/**" }
    
  -- *** Bifrost Loader ***
  project "bifrost_loader"
    kind "SharedLib"
    includedirs { "source" }
    
    files { "source/bifrost/api/loader.cpp" }
    disablewarnings { "4267", "4146" }
    
    bifrost_add_bifrost_core()
    
    function bifrost_add_bifrost_loader()
      includedirs "source" 
      links "bifrost_loader"
    end
    
    
  -- *** Bifrost Injector ***
  project "bifrost_injector"
    kind "SharedLib"
    includedirs { "source" }
    
    files { "source/bifrost/api/injector.cpp", "source/bifrost/api/injector.h" }
    disablewarnings { "4267", "4146" }
    defines { "BIFROST_INJECTOR_EXPORTS" }
    
    dependson { "bifrost_loader" }
    bifrost_add_bifrost_core()
    bifrost_add_bifrost_debugger()
    bifrost_add_bifrost_loader()

    function bifrost_add_bifrost_injector()
      includedirs "source" 
      links "bifrost_injector"
    end
    
  -- *** Plugin ***
  project "bifrost_plugin"
    kind "SharedLib"
    includedirs { "source" }
    
    files { "source/bifrost/api/plugin*" }
    disablewarnings { "4267", "4146" }
    defines { "BIFROST_PLUGIN_EXPORTS" }
    
    bifrost_add_bifrost_core()

    function bifrost_add_bifrost_plugin()
      includedirs "source" 
      links "bifrost_plugin"
    end
    
  -- *** Bifrost Core Test (mock executable) ***
  project "bifrost_core_test_mock_executable"
    kind "ConsoleApp"
    targetname "test-bifrost-core-mock-executable"
    files { "source/bifrost/core/test/data/mock_executable.cpp" }
    
  -- *** Bifrost Core Test (mock dll) ***
  project "bifrost_core_test_mock_dll"
    kind "SharedLib"
    targetname "test-bifrost-core-mock-dll"
    files { "source/bifrost/core/test/data/mock_dll.cpp" }

  -- *** Bifrost Core Test ***
  project "bifrost_core_test"
    kind "ConsoleApp"
    includedirs { "source" }
    targetname "test-bifrost-core"
    
    pchheader "bifrost/core/test/test.h"
    pchsource "source/bifrost/core/test/test.cpp"
    
    files { "source/bifrost/core/test/*" }
	
    bifrost_add_external_gtest()
    bifrost_add_bifrost_core()
    dependson { "bifrost_core_test_mock_executable", "bifrost_core_test_mock_dll" } 
    
  -- *****
  -- *** Injector ***
  -- *
    
  project "injector"
    kind "ConsoleApp"
    includedirs { "source" }
    
    pchheader "injector/common.h"
    pchsource "source/injector/common.cpp"
    
    files { "source/injector/*.cpp", "source/injector/*.h" }
    
    bifrost_add_bifrost_core()
    bifrost_add_bifrost_injector()
    bifrost_add_external_args()
    bifrost_add_external_spdlog()
    
  -- *****
  -- *** Example ***
  -- *

  -- *** Hello World ***
  project "01_hello_world_dll"
    kind "SharedLib"
    targetname "01-hello-world"
    files { 
      "example/01-hello-world/hello_world.h", 
      "example/01-hello-world/hello_world.cpp",
    }
    
  project "01_hello_world_executable"
    kind "ConsoleApp"
    targetname "01-hello-world"
    files { 
      "example/01-hello-world/hello_world.h", 
      "example/01-hello-world/hello_world_main.cpp",
    }
    links "01_hello_world_dll"
    dependson "01_hello_world_dll"
  
  project "01_hello_world_plugin"
    kind "SharedLib"
    includedirs { "source" }
    files { 
      "example/01-hello-world/hello_world_plugin.cpp", 

      "source/bifrost/template/plugin_main.h",
      "source/bifrost/template/plugin_decl.h"
    }
    