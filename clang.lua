--
-- clang.lua
-- Clang toolset adapter for Premake
-- Copyright (c) 2013 Jason Perkins and the Premake project
--

premake.tools.clang34 = {}
table.insert(premake.fields.toolset.allowed, "clang34")
local clang = premake.tools.clang34
local gcc = premake.tools.gcc

function clang.getcppflags(cfg)
   -- Just pass through to GCC for now
   local flags = gcc.getcppflags(cfg)
   return flags
end

function clang.getcflags(cfg)
   local flags = gcc.getcflags(cfg)
   table.insert(flags, "-flto")
--   table.insert(flags, "-fno-fat-lto-objects")
   return flags

end

function clang.getcxxflags(cfg)
   local flags = gcc.getcxxflags(cfg)
   return flags
end

function clang.getdefines(defines)
   local flags = gcc.getdefines(defines)
   return flags

end

function clang.getforceincludes(cfg)
   local flags = gcc.getforceincludes(cfg)
   return flags

end

function clang.getincludedirs(cfg, dirs)
   local flags = gcc.getincludedirs(cfg, dirs)
   return flags

end

function clang.getldflags(cfg)
   local flags = gcc.getldflags(cfg)
   table.insert(flags, "-flto=2")
   return flags

end

function clang.getlinks(cfg, systemOnly)
   local flags = gcc.getlinks(cfg, systemOnly)
   return flags

end


function clang.getmakesettings(cfg)
   local flags = gcc.getmakesettings(cfg)
   return flags

end

function clang.gettoolname(cfg, tool)
   local prefix = "~/polly/llvm_build/bin/"
   if tool == "ar" then
      return prefix .. "llvm-ar"
   elseif tool == "cc" then
      return prefix .. "clang"
   else
      return prefix .. "clang++"
   end
end
