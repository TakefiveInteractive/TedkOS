case "$(uname -s)" in

   Darwin)
     echo 'i686-elf-gcc'
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

