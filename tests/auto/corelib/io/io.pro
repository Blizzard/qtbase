TEMPLATE=subdirs
SUBDIRS=\
    qabstractfileengine \
    qbuffer \
    qdatastream \
    qdataurl \
    qdebug \
    qdir \
    qdiriterator \
    qfile \
    largefile \
    qfileinfo \
    qfileselector \
    qfilesystemmetadata \
    qfilesystementry \
    qfilesystemwatcher \
    qiodevice \
    qipaddress \
    qlockfile \
    qloggingcategory \
    qloggingregistry \
    qnodebug \
    qprocess \
    qprocess-noapplication \
    qprocessenvironment \
    qresourceengine \
    qsettings \
    qsavefile \
    qstandardpaths \
    qstorageinfo \
    qtemporarydir \
    qtemporaryfile \
    qtextstream \
    qurl \
    qurlinternal \
    qurlquery \
    qwinoverlappedionotifier \

!win32 {
    SUBDIRS -=\
        qwinoverlappedionotifier
}

!qtHaveModule(gui): SUBDIRS -= \
    qdatastream \
    qsettings

!qtHaveModule(network): SUBDIRS -= \
    qiodevice \
    qprocess \
    qtextstream

!qtHaveModule(concurrent): SUBDIRS -= \
    qdebug \
    qlockfile \
    qurl

!qtConfig(private_tests): SUBDIRS -= \
    qabstractfileengine \
    qfileinfo \
    qipaddress \
    qurlinternal \
    qloggingregistry

win32:!qtConfig(private_tests): SUBDIRS -= \
    qfilesystementry

!qtConfig(processenvironment): SUBDIRS -= \
    qprocessenvironment

!qtConfig(process): SUBDIRS -= \
    qprocess \
    qprocess-noapplication

winrt: SUBDIRS -= \
    qstorageinfo \
    qwinoverlappedionotifier
