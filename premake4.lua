solution "brogue"
    configurations { "Debug", "Release" }

    includedirs {
        "src/brogue",
        "src/platform",
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
