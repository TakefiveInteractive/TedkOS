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
            if [[ ! -a gcc.tar.gz ]]; then
                wget https://github.com/TakefiveInteractive/ece391ForMac/releases/download/0.1/gcc.tar.gz &> /dev/null
            fi
            if [[ ! -a gcc/cross/bin/i686-elf-gcc ]]; then
                tar xzf gcc.tar.gz
            fi
            echo 'gcc/cross/bin/i686-elf-gcc'
        fi
        ;;

    *)
        exit 1
        ;;
esac

