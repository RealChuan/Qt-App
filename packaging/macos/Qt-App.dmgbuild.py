# Path: packaging/macos/Qt-App.dmgbuild.py
# Use: dmgbuild -s ./packaging/macos/Qt-App.dmgbuild.py "Qt-App" bin-64/RelWithDebInfo/Qt-App.dmg

import os.path
import plistlib


def icon_from_app(app_path):
    plist_path = os.path.join(app_path, "Contents", "Info.plist")
    with open(plist_path, "rb") as f:
        plist = plistlib.load(f)
    icon_name = plist["CFBundleIconFile"]
    icon_root, icon_ext = os.path.splitext(icon_name)
    if not icon_ext:
        icon_ext = ".icns"
    icon_name = icon_root + icon_ext
    return os.path.join(app_path, "Contents", "Resources", icon_name)


application = defines.get("app", "Qt-App.app")
icon = icon_from_app(application)

# Volume format (see hdiutil create -help)
format = defines.get("format", "UDZO")

# Volume size (must be large enough for your files)
size = defines.get("size", None)

# Files to include
files = [application]

# Symlinks to create
symlinks = {"Applications": "/Applications"}

# Volume icon
badge_icon = icon_from_app(application)

# Where to put the icons
icon_locations = {
    os.path.basename(application): (140, 120),
    "Applications": (500, 120),
}

# Where to put the files
icon_positions = {
    os.path.basename(application): (140, 240),
}

# Background
background = "builtin-arrow"

# Volume label (not displayed by default)
volume_label = "Qt-App"

# Applications link (make a link to /Applications, optional, default is True)
applications_link = True

# DMG format version (internet-enable the dmg, optional, default is 2)
# (set to False to disable internet-enable)
internet_enable = True

# Compression level (optional, default is 9)
compression_level = 9

# Additional dmg window options (background color, position and size)
window_rect = ((100, 100), (640, 360))

# Additional dmg window view options (icon positions, background picture)
# (icon positions are relative to the background picture)
window_pos = {
    "background.png": (0, 0),
    os.path.basename(application): (140, 240),
    "Applications": (500, 120),
}

# Additional dmg window position options (window position, width and height)
window_options = {
    "background.png": (0, 0, 640, 360),
    os.path.basename(application): (140, 240, 320, 240),
    "Applications": (500, 120, 140, 240),
}

# Select the default view; must be one of
#
#    'icon-view'
#    'list-view'
#    'column-view'
#    'coverflow'
#
default_view = "icon-view"

# General view configuration
show_icon_preview = False

# Set these to True to force inclusion of icon/list view settings (otherwise
# we only include settings for the default view)
include_icon_view_settings = "auto"
include_list_view_settings = "auto"

# .. Icon view configuration ...................................................
arrange_by = None
grid_offset = (0, 0)
grid_spacing = 100
scroll_position = (0, 0)
label_pos = "bottom"  # or 'right'
text_size = 16
icon_size = 128

# Code signing identity (optional)
# Developer ID Application: Your Name (XXXXXXXXXX)
# identity = "Developer ID Application: Your Name (XXXXXXXXXX)"

# Code signing entitlements (optional)
# entitlements = "entitlements.plist"

# Code signing requirements (optional)
# requirements = "requirements.plist"

# Code signing resource rules (optional)
# resource_rules = "resource-rules.plist"

# Code signing options (optional)
# code_sign = True

# Code signing flags (optional)
# code_sign_flags = "--deep"

# Code signing timestamp (optional)
# code_sign_timestamp = True

# Code signing identifier (optional)
# code_sign_identifier = "com.example.pkg"

# Code signing entitlements (optional)
# code_sign_entitlements = "entitlements.plist"

# Code signing requirements (optional)
# code_sign_requirements = "requirements.plist"

# Code signing resource rules (optional)
# code_sign_resource_rules = "resource-rules.plist"

# Code signing options (optional)
# code_sign_options = "--timestamp --options=runtime"
