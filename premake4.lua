solution "brogue"
    configurations { "Debug", "Release" }

    newoption {
        trigger = "sodna",
        description = "Enable Sodna graphical terminal as backend",
    }

    newoption {
        trigger = "curses",
        description = "Enable Curses terminal backend (not available on Windows)",
    }

    project "brogue"
        kind "WindowedApp"
        language "C"
        files {
            "src/brogue/**.c",
            "src/brogue/**.h",
            "src/platform/platform.h",
            "src/platform/platform.h",
            "src/platform/main.c",
            "src/platform/PlatformDefines.h",
            "src/platform/platformdependent.c",
            "src/platform/platform.h",
        }

        includedirs {
            "src/brogue",
            "src/platform",
        }

        links {
            "m",
        }

        -- Default options
        if os.get() == "windows" then
            -- Default to building only Sodna backend on Windows.
            if not _OPTIONS.sodna then _OPTIONS.sodna = "" end
        else
            -- On platforms with Curses, default to building
            -- both Sodna and Curses backends.
            if not _OPTIONS.sodna and not _OPTIONS.curses then
                _OPTIONS.sodna = ""
                _OPTIONS.curses = ""
            end
        end

        configuration "sodna"
            defines {
                "BROGUE_SODNA",
            }

            links {
                "SDL2",
            }

            files {
                "src/platform/sodna_default_font.inc",
                "src/platform/sodna.h",
                "src/platform/sodna-platform.c",
                "src/platform/sodna_sdl2.c",
                "src/platform/sodna_util.c",
                "src/platform/sodna_util.h",
                "src/platform/stb_image.h",
                "src/platform/stb_image_write.h",
            }

        configuration "curses"
            defines {
                "BROGUE_CURSES",
            }

            files {
                "src/platform/curses-platform.c",
                "src/platform/term.c",
                "src/platform/term.h",
            }

            links {
                "ncurses",
            }

        configuration { "sodna", "windows" }
            includedirs {
                "win32/SDL2-2.0.3/include"
            }

            libdirs {
                "win32/SDL2-2.0.3/i686-w64-mingw32/lib/"
            }

        configuration { "sodna", "linux" }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }
