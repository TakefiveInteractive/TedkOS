#!/bin/sh


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
rm -rf /tmp/mp3
rmdir /mnt/tmpmp3

