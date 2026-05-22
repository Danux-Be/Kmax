install(FILES org.kde.kmax.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

install(FILES org.kde.kmax.appdata.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install(FILES kmaxui.rc
    DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kmax
)
