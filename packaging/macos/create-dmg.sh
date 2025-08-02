#!/bin/bash
# Create macOS DMG installer for WaoN

set -e

VERSION="${1:-0.11}"
APP_NAME="WaoN"
DMG_NAME="${APP_NAME}-${VERSION}.dmg"
VOLUME_NAME="${APP_NAME} ${VERSION}"
BUILD_DIR="build"
APP_BUNDLE="${APP_NAME}.app"

echo "Creating macOS DMG for ${APP_NAME} version ${VERSION}..."

# Build the project if not already built
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Building project..."
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_WAON=ON -DBUILD_PV=ON -DBUILD_GWAON=ON \
        -DBUILD_SHARED_LIB=ON
    cmake --build "${BUILD_DIR}"
fi

# Create app bundle structure
echo "Creating app bundle..."
rm -rf "${APP_BUNDLE}"
mkdir -p "${APP_BUNDLE}/Contents/"{MacOS,Resources,Frameworks}

# Copy binaries
cp "${BUILD_DIR}/waon" "${APP_BUNDLE}/Contents/MacOS/"
cp "${BUILD_DIR}/pv" "${APP_BUNDLE}/Contents/MacOS/"
[ -f "${BUILD_DIR}/gwaon" ] && cp "${BUILD_DIR}/gwaon" "${APP_BUNDLE}/Contents/MacOS/"

# Copy libraries
cp "${BUILD_DIR}/libwaon.dylib" "${APP_BUNDLE}/Contents/Frameworks/" || true

# Copy Info.plist
cp "packaging/macos/Info.plist" "${APP_BUNDLE}/Contents/"

# Update version in Info.plist
sed -i '' "s/<string>0.11<\/string>/<string>${VERSION}<\/string>/g" "${APP_BUNDLE}/Contents/Info.plist"

# Copy resources
cp README.md "${APP_BUNDLE}/Contents/Resources/"
cp LICENSE "${APP_BUNDLE}/Contents/Resources/"
mkdir -p "${APP_BUNDLE}/Contents/Resources/docs"
cp -r docs/* "${APP_BUNDLE}/Contents/Resources/docs/" || true

# Create a simple launcher script
cat > "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}" << EOF
#!/bin/bash
DIR="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
export DYLD_LIBRARY_PATH="\${DIR}/../Frameworks:\${DYLD_LIBRARY_PATH}"

# If called with arguments, run waon with those arguments
if [ \$# -gt 0 ]; then
    exec "\${DIR}/waon" "\$@"
else
    # Otherwise, show help in Terminal
    osascript -e 'tell app "Terminal" to do script "'\${DIR}/waon' --help; echo; echo Press any key to continue...; read -n 1"'
fi
EOF

chmod +x "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}"

# Sign the app bundle if possible (requires Developer ID)
if command -v codesign &> /dev/null && [ -n "${CODESIGN_IDENTITY}" ]; then
    echo "Signing app bundle..."
    codesign --force --deep --sign "${CODESIGN_IDENTITY}" "${APP_BUNDLE}"
fi

# Create DMG
echo "Creating DMG..."
rm -f "${DMG_NAME}"

# Use create-dmg if available, otherwise use hdiutil
if command -v create-dmg &> /dev/null; then
    create-dmg \
        --volname "${VOLUME_NAME}" \
        --window-pos 200 120 \
        --window-size 600 400 \
        --icon-size 100 \
        --icon "${APP_BUNDLE}" 150 185 \
        --hide-extension "${APP_BUNDLE}" \
        --app-drop-link 450 185 \
        "${DMG_NAME}" \
        "${APP_BUNDLE}"
else
    # Fallback to hdiutil
    echo "create-dmg not found, using hdiutil..."
    
    # Create temporary directory
    TEMP_DIR=$(mktemp -d)
    cp -r "${APP_BUNDLE}" "${TEMP_DIR}/"
    
    # Create symbolic link to Applications
    ln -s /Applications "${TEMP_DIR}/Applications"
    
    # Create DMG
    hdiutil create -volname "${VOLUME_NAME}" \
        -srcfolder "${TEMP_DIR}" \
        -ov -format UDZO \
        "${DMG_NAME}"
    
    # Clean up
    rm -rf "${TEMP_DIR}"
fi

# Sign DMG if possible
if command -v codesign &> /dev/null && [ -n "${CODESIGN_IDENTITY}" ]; then
    echo "Signing DMG..."
    codesign --force --sign "${CODESIGN_IDENTITY}" "${DMG_NAME}"
fi

# Notarize if possible (requires notarytool and Apple Developer account)
if command -v xcrun &> /dev/null && [ -n "${NOTARIZE_APPLE_ID}" ]; then
    echo "Notarizing DMG..."
    xcrun notarytool submit "${DMG_NAME}" \
        --apple-id "${NOTARIZE_APPLE_ID}" \
        --password "${NOTARIZE_PASSWORD}" \
        --team-id "${NOTARIZE_TEAM_ID}" \
        --wait || echo "Notarization failed, continuing..."
fi

echo "Successfully created ${DMG_NAME}"