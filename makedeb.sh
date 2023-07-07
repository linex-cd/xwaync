#!/bin/bash

BUILDDIR=./build
INSDIR=/opt/xwaync

mkdir -p $BUILDDIR/DEBIAN
mkdir -p $BUILDDIR$INSDIR

#编译程序
gcc src/xwaync.c -o $BUILDDIR$INSDIR/xwaync -ldrm -I/usr/include/drm/

cat service/xwayncd.service > $BUILDDIR$INSDIR/xwayncd.service 
cat service/start_xwayncd.sh > $BUILDDIR$INSDIR/start_xwayncd.sh

chmod +x $BUILDDIR$INSDIR/start_xwayncd.sh

#配置安装包

cat package/info.txt > $BUILDDIR/DEBIAN/control

cat service/install.sh > $BUILDDIR/DEBIAN/postinst
cat service/remove.sh > $BUILDDIR/DEBIAN/prerm

echo "#!/bin/bash" > $BUILDDIR/DEBIAN/preinst
echo "#!/bin/bash" > $BUILDDIR/DEBIAN/postrm


chmod +x $BUILDDIR/DEBIAN/postinst
chmod +x $BUILDDIR/DEBIAN/prerm

chmod +x $BUILDDIR/DEBIAN/preinst
chmod +x $BUILDDIR/DEBIAN/postrm

#打包

dpkg-deb --build $BUILDDIR
