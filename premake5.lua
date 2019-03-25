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
    bifrost_error("environment variabe '" .. variable .. "' is not defined\n")
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
    
  -- *** External *** 
  project "external_minhook"
    minhook_dir = bifrost_getenv("BIFROST_MINHOOK_DIR")

    kind "StaticLib"
    files(minhook_dir .. "/src/**")
    includedirs(minhook_dir .. "/include")

    function bifrost_add_external_minhook()
      includedirs(minhook_dir .. "/include")
      links "external_minhook"
    end
    
  project "external_spdlog"
    spdlog_dir = bifrost_getenv("BIFROST_SPDLOG_DIR")

    kind "None"
    files { spdlog_dir .. "/**" }
    includedirs(spdlog_dir .. "/include")

    function bifrost_add_external_spdlog()
      includedirs(spdlog_dir .. "/include")
    end
    
  project "external_args"
    args_dir = bifrost_getenv("BIFROST_ARGS_DIR")

    kind "None"
    files { args_dir .. "/**" }
    includedirs(args_dir)

    function bifrost_add_external_args()
      includedirs(args_dir)
    end
	
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
    
  -- *** Bifrost ***
  project "bifrost_core"
    kind "StaticLib"
    includedirs { "source" }
    pchheader "bifrost_core/common.h"
    pchsource "source/bifrost_core/common.cpp"
    
    files "source/bifrost_core/**"
    
    bifrost_add_external_minhook()

    function bifrost_add_bifrost_core()
      includedirs "source" 
      links "bifrost_core"
    end

  project "bifrost"
    kind "SharedLib"
    defines { "BIFROST_EXPORTS" }
    includedirs { "source" }
    
    files "source/bifrost/**" 
    
    bifrost_add_bifrost_core()
    
    function bifrost_add_bifrost()
      includedirs "source" 
      links "bifrost"
    end

  project "bifrost_shared"
    kind "SharedLib"
    defines { "BIFROST_SHARED_EXPORTS" }
    includedirs { "source" }
    
    files "source/bifrost_shared/**" 
    removefiles "**_test.cpp"
    
    bifrost_add_bifrost_core()
    
    function bifrost_add_bifrost_shared()
      includedirs "source"
      links "bifrost_shared"
    end
	
  project "bifrost_shared_test"
    kind "ConsoleApp"
    includedirs { "source" }
    targetname "test-bifrost-shared"
    
    files "source/bifrost_shared/*_test.cpp"
	
	bifrost_add_external_gtest()
	bifrost_add_bifrost_shared()
	
  -- *** Bifrost Injector ***
  project "bifrost_injector"
    kind "ConsoleApp"
    files "source/bifrost_injector/**"
    targetname "bifrost-injector"
	
    pchheader "bifrost_injector/common.h"
    pchsource "source/bifrost_injector/common.cpp"
    
    bifrost_add_bifrost()
    bifrost_add_bifrost_core()
    bifrost_add_external_args()
    bifrost_add_external_spdlog()
    
  project "bifrost_injector_hook_example"
    kind "ConsoleApp"
    files "source/bifrost_injector_hook_example/**" 
    