case "$(uname -s)" in

   Darwin)
     # Cross compiler for 391
     echo '/Volumes/ece391cc/cross/bin/i686-elf-gcc'
     ;;

   Linux)
     which i686-elf-gcc &> /dev/null
     if [[ -z $? ]]; then
         echo 'i686-elf-gcc'
     else
         echo 'gcc'
     fi
     ;;

   *)
     exit 1
     ;;
esac

