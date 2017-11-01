/****************************************************************************
**
** Copyright (C) 2017 Intel Corporation.
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2013 Samuel Gaist <samuel.gaist@edeltech.ch>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qfilesystemengine_p.h"
#include "qfile.h"

#include <QtCore/qvarlengtharray.h>

#include <pwd.h>
#include <stdlib.h> // for realpath()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


#if defined(Q_OS_MAC)
# include <QtCore/private/qcore_mac_p.h>
# include <CoreFoundation/CFBundle.h>
#endif

#ifdef Q_OS_OSX
#include <CoreServices/CoreServices.h>
#endif

#if defined(QT_PLATFORM_UIKIT)
#include <MobileCoreServices/MobileCoreServices.h>
#endif

#if defined(Q_OS_DARWIN)
// We cannot include <Foundation/Foundation.h> (it's an Objective-C header), but
// we need these declarations:
Q_FORWARD_DECLARE_OBJC_CLASS(NSString);
extern "C" NSString *NSTemporaryDirectory();
#endif

QT_BEGIN_NAMESPACE

#if defined(Q_OS_DARWIN)
static inline bool hasResourcePropertyFlag(const QFileSystemMetaData &data,
                                           const QFileSystemEntry &entry,
                                           CFStringRef key)
{
    QCFString path = CFStringCreateWithFileSystemRepresentation(0,
        entry.nativeFilePath().constData());
    if (!path)
        return false;

    QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, path, kCFURLPOSIXPathStyle,
        data.hasFlags(QFileSystemMetaData::DirectoryType));
    if (!url)
        return false;

    CFBooleanRef value;
    if (CFURLCopyResourcePropertyForKey(url, key, &value, NULL)) {
        if (value == kCFBooleanTrue)
            return true;
    }

    return false;
}

static bool isPackage(const QFileSystemMetaData &data, const QFileSystemEntry &entry)
{
    if (!data.isDirectory())
        return false;

    QFileInfo info(entry.filePath());
    QString suffix = info.suffix();

    if (suffix.length() > 0) {
        // First step: is the extension known ?
        QCFType<CFStringRef> extensionRef = suffix.toCFString();
        QCFType<CFStringRef> uniformTypeIdentifier = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, extensionRef, NULL);
        if (UTTypeConformsTo(uniformTypeIdentifier, kUTTypeBundle))
            return true;

        // Second step: check if an application knows the package type
        QCFType<CFStringRef> path = entry.filePath().toCFString();
        QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, path, kCFURLPOSIXPathStyle, true);

        UInt32 type, creator;
        // Well created packages have the PkgInfo file
        if (CFBundleGetPackageInfoInDirectory(url, &type, &creator))
            return true;

#ifdef Q_OS_MACOS
        // Find if an application other than Finder claims to know how to handle the package
        QCFType<CFURLRef> application = LSCopyDefaultApplicationURLForURL(url,
            kLSRolesEditor | kLSRolesViewer, nullptr);

        if (application) {
            QCFType<CFBundleRef> bundle = CFBundleCreate(kCFAllocatorDefault, application);
            CFStringRef identifier = CFBundleGetIdentifier(bundle);
            QString applicationId = QString::fromCFString(identifier);
            if (applicationId != QLatin1String("com.apple.finder"))
                return true;
        }
#endif
    }

    // Third step: check if the directory has the package bit set
    return hasResourcePropertyFlag(data, entry, kCFURLIsPackageKey);
}
#endif

//static
QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link, QFileSystemMetaData &data)
{
#if defined(__GLIBC__) && !defined(PATH_MAX)
#define PATH_CHUNK_SIZE 256
    char *s = 0;
    int len = -1;
    int size = PATH_CHUNK_SIZE;

    while (1) {
        s = (char *) ::realloc(s, size);
        Q_CHECK_PTR(s);
        len = ::readlink(link.nativeFilePath().constData(), s, size);
        if (len < 0) {
            ::free(s);
            break;
        }
        if (len < size) {
            break;
        }
        size *= 2;
    }
#else
    char s[PATH_MAX+1];
    int len = readlink(link.nativeFilePath().constData(), s, PATH_MAX);
#endif
    if (len > 0) {
        QString ret;
        if (!data.hasFlags(QFileSystemMetaData::DirectoryType))
            fillMetaData(link, data, QFileSystemMetaData::DirectoryType);
        if (data.isDirectory() && s[0] != '/') {
            QDir parent(link.filePath());
            parent.cdUp();
            ret = parent.path();
            if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
                ret += QLatin1Char('/');
        }
        s[len] = '\0';
        ret += QFile::decodeName(QByteArray(s));
#if defined(__GLIBC__) && !defined(PATH_MAX)
        ::free(s);
#endif

        if (!ret.startsWith(QLatin1Char('/'))) {
            const QString linkPath = link.path();
            if (linkPath.startsWith(QLatin1Char('/')))
                ret.prepend(linkPath + QLatin1Char('/'));
            else
                ret.prepend(QDir::currentPath() + QLatin1Char('/') + linkPath + QLatin1Char('/'));
        }
        ret = QDir::cleanPath(ret);
        if (ret.size() > 1 && ret.endsWith(QLatin1Char('/')))
            ret.chop(1);
        return QFileSystemEntry(ret);
    }
#if defined(Q_OS_DARWIN)
    {
        QCFString path = CFStringCreateWithFileSystemRepresentation(0,
            QFile::encodeName(QDir::cleanPath(link.filePath())).data());
        if (!path)
            return QFileSystemEntry();

        QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, path, kCFURLPOSIXPathStyle,
            data.hasFlags(QFileSystemMetaData::DirectoryType));
        if (!url)
            return QFileSystemEntry();

        QCFType<CFDataRef> bookmarkData = CFURLCreateBookmarkDataFromFile(0, url, NULL);
        if (!bookmarkData)
            return QFileSystemEntry();

        QCFType<CFURLRef> resolvedUrl = CFURLCreateByResolvingBookmarkData(0,
            bookmarkData,
            (CFURLBookmarkResolutionOptions)(kCFBookmarkResolutionWithoutUIMask
                | kCFBookmarkResolutionWithoutMountingMask), NULL, NULL, NULL, NULL);
        if (!resolvedUrl)
            return QFileSystemEntry();

        QCFString cfstr(CFURLCopyFileSystemPath(resolvedUrl, kCFURLPOSIXPathStyle));
        if (!cfstr)
            return QFileSystemEntry();

        return QFileSystemEntry(QString::fromCFString(cfstr));
    }
#endif
    return QFileSystemEntry();
}

//static
QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry, QFileSystemMetaData &data)
{
    if (entry.isEmpty() || entry.isRoot())
        return entry;

#if !defined(Q_OS_MAC) && !defined(Q_OS_QNX) && !defined(Q_OS_ANDROID) && !defined(Q_OS_HAIKU) && _POSIX_VERSION < 200809L
    // realpath(X,0) is not supported
    Q_UNUSED(data);
    return QFileSystemEntry(slowCanonicalized(absoluteName(entry).filePath()));
#else
    char *ret = 0;
# if defined(Q_OS_DARWIN)
    ret = (char*)malloc(PATH_MAX + 1);
    if (ret && realpath(entry.nativeFilePath().constData(), (char*)ret) == 0) {
        const int savedErrno = errno; // errno is checked below, and free() might change it
        free(ret);
        errno = savedErrno;
        ret = 0;
    }
# elif defined(Q_OS_ANDROID)
    // On some Android versions, realpath() will return a path even if it does not exist
    // To work around this, we check existence in advance.
    if (!data.hasFlags(QFileSystemMetaData::ExistsAttribute))
        fillMetaData(entry, data, QFileSystemMetaData::ExistsAttribute);

    if (!data.exists()) {
        ret = 0;
        errno = ENOENT;
    } else {
        ret = (char*)malloc(PATH_MAX + 1);
        if (realpath(entry.nativeFilePath().constData(), (char*)ret) == 0) {
            const int savedErrno = errno; // errno is checked below, and free() might change it
            free(ret);
            errno = savedErrno;
            ret = 0;
        }
    }

# else
#  if _POSIX_VERSION >= 200801L
    ret = realpath(entry.nativeFilePath().constData(), (char*)0);
#  else
    ret = (char*)malloc(PATH_MAX + 1);
    if (realpath(entry.nativeFilePath().constData(), (char*)ret) == 0) {
        const int savedErrno = errno; // errno is checked below, and free() might change it
        free(ret);
        errno = savedErrno;
        ret = 0;
    }
#  endif
# endif
    if (ret) {
        data.knownFlagsMask |= QFileSystemMetaData::ExistsAttribute;
        data.entryFlags |= QFileSystemMetaData::ExistsAttribute;
        QString canonicalPath = QDir::cleanPath(QFile::decodeName(ret));
        free(ret);
        return QFileSystemEntry(canonicalPath);
    } else if (errno == ENOENT) { // file doesn't exist
        data.knownFlagsMask |= QFileSystemMetaData::ExistsAttribute;
        data.entryFlags &= ~(QFileSystemMetaData::ExistsAttribute);
        return QFileSystemEntry();
    }
    return entry;
#endif
}

//static
QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
    if (entry.isAbsolute() && entry.isClean())
        return entry;

    QByteArray orig = entry.nativeFilePath();
    QByteArray result;
    if (orig.isEmpty() || !orig.startsWith('/')) {
        QFileSystemEntry cur(currentPath());
        result = cur.nativeFilePath();
    }
    if (!orig.isEmpty() && !(orig.length() == 1 && orig[0] == '.')) {
        if (!result.isEmpty() && !result.endsWith('/'))
            result.append('/');
        result.append(orig);
    }

    if (result.length() == 1 && result[0] == '/')
        return QFileSystemEntry(result, QFileSystemEntry::FromNativePath());
    const bool isDir = result.endsWith('/');

    /* as long as QDir::cleanPath() operates on a QString we have to convert to a string here.
     * ideally we never convert to a string since that loses information. Please fix after
     * we get a QByteArray version of QDir::cleanPath()
     */
    QFileSystemEntry resultingEntry(result, QFileSystemEntry::FromNativePath());
    QString stringVersion = QDir::cleanPath(resultingEntry.filePath());
    if (isDir)
        stringVersion.append(QLatin1Char('/'));
    return QFileSystemEntry(stringVersion);
}

//static
QByteArray QFileSystemEngine::id(const QFileSystemEntry &entry)
{
    QT_STATBUF statResult;
    if (QT_STAT(entry.nativeFilePath().constData(), &statResult)) {
        if (errno != ENOENT)
            qErrnoWarning("stat() failed for '%s'", entry.nativeFilePath().constData());
        return QByteArray();
    }
    QByteArray result = QByteArray::number(quint64(statResult.st_dev), 16);
    result += ':';
    result += QByteArray::number(quint64(statResult.st_ino));
    return result;
}

//static
QByteArray QFileSystemEngine::id(int id)
{
    QT_STATBUF statResult;
    if (QT_FSTAT(id, &statResult)) {
        qErrnoWarning("fstat() failed for fd %d", id);
        return QByteArray();
    }
    QByteArray result = QByteArray::number(quint64(statResult.st_dev), 16);
    result += ':';
    result += QByteArray::number(quint64(statResult.st_ino));
    return result;
}

//static
QString QFileSystemEngine::resolveUserName(uint userId)
{
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
    int size_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size_max == -1)
        size_max = 1024;
    QVarLengthArray<char, 1024> buf(size_max);
#endif

#if !defined(Q_OS_INTEGRITY)
    struct passwd *pw = 0;
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD) && !defined(Q_OS_VXWORKS)
    struct passwd entry;
    getpwuid_r(userId, &entry, buf.data(), buf.size(), &pw);
#else
    pw = getpwuid(userId);
#endif
    if (pw)
        return QFile::decodeName(QByteArray(pw->pw_name));
#endif
    return QString();
}

//static
QString QFileSystemEngine::resolveGroupName(uint groupId)
{
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
    int size_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size_max == -1)
        size_max = 1024;
    QVarLengthArray<char, 1024> buf(size_max);
#endif

#if !defined(Q_OS_INTEGRITY)
    struct group *gr = 0;
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD) && !defined(Q_OS_VXWORKS) && (!defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID) && (__ANDROID_API__ >= 24))
    size_max = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (size_max == -1)
        size_max = 1024;
    buf.resize(size_max);
    struct group entry;
    // Some large systems have more members than the POSIX max size
    // Loop over by doubling the buffer size (upper limit 250k)
    for (unsigned size = size_max; size < 256000; size += size)
    {
        buf.resize(size);
        // ERANGE indicates that the buffer was too small
        if (!getgrgid_r(groupId, &entry, buf.data(), buf.size(), &gr)
            || errno != ERANGE)
            break;
    }
#else
    gr = getgrgid(groupId);
#endif
    if (gr)
        return QFile::decodeName(QByteArray(gr->gr_name));
#endif
    return QString();
}

#if defined(Q_OS_DARWIN)
//static
QString QFileSystemEngine::bundleName(const QFileSystemEntry &entry)
{
    QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, QCFString(entry.filePath()),
            kCFURLPOSIXPathStyle, true);
    if (QCFType<CFDictionaryRef> dict = CFBundleCopyInfoDictionaryForURL(url)) {
        if (CFTypeRef name = (CFTypeRef)CFDictionaryGetValue(dict, kCFBundleNameKey)) {
            if (CFGetTypeID(name) == CFStringGetTypeID())
                return QString::fromCFString((CFStringRef)name);
        }
    }
    return QString();
}
#endif

//static
bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data,
        QFileSystemMetaData::MetaDataFlags what)
{
#if defined(Q_OS_DARWIN)
    if (what & QFileSystemMetaData::BundleType) {
        if (!data.hasFlags(QFileSystemMetaData::DirectoryType))
            what |= QFileSystemMetaData::DirectoryType;
    }
    if (what & QFileSystemMetaData::HiddenAttribute) {
        // OS X >= 10.5: st_flags & UF_HIDDEN
        what |= QFileSystemMetaData::PosixStatFlags;
    }
#endif // defined(Q_OS_DARWIN)

    if (what & QFileSystemMetaData::PosixStatFlags)
        what |= QFileSystemMetaData::PosixStatFlags;

    if (what & QFileSystemMetaData::ExistsAttribute) {
        //  FIXME:  Would other queries being performed provide this bit?
        what |= QFileSystemMetaData::PosixStatFlags;
    }

    data.entryFlags &= ~what;

    const QByteArray nativeFilePath = entry.nativeFilePath();
    bool entryExists = true; // innocent until proven otherwise

    QT_STATBUF statBuffer;
    bool statBufferValid = false;
    if (what & QFileSystemMetaData::LinkType) {
        if (QT_LSTAT(nativeFilePath, &statBuffer) == 0) {
            if (S_ISLNK(statBuffer.st_mode)) {
                data.entryFlags |= QFileSystemMetaData::LinkType;
            } else {
                statBufferValid = true;
                data.entryFlags &= ~QFileSystemMetaData::PosixStatFlags;
            }
        } else {
            entryExists = false;
        }

        data.knownFlagsMask |= QFileSystemMetaData::LinkType;
    }

    if (statBufferValid || (what & QFileSystemMetaData::PosixStatFlags)) {
        if (entryExists && !statBufferValid)
            statBufferValid = (QT_STAT(nativeFilePath, &statBuffer) == 0);

        if (statBufferValid)
            data.fillFromStatBuf(statBuffer);
        else {
            entryExists = false;
            data.creationTime_ = 0;
            data.modificationTime_ = 0;
            data.accessTime_ = 0;
            data.size_ = 0;
            data.userId_ = (uint) -2;
            data.groupId_ = (uint) -2;
        }

        // reset the mask
        data.knownFlagsMask |= QFileSystemMetaData::PosixStatFlags
            | QFileSystemMetaData::ExistsAttribute;
    }

#if defined(Q_OS_DARWIN)
    if (what & QFileSystemMetaData::AliasType)
    {
        if (entryExists && hasResourcePropertyFlag(data, entry, kCFURLIsAliasFileKey))
            data.entryFlags |= QFileSystemMetaData::AliasType;
        data.knownFlagsMask |= QFileSystemMetaData::AliasType;
    }
#endif

    if (what & QFileSystemMetaData::UserPermissions) {
        // calculate user permissions

        if (entryExists) {
            if (what & QFileSystemMetaData::UserReadPermission) {
                if (QT_ACCESS(nativeFilePath, R_OK) == 0)
                    data.entryFlags |= QFileSystemMetaData::UserReadPermission;
            }
            if (what & QFileSystemMetaData::UserWritePermission) {
                if (QT_ACCESS(nativeFilePath, W_OK) == 0)
                    data.entryFlags |= QFileSystemMetaData::UserWritePermission;
            }
            if (what & QFileSystemMetaData::UserExecutePermission) {
                if (QT_ACCESS(nativeFilePath, X_OK) == 0)
                    data.entryFlags |= QFileSystemMetaData::UserExecutePermission;
            }
        }
        data.knownFlagsMask |= (what & QFileSystemMetaData::UserPermissions);
    }

    if (what & QFileSystemMetaData::HiddenAttribute
            && !data.isHidden()) {
        QString fileName = entry.fileName();
        if ((fileName.size() > 0 && fileName.at(0) == QLatin1Char('.'))
#if defined(Q_OS_DARWIN)
                || (entryExists && hasResourcePropertyFlag(data, entry, kCFURLIsHiddenKey))
#endif
                )
            data.entryFlags |= QFileSystemMetaData::HiddenAttribute;
        data.knownFlagsMask |= QFileSystemMetaData::HiddenAttribute;
    }

#if defined(Q_OS_DARWIN)
    if (what & QFileSystemMetaData::BundleType) {
        if (entryExists && isPackage(data, entry))
            data.entryFlags |= QFileSystemMetaData::BundleType;

        data.knownFlagsMask |= QFileSystemMetaData::BundleType;
    }
#endif
    if (!entryExists) {
        data.clearFlags(what);
        return false;
    }
    return data.hasFlags(what);
}

// Note: if \a shouldMkdirFirst is false, we assume the caller did try to mkdir
// before calling this function.
static bool createDirectoryWithParents(const QByteArray &nativeName, bool shouldMkdirFirst = true)
{
    // helper function to check if a given path is a directory, since mkdir can
    // fail if the dir already exists (it may have been created by another
    // thread or another process)
    const auto isDir = [](const QByteArray &nativeName) {
        QT_STATBUF st;
        return QT_STAT(nativeName.constData(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR;
    };

    if (shouldMkdirFirst && QT_MKDIR(nativeName, 0777) == 0)
        return true;
    if (errno == EEXIST)
        return isDir(nativeName);
    if (errno != ENOENT)
        return false;

    // mkdir failed because the parent dir doesn't exist, so try to create it
    int slash = nativeName.lastIndexOf('/');
    if (slash < 1)
        return false;

    QByteArray parentNativeName = nativeName.left(slash);
    if (!createDirectoryWithParents(parentNativeName))
        return false;

    // try again
    if (QT_MKDIR(nativeName, 0777) == 0)
        return true;
    return errno == EEXIST && isDir(nativeName);
}

//static
bool QFileSystemEngine::createDirectory(const QFileSystemEntry &entry, bool createParents)
{
    QString dirName = entry.filePath();

    // Darwin doesn't support trailing /'s, so remove for everyone
    while (dirName.size() > 1 && dirName.endsWith(QLatin1Char('/')))
        dirName.chop(1);

    // try to mkdir this directory
    QByteArray nativeName = QFile::encodeName(dirName);
    if (QT_MKDIR(nativeName, 0777) == 0)
        return true;
    if (!createParents)
        return false;

    return createDirectoryWithParents(nativeName, false);
}

//static
bool QFileSystemEngine::removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents)
{
    if (removeEmptyParents) {
        QString dirName = QDir::cleanPath(entry.filePath());
        for (int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            const QByteArray chunk = QFile::encodeName(dirName.left(slash));
            QT_STATBUF st;
            if (QT_STAT(chunk.constData(), &st) != -1) {
                if ((st.st_mode & S_IFMT) != S_IFDIR)
                    return false;
                if (::rmdir(chunk.constData()) != 0)
                    return oldslash != 0;
            } else {
                return false;
            }
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    return rmdir(QFile::encodeName(entry.filePath()).constData()) == 0;
}

//static
bool QFileSystemEngine::createLink(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
    if (::symlink(source.nativeFilePath().constData(), target.nativeFilePath().constData()) == 0)
        return true;
    error = QSystemError(errno, QSystemError::StandardLibraryError);
    return false;
}

//static
bool QFileSystemEngine::copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
    Q_UNUSED(source);
    Q_UNUSED(target);
    error = QSystemError(ENOSYS, QSystemError::StandardLibraryError); //Function not implemented
    return false;
}

//static
bool QFileSystemEngine::renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
    if (::rename(source.nativeFilePath().constData(), target.nativeFilePath().constData()) == 0)
        return true;
    error = QSystemError(errno, QSystemError::StandardLibraryError);
    return false;
}

//static
bool QFileSystemEngine::removeFile(const QFileSystemEntry &entry, QSystemError &error)
{
    if (unlink(entry.nativeFilePath().constData()) == 0)
        return true;
    error = QSystemError(errno, QSystemError::StandardLibraryError);
    return false;

}

static mode_t toMode_t(QFile::Permissions permissions)
{
    mode_t mode = 0;
    if (permissions & (QFile::ReadOwner | QFile::ReadUser))
        mode |= S_IRUSR;
    if (permissions & (QFile::WriteOwner | QFile::WriteUser))
        mode |= S_IWUSR;
    if (permissions & (QFile::ExeOwner | QFile::ExeUser))
        mode |= S_IXUSR;
    if (permissions & QFile::ReadGroup)
        mode |= S_IRGRP;
    if (permissions & QFile::WriteGroup)
        mode |= S_IWGRP;
    if (permissions & QFile::ExeGroup)
        mode |= S_IXGRP;
    if (permissions & QFile::ReadOther)
        mode |= S_IROTH;
    if (permissions & QFile::WriteOther)
        mode |= S_IWOTH;
    if (permissions & QFile::ExeOther)
        mode |= S_IXOTH;
    return mode;
}

//static
bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions, QSystemError &error, QFileSystemMetaData *data)
{
    mode_t mode = toMode_t(permissions);

    bool success = ::chmod(entry.nativeFilePath().constData(), mode) == 0;
    if (success && data) {
        data->entryFlags &= ~QFileSystemMetaData::Permissions;
        data->entryFlags |= QFileSystemMetaData::MetaDataFlag(uint(permissions));
        data->knownFlagsMask |= QFileSystemMetaData::Permissions;
    }
    if (!success)
        error = QSystemError(errno, QSystemError::StandardLibraryError);
    return success;
}

//static
bool QFileSystemEngine::setPermissions(int fd, QFile::Permissions permissions, QSystemError &error, QFileSystemMetaData *data)
{
    mode_t mode = toMode_t(permissions);

    bool success = ::fchmod(fd, mode) == 0;
    if (success && data) {
        data->entryFlags &= ~QFileSystemMetaData::Permissions;
        data->entryFlags |= QFileSystemMetaData::MetaDataFlag(uint(permissions));
        data->knownFlagsMask |= QFileSystemMetaData::Permissions;
    }
    if (!success)
        error = QSystemError(errno, QSystemError::StandardLibraryError);
    return success;
}

QString QFileSystemEngine::homePath()
{
    QString home = QFile::decodeName(qgetenv("HOME"));
    if (home.isEmpty())
        home = rootPath();
    return QDir::cleanPath(home);
}

QString QFileSystemEngine::rootPath()
{
    return QLatin1String("/");
}

QString QFileSystemEngine::tempPath()
{
#ifdef QT_UNIX_TEMP_PATH_OVERRIDE
    return QLatin1String(QT_UNIX_TEMP_PATH_OVERRIDE);
#else
    QString temp = QFile::decodeName(qgetenv("TMPDIR"));
    if (temp.isEmpty()) {
#if defined(Q_OS_DARWIN) && !defined(QT_BOOTSTRAPPED)
        if (NSString *nsPath = NSTemporaryDirectory()) {
            temp = QString::fromCFString((CFStringRef)nsPath);
        } else {
#else
        {
#endif
            temp = QLatin1String("/tmp");
        }
    }
    return QDir::cleanPath(temp);
#endif
}

bool QFileSystemEngine::setCurrentPath(const QFileSystemEntry &path)
{
    int r;
    r = QT_CHDIR(path.nativeFilePath().constData());
    return r >= 0;
}

QFileSystemEntry QFileSystemEngine::currentPath()
{
    QFileSystemEntry result;
#if defined(__GLIBC__) && !defined(PATH_MAX)
    char *currentName = ::get_current_dir_name();
    if (currentName) {
        result = QFileSystemEntry(QByteArray(currentName), QFileSystemEntry::FromNativePath());
        ::free(currentName);
    }
#else
    char currentName[PATH_MAX+1];
    if (::getcwd(currentName, PATH_MAX)) {
#if defined(Q_OS_VXWORKS) && defined(VXWORKS_VXSIM)
        QByteArray dir(currentName);
        if (dir.indexOf(':') < dir.indexOf('/'))
            dir.remove(0, dir.indexOf(':')+1);

        qstrncpy(currentName, dir.constData(), PATH_MAX);
#endif
        result = QFileSystemEntry(QByteArray(currentName), QFileSystemEntry::FromNativePath());
    }
# if defined(QT_DEBUG)
    if (result.isEmpty())
        qWarning("QFileSystemEngine::currentPath: getcwd() failed");
# endif
#endif
    return result;
}
QT_END_NAMESPACE
