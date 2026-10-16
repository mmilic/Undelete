#include <windows.h>
#include <string.h>
#include <wchar.h>

#include "FileContents.h"

#define SIG_ANYWHERE 0xFFFFFFFF // the signature may be anywhere in the first KB (PDF)
#define SIG_TEXT 0xFFFFFFFE // no signature - plain text, checked for binary data

struct FileSignature {
    const wchar_t *szExtension;
    DWORD dwOffset; // where the bytes are, or SIG_ANYWHERE / SIG_TEXT
    const char *szBytes;
    DWORD dwLen;
};

// a type may have several rows - any one of them matching is enough
#define SIG(ext, offset, bytes) {ext, offset, bytes, sizeof (bytes) - 1}
#define TEXT_TYPE(ext) {ext, SIG_TEXT, "", 0}

static const FileSignature s_aSignatures[] = {
    SIG(L"jpg", 0, "\xFF\xD8\xFF"), SIG(L"jpeg", 0, "\xFF\xD8\xFF"), SIG(L"jpe", 0, "\xFF\xD8\xFF"),
    SIG(L"png", 0, "\x89PNG\r\n\x1A\n"),
    SIG(L"gif", 0, "GIF87a"), SIG(L"gif", 0, "GIF89a"),
    SIG(L"bmp", 0, "BM"),
    SIG(L"ico", 0, "\0\0\1\0"),
    SIG(L"psd", 0, "8BPS"),
    SIG(L"webp", 8, "WEBP"),
    // TIFF, and the camera RAW formats built on it
    SIG(L"tif", 0, "II*\0"), SIG(L"tif", 0, "MM\0*"), SIG(L"tiff", 0, "II*\0"), SIG(L"tiff", 0, "MM\0*"),
    SIG(L"nef", 0, "II*\0"), SIG(L"nef", 0, "MM\0*"), SIG(L"dng", 0, "II*\0"), SIG(L"dng", 0, "MM\0*"),
    SIG(L"arw", 0, "II*\0"), SIG(L"cr2", 0, "II*\0"),
    // ISO base media (MP4 family): a box size, then the box type
    SIG(L"mp4", 4, "ftyp"), SIG(L"mp4", 4, "moov"), SIG(L"mp4", 4, "mdat"), SIG(L"mp4", 4, "free"),
    SIG(L"m4v", 4, "ftyp"), SIG(L"m4a", 4, "ftyp"), SIG(L"3gp", 4, "ftyp"),
    SIG(L"heic", 4, "ftyp"), SIG(L"heif", 4, "ftyp"),
    SIG(L"mov", 4, "ftyp"), SIG(L"mov", 4, "moov"), SIG(L"mov", 4, "mdat"), SIG(L"mov", 4, "wide"),
    SIG(L"mov", 4, "free"), SIG(L"mov", 4, "skip"), SIG(L"mov", 4, "pnot"),
    SIG(L"avi", 8, "AVI "),
    SIG(L"wmv", 0, "\x30\x26\xB2\x75\x8E\x66\xCF\x11"), SIG(L"wma", 0, "\x30\x26\xB2\x75\x8E\x66\xCF\x11"),
    SIG(L"mkv", 0, "\x1A\x45\xDF\xA3"), SIG(L"webm", 0, "\x1A\x45\xDF\xA3"),
    SIG(L"mpg", 0, "\0\0\1\xBA"), SIG(L"mpg", 0, "\0\0\1\xB3"), SIG(L"mpeg", 0, "\0\0\1\xBA"),
    SIG(L"mpeg", 0, "\0\0\1\xB3"), SIG(L"vob", 0, "\0\0\1\xBA"),
    SIG(L"flv", 0, "FLV"),
    SIG(L"mts", 0, "\x47"), SIG(L"m2ts", 4, "\x47"),
    // MP3: an ID3 tag, or straight away an MPEG audio frame
    SIG(L"mp3", 0, "ID3"), SIG(L"mp3", 0, "\xFF\xFB"), SIG(L"mp3", 0, "\xFF\xFA"), SIG(L"mp3", 0, "\xFF\xF3"),
    SIG(L"mp3", 0, "\xFF\xF2"), SIG(L"mp3", 0, "\xFF\xE3"), SIG(L"mp3", 0, "\xFF\xE2"),
    SIG(L"wav", 8, "WAVE"),
    SIG(L"flac", 0, "fLaC"),
    SIG(L"ogg", 0, "OggS"), SIG(L"opus", 0, "OggS"),
    SIG(L"aac", 0, "\xFF\xF1"), SIG(L"aac", 0, "\xFF\xF9"), SIG(L"aac", 0, "ADIF"), SIG(L"aac", 0, "ID3"),
    SIG(L"aif", 0, "FORM"), SIG(L"aiff", 0, "FORM"),
    SIG(L"mid", 0, "MThd"), SIG(L"midi", 0, "MThd"),

    SIG(L"pdf", SIG_ANYWHERE, "%PDF"),
    // old Office: an OLE compound file
    SIG(L"doc", 0, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"), SIG(L"xls", 0, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"),
    SIG(L"ppt", 0, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"),
    // new Office, OpenDocument, EPUB, iWork: ZIP files
    SIG(L"docx", 0, "PK\3\4"), SIG(L"xlsx", 0, "PK\3\4"), SIG(L"pptx", 0, "PK\3\4"), SIG(L"odt", 0, "PK\3\4"),
    SIG(L"ods", 0, "PK\3\4"), SIG(L"odp", 0, "PK\3\4"), SIG(L"epub", 0, "PK\3\4"), SIG(L"pages", 0, "PK\3\4"),
    SIG(L"numbers", 0, "PK\3\4"), SIG(L"key", 0, "PK\3\4"),
    SIG(L"rtf", 0, "{\\rtf"),

    SIG(L"zip", 0, "PK\3\4"), SIG(L"zip", 0, "PK\5\6"),
    SIG(L"rar", 0, "Rar!\x1A\x07"),
    SIG(L"7z", 0, "7z\xBC\xAF\x27\x1C"),
    SIG(L"gz", 0, "\x1F\x8B"), SIG(L"tgz", 0, "\x1F\x8B"),
    SIG(L"bz2", 0, "BZh"),
    SIG(L"xz", 0, "\xFD" "7zXZ\0"),
    SIG(L"cab", 0, "MSCF"),
    SIG(L"tar", 257, "ustar"),

    TEXT_TYPE(L"txt"), TEXT_TYPE(L"csv"), TEXT_TYPE(L"log"), TEXT_TYPE(L"md"), TEXT_TYPE(L"xml"), TEXT_TYPE(L"json"), TEXT_TYPE(L"htm"),
    TEXT_TYPE(L"html"), TEXT_TYPE(L"ini"), TEXT_TYPE(L"svg"),
};

#define SIGNATURE_COUNT (sizeof (s_aSignatures) / sizeof (s_aSignatures[0]))

const char *ContentLabel(int iContent) {
    switch (iContent) {
        case CONTENT_OK: return "Looks valid";
        case CONTENT_DAMAGED: return "Damaged";
        case CONTENT_ZEROS: return "Damaged (zeros)";
        case CONTENT_START_OVERWRITTEN: return "Start overwritten";
        case CONTENT_NO_SIGNATURE: return "Can't check";
        case CONTENT_UNREADABLE: return "Can't read";
        default: return "";
    }
}

// the extension without the dot, or NULL (".profile" is a name, not an extension)

static const wchar_t *ExtensionOf(const wchar_t *szFilename) {
    const wchar_t *pDot = wcsrchr(szFilename, L'.');
    if (!pDot || pDot == szFilename || !pDot[1])
        return NULL;
    return pDot + 1;
}

bool CanCheckFileContents(const wchar_t *szFilename) {
    const wchar_t *szExt = ExtensionOf(szFilename);
    if (szExt)
        for (size_t i = 0; i < SIGNATURE_COUNT; i++)
            if (!_wcsicmp(szExt, s_aSignatures[i].szExtension))
                return true;
    return false;
}

// text: no NUL bytes (unless it is UTF-16) and hardly any control characters - random data has plenty of both

static bool LooksLikeText(const BYTE *p, DWORD dwLen) {
    if (dwLen >= 2 && ((p[0] == 0xFF && p[1] == 0xFE) || (p[0] == 0xFE && p[1] == 0xFF)))
        return true; // UTF-16 byte order mark
    DWORD dwControl = 0;
    for (DWORD i = 0; i < dwLen; i++) {
        if (p[i] == 0)
            return false;
        if (p[i] < 0x20 && p[i] != '\t' && p[i] != '\n' && p[i] != '\r' && p[i] != '\f' && p[i] != 0x1B)
            dwControl++;
    }
    return dwControl * 100 <= dwLen * 2; // at most 2%
}

int ClassifyFileContents(const wchar_t *szFilename, const BYTE *puchStart, DWORD dwLen) {
    const wchar_t *szExt = ExtensionOf(szFilename);
    if (!szExt || !CanCheckFileContents(szFilename) || !dwLen)
        return CONTENT_NO_SIGNATURE;

    DWORD i;
    for (i = 0; i < dwLen && !puchStart[i]; i++)
        ;
    if (i == dwLen)
        return CONTENT_ZEROS;

    for (i = 0; i < SIGNATURE_COUNT; i++) {
        const FileSignature &sig = s_aSignatures[i];
        if (_wcsicmp(szExt, sig.szExtension))
            continue;

        if (sig.dwOffset == SIG_TEXT) {
            if (LooksLikeText(puchStart, dwLen))
                return CONTENT_OK;
        } else if (sig.dwOffset == SIG_ANYWHERE) {
            DWORD dwEnd = (dwLen < 1024) ? dwLen : 1024;
            for (DWORD dwPos = 0; dwPos + sig.dwLen <= dwEnd; dwPos++)
                if (!memcmp(puchStart + dwPos, sig.szBytes, sig.dwLen))
                    return CONTENT_OK;
        } else if (sig.dwOffset + sig.dwLen <= dwLen && !memcmp(puchStart + sig.dwOffset, sig.szBytes, sig.dwLen)) {
            return CONTENT_OK;
        }
    }
    return CONTENT_DAMAGED;
}
