solution "brogue"
    configurations { "Debug", "Release" }
    includedirs {
        "src/brogue",
        "src/platform",
        "src/sodna-0.2.0/include",
        "/usr/include/SDL2",
    }

    project "brogue"
        kind "WindowedApp"
        language "C"
        files {
            "src/brogue/**.c",
            "src/brogue/**.h",
            "src/platform/**.h",
            "src/platform/**.c",
            "src/sodna-0.2.0/src_sdl2/**.c",
        }

        defines {
            "BROGUE_CURSES",
            "BROGUE_SODNA",
        }

        links {
            "m",
            "SDL2",
            "ncurses",
        }
