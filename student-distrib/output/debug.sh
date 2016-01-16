#!/bin/bash

case "$(uname -s)" in

    Darwin)
		set confirm = "n"
		if [ "$(id -u)" == "0" ]; then
			echo "OS X safe build cannot be used when you are root."
		else
			echo
			echo "Now we can build system withOUT third party tools"
			echo
			echo "** you may need to modify qemu settings **"
			echo
			echo -ne "Use OS X safe build? (y/*) __\b\b"
			read -n 1 confirm
			echo
		fi
        if [ "$confirm" == "y" ];
        then
            cp ./osx/orig_fat.img ./osx/tedkos.img
            if [ -d ~/mnt/tedkos ]; then
                rm -rf ~/mnt/tedkos
            fi
            mkdir -p ~/mnt/tedkos
            disk=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount ./osx/tedkos.img)
            disk=$(echo $disk | awk '{print $1}')
            mount -w -t msdos ${disk}s1 ~/mnt/tedkos
            cp ./bootimg ~/mnt/tedkos/
            cp ./filesys_img ~/mnt/tedkos/
            sync
            umount ~/mnt/tedkos
            hdiutil detach $disk

            echo "System image successfully built."
            echo "If you choose to start qemu here:"
            echo ""
            echo "    ** CHOOSE 'hdb' in boot menu! **"
            echo ""
            echo -ne "Start qemu? (y/*) __\b\b"
            read -n 1 confirm
            echo
            if [ "$confirm" == "y" ];
            then
                (qemu-system-i386 -hda ./osx/grub.img -hdb ./osx/tedkos.img  -hdc ./osx/logfs.img -soundhw all -m 256 -gdb tcp:127.0.0.1:1234 -name "TedkOS (hdb)")&
                echo "PID of qemu = $!"
            fi
            exit 0
        fi
        ;;
    *)
        ;;
esac


if [ "$(id -u)" != "0" ]; then
	sudo $0
	exit 0
fi

if [ -d /mnt/tmpmp3 ]; then
rm -rf /mnt/tmpmp3
fi

if [ -d /tmp/mp3 ]; then
rm -rf /tmp/mp3
fi

mkdir -p /mnt/tmpmp3
mkdir -p /tmp/mp3
cp ./bootimg /tmp/mp3/
cp ./filesys_img /tmp/mp3/
cp ./orig.img /tmp/mp3/mp3.img

case "$(uname -s)" in

   Darwin)
     disk=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount -section 63 /tmp/mp3/mp3.img)
     mount_cmd="mount -w -t fuse-ext2 $disk /mnt/tmpmp3"
     unmount_cmd="umount /mnt/tmpmp3"
     unload_cmd="hdiutil detach $disk"
     ;;

   Linux)
     mount_cmd="mount -o loop,offset=32256 /tmp/mp3/mp3.img /mnt/tmpmp3"
     unmount_cmd=""
     unload_cmd="umount /mnt/tmpmp3"
     ;;

   *)
     echo 'other OS'
     exit 1
     ;;
esac

$mount_cmd
cp -f /tmp/mp3/bootimg /mnt/tmpmp3/
cp -f /tmp/mp3/filesys_img /mnt/tmpmp3/
$unmount_cmd
$unload_cmd
cp -f /tmp/mp3/mp3.img ./
chmod 666 ./mp3.img
rm -rf /tmp/mp3
rmdir /mnt/tmpmp3
