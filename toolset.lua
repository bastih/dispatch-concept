--
-- clang.lua
-- Clang toolset adapter for Premake
-- Copyright (c) 2013 Jason Perkins and the Premake project
--

premake.tools.flto = {}
table.insert(premake.fields.toolset.allowed, "flto")
local flto = premake.tools.flto
local gcc = premake.tools.gcc

function flto.getcppflags(cfg)
   -- Just pass through to GCC for now
   local flags = gcc.getcppflags(cfg)
   return flags
end

function flto.getcflags(cfg)
   local flags = gcc.getcflags(cfg)
   table.insert(flags, "-flto")
   table.insert(flags, "-fno-fat-lto-objects")
   return flags

end

function flto.getcxxflags(cfg)
   local flags = gcc.getcxxflags(cfg)
   return flags
end

function flto.getdefines(defines)
   local flags = gcc.getdefines(defines)
   return flags

end

function flto.getforceincludes(cfg)
   local flags = gcc.getforceincludes(cfg)
   return flags

end

function flto.getincludedirs(cfg, dirs)
   local flags = gcc.getincludedirs(cfg, dirs)
   return flags

end

function flto.getldflags(cfg)
   local flags = gcc.getldflags(cfg)
   table.insert(flags, "-flto=2")
   return flags

end

function flto.getlinks(cfg, systemOnly)
   local flags = gcc.getlinks(cfg, systemOnly)
   return flags

end


function flto.getmakesettings(cfg)
   local flags = gcc.getmakesettings(cfg)
   return flags

end

function flto.gettoolname(cfg, tool)
   if tool == "ar" then
      return "ar --plugin=/usr/lib/gcc/x86_64-linux-gnu/4.8/liblto_plugin.so"
   elseif tool == "cc" then
      return "gcc-4.8"
   else
      return "g++-4.8 -time"
   end
end
