case "$(uname -s)" in

    Darwin)
        # Cross compiler for 391
        if [[ ! -a gcc.dmg ]]; then
            wget https://github.com/TakefiveInteractive/ece391ForMac/releases/download/0.1/gcc.dmg &> /dev/null
        fi
        if [[ ! -a /Volumes/ece391cc/cross/bin/i686-elf-gcc ]]; then
            disk=$(hdiutil attach -noverify gcc.dmg -nomount | awk '{print $1}' | head -n 1)
            mkdir /Volumes/ece391cc &> /dev/null
            diskutil mount -mountPoint /Volumes/ece391cc ${disk}s1 &> /dev/null
        fi
        echo '/Volumes/ece391cc/cross/bin/i686-elf-gcc'
        ;;

    Linux)
        which i686-elf-gcc &> /dev/null
        if [[ -z $? ]]; then
            echo 'i686-elf-gcc'
        else
            if [[ ! -a gcc-strip.tar.gz ]]; then
                wget --no-check-certificate https://github.com/TakefiveInteractive/ece391ForMac/releases/download/0.1/gcc-strip.tar.gz &> /dev/null
            fi
            if [[ ! -a cross/bin/i686-elf-gcc ]] || [[ ! -a cross/bin/i686-elf-ld ]]; then
                tar xzf gcc-strip.tar.gz
            fi
            PWD=$(pwd)
            echo "${PWD}/cross/bin/i686-elf-gcc"
        fi
        ;;

    *)
        exit 1
        ;;
esac

