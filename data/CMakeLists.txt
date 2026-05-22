install(FILES org.kde.kmax.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

install(FILES org.kde.kmax.appdata.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install(FILES kmaxui.rc
    DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/kmax
)

ecm_install_icons(ICONS
    icons/16-apps-kmax.png
    icons/22-apps-kmax.png
    icons/32-apps-kmax.png
    icons/48-apps-kmax.png
    icons/64-apps-kmax.png
    icons/128-apps-kmax.png
    icons/256-apps-kmax.png
    icons/sc-apps-kmax.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}
)
