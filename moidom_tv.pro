DEFINES += _TASTE_MOIDOM_TV \
           _HAS_VOD_MANAGER \
           _IS_OEM \
           __VLC_FOR_HLS
RESOURCES += moidom_tv.qrc
HEADERS += tastes/defines_moidom_tv.h \
           cmoidomxmlparser.h
SOURCES += cmoidomxmlparser.cpp

WINICO = moidom_tv.ico
TRANSLATIONS = lang_ru.ts \
               lang_de.ts
include (common.pri)

