-- include subprojects
includes("lib/commonlibsf")

-- set project constants
set_project("SeamlessGravJumps")
set_version("0.0.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- define targets
target("SeamlessGravJumps")
    add_rules("commonlibsf.plugin", {
        name = "SeamlessGravJumps",
        author = "bobby",
        description = "Remove grav jump loading screen!",
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
