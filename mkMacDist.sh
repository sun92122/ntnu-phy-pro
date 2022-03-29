#!/bin/bash
# Create a distributable installable package

MAC_DIST_DIR=scidavis/scidavis.app/Contents/MacOS
RES_DIR=scidavis/scidavis.app/Contents/Resources
version=`grep scidavis_version libscidavis/src/version.cpp|tail -1|cut -f5 -d' '|tr -d '";'`
if [ $version = '"unknown"' ]; then
    version=0.0.0.0
fi

rewrite_dylibs()
{
    local target=$1
    echo "rewrite_dylibs $target"
    otool -L $target|grep opt/local|cut -f1 -d' '|while read dylib; do
        # avoid infinite loops
        if [ "${dylib##*/}" != "${target##*/}" ]; then 
            cp -f $dylib $MAC_DIST_DIR
            chmod u+rw $MAC_DIST_DIR/${dylib##*/}
            rewrite_dylibs $MAC_DIST_DIR/${dylib##*/}
            echo "install_name_tool -change $dylib @executable_path/${dylib##*/} $target"
        fi
        install_name_tool -change $dylib @executable_path/${dylib##*/} $target
    done
    otool -L $target|grep usr/local|cut -f1 -d' '|while read dylib; do
        # avoid infinite loops
        if [ "${dylib##*/}" != "${target##*/}" ]; then 
            cp -f $dylib $MAC_DIST_DIR
            chmod u+rw $MAC_DIST_DIR/${dylib##*/}
            rewrite_dylibs $MAC_DIST_DIR/${dylib##*/}
        fi
        install_name_tool -change $dylib @executable_path/${dylib##*/} $target
    done
}

cp scidavis/scidavis $MAC_DIST_DIR
rewrite_dylibs $MAC_DIST_DIR/scidavis

chmod u+w $MAC_DIST_DIR/*

# Generic resources required for Qt
cp -rf /opt/local/libexec/qt5/Library/Frameworks/QtGui.framework/Resources/qt_menu.nib $RES_DIR

# python resources
mkdir -p $RES_DIR/lib
cp -rf /opt/local/Library/Frameworks/Python.framework/Versions/3.9/lib/python3.9 $RES_DIR/lib

# python resources contain some dynamic libraries that need rewriting
find $RES_DIR/lib -name "*.so" -print | while read soname; do
    otool -L $soname|grep /opt/local|cut -f1 -d' '|while read oldName; do
        install_name_tool -change $oldName @executable_path/${oldName##*/} $soname
        done
done

# copy translation files
cp -rf scidavis/translations $MAC_DIST_DIR

# copy icon, and create mainfest
cp -f scidavis/icons/scidavis.icns $RES_DIR
cat >scidavis/scidavis.app/Contents/Info.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>NSPrincipalClass</key>
	<string>NSApplication</string>
	<key>CFBundleIconFile</key>
	<string>scidavis.icns</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleGetInfoString</key>
	<string>Created by Qt/QMake</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleExecutable</key>
	<string>scidavis</string>
	<key>CFBundleIdentifier</key>
	<string>net.sourceforge.scidavis</string>
        <key>LSEnvironment</key>
        <dict>
           <key>PYTHONHOME</key>
           <string>/Applications/scidavis.app/Contents/Resources</string>
        </dict>
</dict>
</plist>
EOF

codesign -s "Developer ID Application" --deep --force scidavis/scidavis.app
if [ $? -ne 0 ]; then
    echo "try running this script on the Mac GUI desktop, not ssh shell"
    exit
fi

rm -f scidavis-$version-mac-dist.dmg
hdiutil create -srcfolder scidavis/scidavis.app -volname SciDAVis -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 200M temp.dmg
hdiutil convert -format UDZO -imagekey zlib-level=9 -o scidavis-$version-mac-dist.dmg temp.dmg 
rm -f temp.dmg
codesign -s "Developer ID Application" scidavis-$version-mac-dist.dmg
xcrun altool --notarize-app --primary-bundle-id SciDAVis --username apple@hpcoders.com.au --password "@keychain:Minsky" --file scidavis-$version-mac-dist.dmg
# check using xcrun altool --notarization-history 0 -u apple@hpcoders.com.au -p "@keychain:Minsky"
xcrun stapler staple scidavis-$version-mac-dist.dmg
if [ $? -ne 0 ]; then
    echo "Manually staple with:"
    echo "xcrun stapler staple scidavis-$version-mac-dist.dmg"
fi
