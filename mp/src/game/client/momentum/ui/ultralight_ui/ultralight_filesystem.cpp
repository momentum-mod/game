#include "cbase.h"
#include "ultralight_filesystem.h"

#include "filesystem.h"
#include "utlbuffer.h"

using namespace ultralight;

static const char *FileExtensionToMimeType(const char *ext);
static const char *ToUTF8(const String16 &str)
{
    String utf8_str(str);
    return utf8_str.utf8().data();
}

bool SourceFileSystem::FileExists(const String16 &path) { return g_pFullFileSystem->FileExists(ToUTF8(path)); }

bool SourceFileSystem::DeleteFile_(const String16 &path)
{
    g_pFullFileSystem->RemoveFile(ToUTF8(path));
    return true;
}

bool SourceFileSystem::DeleteEmptyDirectory(const String16 &path) { return false; }

bool SourceFileSystem::MoveFile_(const String16 &old_path, const String16 &new_path)
{
    return g_pFullFileSystem->RenameFile(ToUTF8(old_path), ToUTF8(new_path));
}

bool SourceFileSystem::GetFileSize(const String16 &path, int64_t &result)
{
    result = (int64_t)g_pFullFileSystem->Size(ToUTF8(path));
    return true;
}

bool SourceFileSystem::GetFileSize(FileHandle handle, int64_t &result)
{
    result = (int64_t)g_pFullFileSystem->Size((FileHandle_t)handle);
    return true;
}

bool SourceFileSystem::GetFileMimeType(const String16 &path, String16 &result)
{
    const char *szPath = ToUTF8(path);
    const char *szExtension = &Q_GetFileExtension(szPath)[1];
    result = String16(FileExtensionToMimeType(szExtension));

    return true;
}

bool SourceFileSystem::GetFileModificationTime(const String16 &path, time_t &result) { return false; }

bool SourceFileSystem::GetFileCreationTime(const String16 &path, time_t &result) { return false; }

MetadataType SourceFileSystem::GetMetadataType(const String16 &path)
{
    return g_pFullFileSystem->IsDirectory(ToUTF8(path)) ? MetadataType::kMetadataType_Directory
                                                        : MetadataType::kMetadataType_File;
}

String16 SourceFileSystem::GetPathByAppendingComponent(const String16 &path, const String16 &component)
{
    return path + component;
}

bool SourceFileSystem::CreateDirectory_(const String16 &path) { return false; }

String16 SourceFileSystem::GetHomeDirectory()
{
    char szPath[MAX_PATH];
    g_pFullFileSystem->RelativePathToFullPath(".", "MOD", szPath, sizeof(szPath));
    return String16(szPath);
}

String16 SourceFileSystem::GetFilenameFromPath(const String16 &path)
{
    char szFileBase[MAX_PATH];
    Q_FileBase(ToUTF8(path), szFileBase, sizeof(szFileBase));
    Q_DefaultExtension(szFileBase, Q_GetFileExtension(ToUTF8(path)), sizeof(szFileBase));

    return String16(szFileBase);
}

String16 SourceFileSystem::GetDirectoryNameFromPath(const String16 &path)
{
    char szDirBase[MAX_PATH];
    Q_FileBase(ToUTF8(path), szDirBase, sizeof(szDirBase));

	return String16(szDirBase);
}

bool SourceFileSystem::GetVolumeFreeSpace(const String16 &path, uint64_t &result) { return false; }

int32_t SourceFileSystem::GetVolumeId(const String16 &path) { return false; }

Ref<String16Vector> SourceFileSystem::ListDirectory(const String16 &path, const String16 &filter)
{
    auto result = String16Vector::Create();
    const char *searchpath = ToUTF8(path);
    FileFindHandle_t hfind = FILESYSTEM_INVALID_FIND_HANDLE;
    const char *findfn = g_pFullFileSystem->FindFirst(searchpath, &hfind);

    while (findfn)
    {
        result->push_back(String16(findfn));
        findfn = g_pFullFileSystem->FindNext(hfind);
    }

    g_pFullFileSystem->FindClose(hfind);

	return result;
}

String16 SourceFileSystem::OpenTemporaryFile(const String16 &prefix, FileHandle &handle) { return String16(); }

FileHandle SourceFileSystem::OpenFile(const String16 &path, bool open_for_writing)
{
    return (FileHandle)g_pFullFileSystem->Open(ToUTF8(path), open_for_writing ? "w" : "r");
}

void SourceFileSystem::CloseFile(FileHandle &handle) { g_pFullFileSystem->Close((FileHandle_t)handle); }

int64_t SourceFileSystem::SeekFile(FileHandle handle, int64_t offset, FileSeekOrigin origin)
{
    g_pFullFileSystem->Seek((FileHandle_t)handle, (int)offset, (FileSystemSeek_t)origin);
    return 0;
}

bool SourceFileSystem::TruncateFile(FileHandle handle, int64_t offset) { return false; }

int64_t SourceFileSystem::WriteToFile(FileHandle handle, const char *data, int64_t length)
{
    return (int64_t)g_pFullFileSystem->Write(data, (int)length, (FileHandle_t)handle);
}

int64_t SourceFileSystem::ReadFromFile(FileHandle handle, char *data, int64_t length)
{
    return (int64_t)g_pFullFileSystem->Read(data, length, (FileHandle_t)handle);
}

bool SourceFileSystem::CopyFile_(const String16 &source_path, const String16 &destination_path) { return false; }

const char *FileExtensionToMimeType(const char *ext)
{
    if (!Q_strcmp(ext, "323"))
        return "text/h323";
    if (!Q_strcmp(ext, "3g2"))
        return "video/3gpp2";
    if (!Q_strcmp(ext, "3gp"))
        return "video/3gpp";
    if (!Q_strcmp(ext, "3gp2"))
        return "video/3gpp2";
    if (!Q_strcmp(ext, "3gpp"))
        return "video/3gpp";
    if (!Q_strcmp(ext, "7z"))
        return "application/x-7z-compressed";
    if (!Q_strcmp(ext, "aa"))
        return "audio/audible";
    if (!Q_strcmp(ext, "AAC"))
        return "audio/aac";
    if (!Q_strcmp(ext, "aaf"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "aax"))
        return "audio/vnd.audible.aax";
    if (!Q_strcmp(ext, "ac3"))
        return "audio/ac3";
    if (!Q_strcmp(ext, "aca"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "accda"))
        return "application/msaccess.addin";
    if (!Q_strcmp(ext, "accdb"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "accdc"))
        return "application/msaccess.cab";
    if (!Q_strcmp(ext, "accde"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "accdr"))
        return "application/msaccess.runtime";
    if (!Q_strcmp(ext, "accdt"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "accdw"))
        return "application/msaccess.webapplication";
    if (!Q_strcmp(ext, "accft"))
        return "application/msaccess.ftemplate";
    if (!Q_strcmp(ext, "acx"))
        return "application/internet-property-stream";
    if (!Q_strcmp(ext, "AddIn"))
        return "text/xml";
    if (!Q_strcmp(ext, "ade"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "adobebridge"))
        return "application/x-bridge-url";
    if (!Q_strcmp(ext, "adp"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "ADT"))
        return "audio/vnd.dlna.adts";
    if (!Q_strcmp(ext, "ADTS"))
        return "audio/aac";
    if (!Q_strcmp(ext, "afm"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "ai"))
        return "application/postscript";
    if (!Q_strcmp(ext, "aif"))
        return "audio/aiff";
    if (!Q_strcmp(ext, "aifc"))
        return "audio/aiff";
    if (!Q_strcmp(ext, "aiff"))
        return "audio/aiff";
    if (!Q_strcmp(ext, "air"))
        return "application/vnd.adobe.air-application-installer-package+zip";
    if (!Q_strcmp(ext, "amc"))
        return "application/mpeg";
    if (!Q_strcmp(ext, "anx"))
        return "application/annodex";
    if (!Q_strcmp(ext, "apk"))
        return "application/vnd.android.package-archive";
    if (!Q_strcmp(ext, "application"))
        return "application/x-ms-application";
    if (!Q_strcmp(ext, "art"))
        return "image/x-jg";
    if (!Q_strcmp(ext, "asa"))
        return "application/xml";
    if (!Q_strcmp(ext, "asax"))
        return "application/xml";
    if (!Q_strcmp(ext, "ascx"))
        return "application/xml";
    if (!Q_strcmp(ext, "asd"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "asf"))
        return "video/x-ms-asf";
    if (!Q_strcmp(ext, "ashx"))
        return "application/xml";
    if (!Q_strcmp(ext, "asi"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "asm"))
        return "text/plain";
    if (!Q_strcmp(ext, "asmx"))
        return "application/xml";
    if (!Q_strcmp(ext, "aspx"))
        return "application/xml";
    if (!Q_strcmp(ext, "asr"))
        return "video/x-ms-asf";
    if (!Q_strcmp(ext, "asx"))
        return "video/x-ms-asf";
    if (!Q_strcmp(ext, "atom"))
        return "application/atom+xml";
    if (!Q_strcmp(ext, "au"))
        return "audio/basic";
    if (!Q_strcmp(ext, "avi"))
        return "video/x-msvideo";
    if (!Q_strcmp(ext, "axa"))
        return "audio/annodex";
    if (!Q_strcmp(ext, "axs"))
        return "application/olescript";
    if (!Q_strcmp(ext, "axv"))
        return "video/annodex";
    if (!Q_strcmp(ext, "bas"))
        return "text/plain";
    if (!Q_strcmp(ext, "bcpio"))
        return "application/x-bcpio";
    if (!Q_strcmp(ext, "bin"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "bmp"))
        return "image/bmp";
    if (!Q_strcmp(ext, "c"))
        return "text/plain";
    if (!Q_strcmp(ext, "cab"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "caf"))
        return "audio/x-caf";
    if (!Q_strcmp(ext, "calx"))
        return "application/vnd.ms-office.calx";
    if (!Q_strcmp(ext, "cat"))
        return "application/vnd.ms-pki.seccat";
    if (!Q_strcmp(ext, "cc"))
        return "text/plain";
    if (!Q_strcmp(ext, "cd"))
        return "text/plain";
    if (!Q_strcmp(ext, "cdda"))
        return "audio/aiff";
    if (!Q_strcmp(ext, "cdf"))
        return "application/x-cdf";
    if (!Q_strcmp(ext, "cer"))
        return "application/x-x509-ca-cert";
    if (!Q_strcmp(ext, "cfg"))
        return "text/plain";
    if (!Q_strcmp(ext, "chm"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "class"))
        return "application/x-java-applet";
    if (!Q_strcmp(ext, "clp"))
        return "application/x-msclip";
    if (!Q_strcmp(ext, "cmd"))
        return "text/plain";
    if (!Q_strcmp(ext, "cmx"))
        return "image/x-cmx";
    if (!Q_strcmp(ext, "cnf"))
        return "text/plain";
    if (!Q_strcmp(ext, "cod"))
        return "image/cis-cod";
    if (!Q_strcmp(ext, "config"))
        return "application/xml";
    if (!Q_strcmp(ext, "contact"))
        return "text/x-ms-contact";
    if (!Q_strcmp(ext, "coverage"))
        return "application/xml";
    if (!Q_strcmp(ext, "cpio"))
        return "application/x-cpio";
    if (!Q_strcmp(ext, "cpp"))
        return "text/plain";
    if (!Q_strcmp(ext, "crd"))
        return "application/x-mscardfile";
    if (!Q_strcmp(ext, "crl"))
        return "application/pkix-crl";
    if (!Q_strcmp(ext, "crt"))
        return "application/x-x509-ca-cert";
    if (!Q_strcmp(ext, "cs"))
        return "text/plain";
    if (!Q_strcmp(ext, "csdproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "csh"))
        return "application/x-csh";
    if (!Q_strcmp(ext, "csproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "css"))
        return "text/css";
    if (!Q_strcmp(ext, "csv"))
        return "text/csv";
    if (!Q_strcmp(ext, "cur"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "cxx"))
        return "text/plain";
    if (!Q_strcmp(ext, "dat"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "datasource"))
        return "application/xml";
    if (!Q_strcmp(ext, "dbproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "dcr"))
        return "application/x-director";
    if (!Q_strcmp(ext, "def"))
        return "text/plain";
    if (!Q_strcmp(ext, "deploy"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "der"))
        return "application/x-x509-ca-cert";
    if (!Q_strcmp(ext, "dgml"))
        return "application/xml";
    if (!Q_strcmp(ext, "dib"))
        return "image/bmp";
    if (!Q_strcmp(ext, "dif"))
        return "video/x-dv";
    if (!Q_strcmp(ext, "dir"))
        return "application/x-director";
    if (!Q_strcmp(ext, "disco"))
        return "text/xml";
    if (!Q_strcmp(ext, "divx"))
        return "video/divx";
    if (!Q_strcmp(ext, "dll"))
        return "application/x-msdownload";
    if (!Q_strcmp(ext, "dll.config"))
        return "text/xml";
    if (!Q_strcmp(ext, "dlm"))
        return "text/dlm";
    if (!Q_strcmp(ext, "doc"))
        return "application/msword";
    if (!Q_strcmp(ext, "docm"))
        return "application/vnd.ms-word.document.macroEnabled.12";
    if (!Q_strcmp(ext, "docx"))
        return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (!Q_strcmp(ext, "dot"))
        return "application/msword";
    if (!Q_strcmp(ext, "dotm"))
        return "application/vnd.ms-word.template.macroEnabled.12";
    if (!Q_strcmp(ext, "dotx"))
        return "application/vnd.openxmlformats-officedocument.wordprocessingml.template";
    if (!Q_strcmp(ext, "dsp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "dsw"))
        return "text/plain";
    if (!Q_strcmp(ext, "dtd"))
        return "text/xml";
    if (!Q_strcmp(ext, "dtsConfig"))
        return "text/xml";
    if (!Q_strcmp(ext, "dv"))
        return "video/x-dv";
    if (!Q_strcmp(ext, "dvi"))
        return "application/x-dvi";
    if (!Q_strcmp(ext, "dwf"))
        return "drawing/x-dwf";
    if (!Q_strcmp(ext, "dwg"))
        return "application/acad";
    if (!Q_strcmp(ext, "dwp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "dxf"))
        return "application/x-dxf";
    if (!Q_strcmp(ext, "dxr"))
        return "application/x-director";
    if (!Q_strcmp(ext, "eml"))
        return "message/rfc822";
    if (!Q_strcmp(ext, "emz"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "eot"))
        return "application/vnd.ms-fontobject";
    if (!Q_strcmp(ext, "eps"))
        return "application/postscript";
    if (!Q_strcmp(ext, "etl"))
        return "application/etl";
    if (!Q_strcmp(ext, "etx"))
        return "text/x-setext";
    if (!Q_strcmp(ext, "evy"))
        return "application/envoy";
    if (!Q_strcmp(ext, "exe"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "exe.config"))
        return "text/xml";
    if (!Q_strcmp(ext, "fdf"))
        return "application/vnd.fdf";
    if (!Q_strcmp(ext, "fif"))
        return "application/fractals";
    if (!Q_strcmp(ext, "filters"))
        return "application/xml";
    if (!Q_strcmp(ext, "fla"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "flac"))
        return "audio/flac";
    if (!Q_strcmp(ext, "flr"))
        return "x-world/x-vrml";
    if (!Q_strcmp(ext, "flv"))
        return "video/x-flv";
    if (!Q_strcmp(ext, "fsscript"))
        return "application/fsharp-script";
    if (!Q_strcmp(ext, "fsx"))
        return "application/fsharp-script";
    if (!Q_strcmp(ext, "generictest"))
        return "application/xml";
    if (!Q_strcmp(ext, "gif"))
        return "image/gif";
    if (!Q_strcmp(ext, "gpx"))
        return "application/gpx+xml";
    if (!Q_strcmp(ext, "group"))
        return "text/x-ms-group";
    if (!Q_strcmp(ext, "gsm"))
        return "audio/x-gsm";
    if (!Q_strcmp(ext, "gtar"))
        return "application/x-gtar";
    if (!Q_strcmp(ext, "gz"))
        return "application/x-gzip";
    if (!Q_strcmp(ext, "h"))
        return "text/plain";
    if (!Q_strcmp(ext, "hdf"))
        return "application/x-hdf";
    if (!Q_strcmp(ext, "hdml"))
        return "text/x-hdml";
    if (!Q_strcmp(ext, "hhc"))
        return "application/x-oleobject";
    if (!Q_strcmp(ext, "hhk"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hhp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hlp"))
        return "application/winhlp";
    if (!Q_strcmp(ext, "hpp"))
        return "text/plain";
    if (!Q_strcmp(ext, "hqx"))
        return "application/mac-binhex40";
    if (!Q_strcmp(ext, "hta"))
        return "application/hta";
    if (!Q_strcmp(ext, "htc"))
        return "text/x-component";
    if (!Q_strcmp(ext, "htm"))
        return "text/html";
    if (!Q_strcmp(ext, "html"))
        return "text/html";
    if (!Q_strcmp(ext, "htt"))
        return "text/webviewhtml";
    if (!Q_strcmp(ext, "hxa"))
        return "application/xml";
    if (!Q_strcmp(ext, "hxc"))
        return "application/xml";
    if (!Q_strcmp(ext, "hxd"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxe"))
        return "application/xml";
    if (!Q_strcmp(ext, "hxf"))
        return "application/xml";
    if (!Q_strcmp(ext, "hxh"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxi"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxk"))
        return "application/xml";
    if (!Q_strcmp(ext, "hxq"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxr"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxs"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxt"))
        return "text/html";
    if (!Q_strcmp(ext, "hxv"))
        return "application/xml";
    if (!Q_strcmp(ext, "hxw"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "hxx"))
        return "text/plain";
    if (!Q_strcmp(ext, "i"))
        return "text/plain";
    if (!Q_strcmp(ext, "ico"))
        return "image/x-icon";
    if (!Q_strcmp(ext, "ics"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "idl"))
        return "text/plain";
    if (!Q_strcmp(ext, "ief"))
        return "image/ief";
    if (!Q_strcmp(ext, "iii"))
        return "application/x-iphone";
    if (!Q_strcmp(ext, "inc"))
        return "text/plain";
    if (!Q_strcmp(ext, "inf"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "ini"))
        return "text/plain";
    if (!Q_strcmp(ext, "inl"))
        return "text/plain";
    if (!Q_strcmp(ext, "ins"))
        return "application/x-internet-signup";
    if (!Q_strcmp(ext, "ipa"))
        return "application/x-itunes-ipa";
    if (!Q_strcmp(ext, "ipg"))
        return "application/x-itunes-ipg";
    if (!Q_strcmp(ext, "ipproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "ipsw"))
        return "application/x-itunes-ipsw";
    if (!Q_strcmp(ext, "iqy"))
        return "text/x-ms-iqy";
    if (!Q_strcmp(ext, "isp"))
        return "application/x-internet-signup";
    if (!Q_strcmp(ext, "ite"))
        return "application/x-itunes-ite";
    if (!Q_strcmp(ext, "itlp"))
        return "application/x-itunes-itlp";
    if (!Q_strcmp(ext, "itms"))
        return "application/x-itunes-itms";
    if (!Q_strcmp(ext, "itpc"))
        return "application/x-itunes-itpc";
    if (!Q_strcmp(ext, "IVF"))
        return "video/x-ivf";
    if (!Q_strcmp(ext, "jar"))
        return "application/java-archive";
    if (!Q_strcmp(ext, "java"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "jck"))
        return "application/liquidmotion";
    if (!Q_strcmp(ext, "jcz"))
        return "application/liquidmotion";
    if (!Q_strcmp(ext, "jfif"))
        return "image/pjpeg";
    if (!Q_strcmp(ext, "jnlp"))
        return "application/x-java-jnlp-file";
    if (!Q_strcmp(ext, "jpb"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "jpe"))
        return "image/jpeg";
    if (!Q_strcmp(ext, "jpeg"))
        return "image/jpeg";
    if (!Q_strcmp(ext, "jpg"))
        return "image/jpeg";
    if (!Q_strcmp(ext, "js"))
        return "application/javascript";
    if (!Q_strcmp(ext, "json"))
        return "application/json";
    if (!Q_strcmp(ext, "jsx"))
        return "text/jscript";
    if (!Q_strcmp(ext, "jsxbin"))
        return "text/plain";
    if (!Q_strcmp(ext, "latex"))
        return "application/x-latex";
    if (!Q_strcmp(ext, "library-ms"))
        return "application/windows-library+xml";
    if (!Q_strcmp(ext, "lit"))
        return "application/x-ms-reader";
    if (!Q_strcmp(ext, "loadtest"))
        return "application/xml";
    if (!Q_strcmp(ext, "lpk"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "lsf"))
        return "video/x-la-asf";
    if (!Q_strcmp(ext, "lst"))
        return "text/plain";
    if (!Q_strcmp(ext, "lsx"))
        return "video/x-la-asf";
    if (!Q_strcmp(ext, "lzh"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "m13"))
        return "application/x-msmediaview";
    if (!Q_strcmp(ext, "m14"))
        return "application/x-msmediaview";
    if (!Q_strcmp(ext, "m1v"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "m2t"))
        return "video/vnd.dlna.mpeg-tts";
    if (!Q_strcmp(ext, "m2ts"))
        return "video/vnd.dlna.mpeg-tts";
    if (!Q_strcmp(ext, "m2v"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "m3u"))
        return "audio/x-mpegurl";
    if (!Q_strcmp(ext, "m3u8"))
        return "audio/x-mpegurl";
    if (!Q_strcmp(ext, "m4a"))
        return "audio/m4a";
    if (!Q_strcmp(ext, "m4b"))
        return "audio/m4b";
    if (!Q_strcmp(ext, "m4p"))
        return "audio/m4p";
    if (!Q_strcmp(ext, "m4r"))
        return "audio/x-m4r";
    if (!Q_strcmp(ext, "m4v"))
        return "video/x-m4v";
    if (!Q_strcmp(ext, "mac"))
        return "image/x-macpaint";
    if (!Q_strcmp(ext, "mak"))
        return "text/plain";
    if (!Q_strcmp(ext, "man"))
        return "application/x-troff-man";
    if (!Q_strcmp(ext, "manifest"))
        return "application/x-ms-manifest";
    if (!Q_strcmp(ext, "map"))
        return "text/plain";
    if (!Q_strcmp(ext, "master"))
        return "application/xml";
    if (!Q_strcmp(ext, "mbox"))
        return "application/mbox";
    if (!Q_strcmp(ext, "mda"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "mdb"))
        return "application/x-msaccess";
    if (!Q_strcmp(ext, "mde"))
        return "application/msaccess";
    if (!Q_strcmp(ext, "mdp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "me"))
        return "application/x-troff-me";
    if (!Q_strcmp(ext, "mfp"))
        return "application/x-shockwave-flash";
    if (!Q_strcmp(ext, "mht"))
        return "message/rfc822";
    if (!Q_strcmp(ext, "mhtml"))
        return "message/rfc822";
    if (!Q_strcmp(ext, "mid"))
        return "audio/mid";
    if (!Q_strcmp(ext, "midi"))
        return "audio/mid";
    if (!Q_strcmp(ext, "mix"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "mk"))
        return "text/plain";
    if (!Q_strcmp(ext, "mk3d"))
        return "video/x-matroska-3d";
    if (!Q_strcmp(ext, "mka"))
        return "audio/x-matroska";
    if (!Q_strcmp(ext, "mkv"))
        return "video/x-matroska";
    if (!Q_strcmp(ext, "mmf"))
        return "application/x-smaf";
    if (!Q_strcmp(ext, "mno"))
        return "text/xml";
    if (!Q_strcmp(ext, "mny"))
        return "application/x-msmoney";
    if (!Q_strcmp(ext, "mod"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mov"))
        return "video/quicktime";
    if (!Q_strcmp(ext, "movie"))
        return "video/x-sgi-movie";
    if (!Q_strcmp(ext, "mp2"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mp2v"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mp3"))
        return "audio/mpeg";
    if (!Q_strcmp(ext, "mp4"))
        return "video/mp4";
    if (!Q_strcmp(ext, "mp4v"))
        return "video/mp4";
    if (!Q_strcmp(ext, "mpa"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mpe"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mpeg"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mpf"))
        return "application/vnd.ms-mediapackage";
    if (!Q_strcmp(ext, "mpg"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mpp"))
        return "application/vnd.ms-project";
    if (!Q_strcmp(ext, "mpv2"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "mqv"))
        return "video/quicktime";
    if (!Q_strcmp(ext, "ms"))
        return "application/x-troff-ms";
    if (!Q_strcmp(ext, "msg"))
        return "application/vnd.ms-outlook";
    if (!Q_strcmp(ext, "msi"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "mso"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "mts"))
        return "video/vnd.dlna.mpeg-tts";
    if (!Q_strcmp(ext, "mtx"))
        return "application/xml";
    if (!Q_strcmp(ext, "mvb"))
        return "application/x-msmediaview";
    if (!Q_strcmp(ext, "mvc"))
        return "application/x-miva-compiled";
    if (!Q_strcmp(ext, "mxp"))
        return "application/x-mmxp";
    if (!Q_strcmp(ext, "nc"))
        return "application/x-netcdf";
    if (!Q_strcmp(ext, "nsc"))
        return "video/x-ms-asf";
    if (!Q_strcmp(ext, "nws"))
        return "message/rfc822";
    if (!Q_strcmp(ext, "ocx"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "oda"))
        return "application/oda";
    if (!Q_strcmp(ext, "odb"))
        return "application/vnd.oasis.opendocument.database";
    if (!Q_strcmp(ext, "odc"))
        return "application/vnd.oasis.opendocument.chart";
    if (!Q_strcmp(ext, "odf"))
        return "application/vnd.oasis.opendocument.formula";
    if (!Q_strcmp(ext, "odg"))
        return "application/vnd.oasis.opendocument.graphics";
    if (!Q_strcmp(ext, "odh"))
        return "text/plain";
    if (!Q_strcmp(ext, "odi"))
        return "application/vnd.oasis.opendocument.image";
    if (!Q_strcmp(ext, "odl"))
        return "text/plain";
    if (!Q_strcmp(ext, "odm"))
        return "application/vnd.oasis.opendocument.text-master";
    if (!Q_strcmp(ext, "odp"))
        return "application/vnd.oasis.opendocument.presentation";
    if (!Q_strcmp(ext, "ods"))
        return "application/vnd.oasis.opendocument.spreadsheet";
    if (!Q_strcmp(ext, "odt"))
        return "application/vnd.oasis.opendocument.text";
    if (!Q_strcmp(ext, "oga"))
        return "audio/ogg";
    if (!Q_strcmp(ext, "ogg"))
        return "audio/ogg";
    if (!Q_strcmp(ext, "ogv"))
        return "video/ogg";
    if (!Q_strcmp(ext, "ogx"))
        return "application/ogg";
    if (!Q_strcmp(ext, "one"))
        return "application/onenote";
    if (!Q_strcmp(ext, "onea"))
        return "application/onenote";
    if (!Q_strcmp(ext, "onepkg"))
        return "application/onenote";
    if (!Q_strcmp(ext, "onetmp"))
        return "application/onenote";
    if (!Q_strcmp(ext, "onetoc"))
        return "application/onenote";
    if (!Q_strcmp(ext, "onetoc2"))
        return "application/onenote";
    if (!Q_strcmp(ext, "opus"))
        return "audio/ogg";
    if (!Q_strcmp(ext, "orderedtest"))
        return "application/xml";
    if (!Q_strcmp(ext, "osdx"))
        return "application/opensearchdescription+xml";
    if (!Q_strcmp(ext, "otf"))
        return "application/font-sfnt";
    if (!Q_strcmp(ext, "otg"))
        return "application/vnd.oasis.opendocument.graphics-template";
    if (!Q_strcmp(ext, "oth"))
        return "application/vnd.oasis.opendocument.text-web";
    if (!Q_strcmp(ext, "otp"))
        return "application/vnd.oasis.opendocument.presentation-template";
    if (!Q_strcmp(ext, "ots"))
        return "application/vnd.oasis.opendocument.spreadsheet-template";
    if (!Q_strcmp(ext, "ott"))
        return "application/vnd.oasis.opendocument.text-template";
    if (!Q_strcmp(ext, "oxt"))
        return "application/vnd.openofficeorg.extension";
    if (!Q_strcmp(ext, "p10"))
        return "application/pkcs10";
    if (!Q_strcmp(ext, "p12"))
        return "application/x-pkcs12";
    if (!Q_strcmp(ext, "p7b"))
        return "application/x-pkcs7-certificates";
    if (!Q_strcmp(ext, "p7c"))
        return "application/pkcs7-mime";
    if (!Q_strcmp(ext, "p7m"))
        return "application/pkcs7-mime";
    if (!Q_strcmp(ext, "p7r"))
        return "application/x-pkcs7-certreqresp";
    if (!Q_strcmp(ext, "p7s"))
        return "application/pkcs7-signature";
    if (!Q_strcmp(ext, "pbm"))
        return "image/x-portable-bitmap";
    if (!Q_strcmp(ext, "pcast"))
        return "application/x-podcast";
    if (!Q_strcmp(ext, "pct"))
        return "image/pict";
    if (!Q_strcmp(ext, "pcx"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "pcz"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "pdf"))
        return "application/pdf";
    if (!Q_strcmp(ext, "pfb"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "pfm"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "pfx"))
        return "application/x-pkcs12";
    if (!Q_strcmp(ext, "pgm"))
        return "image/x-portable-graymap";
    if (!Q_strcmp(ext, "pic"))
        return "image/pict";
    if (!Q_strcmp(ext, "pict"))
        return "image/pict";
    if (!Q_strcmp(ext, "pkgdef"))
        return "text/plain";
    if (!Q_strcmp(ext, "pkgundef"))
        return "text/plain";
    if (!Q_strcmp(ext, "pko"))
        return "application/vnd.ms-pki.pko";
    if (!Q_strcmp(ext, "pls"))
        return "audio/scpls";
    if (!Q_strcmp(ext, "pma"))
        return "application/x-perfmon";
    if (!Q_strcmp(ext, "pmc"))
        return "application/x-perfmon";
    if (!Q_strcmp(ext, "pml"))
        return "application/x-perfmon";
    if (!Q_strcmp(ext, "pmr"))
        return "application/x-perfmon";
    if (!Q_strcmp(ext, "pmw"))
        return "application/x-perfmon";
    if (!Q_strcmp(ext, "png"))
        return "image/png";
    if (!Q_strcmp(ext, "pnm"))
        return "image/x-portable-anymap";
    if (!Q_strcmp(ext, "pnt"))
        return "image/x-macpaint";
    if (!Q_strcmp(ext, "pntg"))
        return "image/x-macpaint";
    if (!Q_strcmp(ext, "pnz"))
        return "image/png";
    if (!Q_strcmp(ext, "pot"))
        return "application/vnd.ms-powerpoint";
    if (!Q_strcmp(ext, "potm"))
        return "application/vnd.ms-powerpoint.template.macroEnabled.12";
    if (!Q_strcmp(ext, "potx"))
        return "application/vnd.openxmlformats-officedocument.presentationml.template";
    if (!Q_strcmp(ext, "ppa"))
        return "application/vnd.ms-powerpoint";
    if (!Q_strcmp(ext, "ppam"))
        return "application/vnd.ms-powerpoint.addin.macroEnabled.12";
    if (!Q_strcmp(ext, "ppm"))
        return "image/x-portable-pixmap";
    if (!Q_strcmp(ext, "pps"))
        return "application/vnd.ms-powerpoint";
    if (!Q_strcmp(ext, "ppsm"))
        return "application/vnd.ms-powerpoint.slideshow.macroEnabled.12";
    if (!Q_strcmp(ext, "ppsx"))
        return "application/vnd.openxmlformats-officedocument.presentationml.slideshow";
    if (!Q_strcmp(ext, "ppt"))
        return "application/vnd.ms-powerpoint";
    if (!Q_strcmp(ext, "pptm"))
        return "application/vnd.ms-powerpoint.presentation.macroEnabled.12";
    if (!Q_strcmp(ext, "pptx"))
        return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    if (!Q_strcmp(ext, "prf"))
        return "application/pics-rules";
    if (!Q_strcmp(ext, "prm"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "prx"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "ps"))
        return "application/postscript";
    if (!Q_strcmp(ext, "psc1"))
        return "application/PowerShell";
    if (!Q_strcmp(ext, "psd"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "psess"))
        return "application/xml";
    if (!Q_strcmp(ext, "psm"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "psp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "pst"))
        return "application/vnd.ms-outlook";
    if (!Q_strcmp(ext, "pub"))
        return "application/x-mspublisher";
    if (!Q_strcmp(ext, "pwz"))
        return "application/vnd.ms-powerpoint";
    if (!Q_strcmp(ext, "qht"))
        return "text/x-html-insertion";
    if (!Q_strcmp(ext, "qhtm"))
        return "text/x-html-insertion";
    if (!Q_strcmp(ext, "qt"))
        return "video/quicktime";
    if (!Q_strcmp(ext, "qti"))
        return "image/x-quicktime";
    if (!Q_strcmp(ext, "qtif"))
        return "image/x-quicktime";
    if (!Q_strcmp(ext, "qtl"))
        return "application/x-quicktimeplayer";
    if (!Q_strcmp(ext, "qxd"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "ra"))
        return "audio/x-pn-realaudio";
    if (!Q_strcmp(ext, "ram"))
        return "audio/x-pn-realaudio";
    if (!Q_strcmp(ext, "rar"))
        return "application/x-rar-compressed";
    if (!Q_strcmp(ext, "ras"))
        return "image/x-cmu-raster";
    if (!Q_strcmp(ext, "rat"))
        return "application/rat-file";
    if (!Q_strcmp(ext, "rc"))
        return "text/plain";
    if (!Q_strcmp(ext, "rc2"))
        return "text/plain";
    if (!Q_strcmp(ext, "rct"))
        return "text/plain";
    if (!Q_strcmp(ext, "rdlc"))
        return "application/xml";
    if (!Q_strcmp(ext, "reg"))
        return "text/plain";
    if (!Q_strcmp(ext, "resx"))
        return "application/xml";
    if (!Q_strcmp(ext, "rf"))
        return "image/vnd.rn-realflash";
    if (!Q_strcmp(ext, "rgb"))
        return "image/x-rgb";
    if (!Q_strcmp(ext, "rgs"))
        return "text/plain";
    if (!Q_strcmp(ext, "rm"))
        return "application/vnd.rn-realmedia";
    if (!Q_strcmp(ext, "rmi"))
        return "audio/mid";
    if (!Q_strcmp(ext, "rmp"))
        return "application/vnd.rn-rn_music_package";
    if (!Q_strcmp(ext, "roff"))
        return "application/x-troff";
    if (!Q_strcmp(ext, "rpm"))
        return "audio/x-pn-realaudio-plugin";
    if (!Q_strcmp(ext, "rqy"))
        return "text/x-ms-rqy";
    if (!Q_strcmp(ext, "rtf"))
        return "application/rtf";
    if (!Q_strcmp(ext, "rtx"))
        return "text/richtext";
    if (!Q_strcmp(ext, "rvt"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "ruleset"))
        return "application/xml";
    if (!Q_strcmp(ext, "s"))
        return "text/plain";
    if (!Q_strcmp(ext, "safariextz"))
        return "application/x-safari-safariextz";
    if (!Q_strcmp(ext, "scd"))
        return "application/x-msschedule";
    if (!Q_strcmp(ext, "scr"))
        return "text/plain";
    if (!Q_strcmp(ext, "sct"))
        return "text/scriptlet";
    if (!Q_strcmp(ext, "sd2"))
        return "audio/x-sd2";
    if (!Q_strcmp(ext, "sdp"))
        return "application/sdp";
    if (!Q_strcmp(ext, "sea"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "searchConnector-ms"))
        return "application/windows-search-connector+xml";
    if (!Q_strcmp(ext, "setpay"))
        return "application/set-payment-initiation";
    if (!Q_strcmp(ext, "setreg"))
        return "application/set-registration-initiation";
    if (!Q_strcmp(ext, "settings"))
        return "application/xml";
    if (!Q_strcmp(ext, "sgimb"))
        return "application/x-sgimb";
    if (!Q_strcmp(ext, "sgml"))
        return "text/sgml";
    if (!Q_strcmp(ext, "sh"))
        return "application/x-sh";
    if (!Q_strcmp(ext, "shar"))
        return "application/x-shar";
    if (!Q_strcmp(ext, "shtml"))
        return "text/html";
    if (!Q_strcmp(ext, "sit"))
        return "application/x-stuffit";
    if (!Q_strcmp(ext, "sitemap"))
        return "application/xml";
    if (!Q_strcmp(ext, "skin"))
        return "application/xml";
    if (!Q_strcmp(ext, "skp"))
        return "application/x-koan";
    if (!Q_strcmp(ext, "sldm"))
        return "application/vnd.ms-powerpoint.slide.macroEnabled.12";
    if (!Q_strcmp(ext, "sldx"))
        return "application/vnd.openxmlformats-officedocument.presentationml.slide";
    if (!Q_strcmp(ext, "slk"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "sln"))
        return "text/plain";
    if (!Q_strcmp(ext, "slupkg-ms"))
        return "application/x-ms-license";
    if (!Q_strcmp(ext, "smd"))
        return "audio/x-smd";
    if (!Q_strcmp(ext, "smi"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "smx"))
        return "audio/x-smd";
    if (!Q_strcmp(ext, "smz"))
        return "audio/x-smd";
    if (!Q_strcmp(ext, "snd"))
        return "audio/basic";
    if (!Q_strcmp(ext, "snippet"))
        return "application/xml";
    if (!Q_strcmp(ext, "snp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "sol"))
        return "text/plain";
    if (!Q_strcmp(ext, "sor"))
        return "text/plain";
    if (!Q_strcmp(ext, "spc"))
        return "application/x-pkcs7-certificates";
    if (!Q_strcmp(ext, "spl"))
        return "application/futuresplash";
    if (!Q_strcmp(ext, "spx"))
        return "audio/ogg";
    if (!Q_strcmp(ext, "src"))
        return "application/x-wais-source";
    if (!Q_strcmp(ext, "srf"))
        return "text/plain";
    if (!Q_strcmp(ext, "SSISDeploymentManifest"))
        return "text/xml";
    if (!Q_strcmp(ext, "ssm"))
        return "application/streamingmedia";
    if (!Q_strcmp(ext, "sst"))
        return "application/vnd.ms-pki.certstore";
    if (!Q_strcmp(ext, "stl"))
        return "application/vnd.ms-pki.stl";
    if (!Q_strcmp(ext, "sv4cpio"))
        return "application/x-sv4cpio";
    if (!Q_strcmp(ext, "sv4crc"))
        return "application/x-sv4crc";
    if (!Q_strcmp(ext, "svc"))
        return "application/xml";
    if (!Q_strcmp(ext, "svg"))
        return "image/svg+xml";
    if (!Q_strcmp(ext, "swf"))
        return "application/x-shockwave-flash";
    if (!Q_strcmp(ext, "step"))
        return "application/step";
    if (!Q_strcmp(ext, "stp"))
        return "application/step";
    if (!Q_strcmp(ext, "t"))
        return "application/x-troff";
    if (!Q_strcmp(ext, "tar"))
        return "application/x-tar";
    if (!Q_strcmp(ext, "tcl"))
        return "application/x-tcl";
    if (!Q_strcmp(ext, "testrunconfig"))
        return "application/xml";
    if (!Q_strcmp(ext, "testsettings"))
        return "application/xml";
    if (!Q_strcmp(ext, "tex"))
        return "application/x-tex";
    if (!Q_strcmp(ext, "texi"))
        return "application/x-texinfo";
    if (!Q_strcmp(ext, "texinfo"))
        return "application/x-texinfo";
    if (!Q_strcmp(ext, "tgz"))
        return "application/x-compressed";
    if (!Q_strcmp(ext, "thmx"))
        return "application/vnd.ms-officetheme";
    if (!Q_strcmp(ext, "thn"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "tif"))
        return "image/tiff";
    if (!Q_strcmp(ext, "tiff"))
        return "image/tiff";
    if (!Q_strcmp(ext, "tlh"))
        return "text/plain";
    if (!Q_strcmp(ext, "tli"))
        return "text/plain";
    if (!Q_strcmp(ext, "toc"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "tr"))
        return "application/x-troff";
    if (!Q_strcmp(ext, "trm"))
        return "application/x-msterminal";
    if (!Q_strcmp(ext, "trx"))
        return "application/xml";
    if (!Q_strcmp(ext, "ts"))
        return "video/vnd.dlna.mpeg-tts";
    if (!Q_strcmp(ext, "tsv"))
        return "text/tab-separated-values";
    if (!Q_strcmp(ext, "ttf"))
        return "application/font-sfnt";
    if (!Q_strcmp(ext, "tts"))
        return "video/vnd.dlna.mpeg-tts";
    if (!Q_strcmp(ext, "txt"))
        return "text/plain";
    if (!Q_strcmp(ext, "u32"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "uls"))
        return "text/iuls";
    if (!Q_strcmp(ext, "user"))
        return "text/plain";
    if (!Q_strcmp(ext, "ustar"))
        return "application/x-ustar";
    if (!Q_strcmp(ext, "vb"))
        return "text/plain";
    if (!Q_strcmp(ext, "vbdproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "vbk"))
        return "video/mpeg";
    if (!Q_strcmp(ext, "vbproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "vbs"))
        return "text/vbscript";
    if (!Q_strcmp(ext, "vcf"))
        return "text/x-vcard";
    if (!Q_strcmp(ext, "vcproj"))
        return "application/xml";
    if (!Q_strcmp(ext, "vcs"))
        return "text/plain";
    if (!Q_strcmp(ext, "vcxproj"))
        return "application/xml";
    if (!Q_strcmp(ext, "vddproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "vdp"))
        return "text/plain";
    if (!Q_strcmp(ext, "vdproj"))
        return "text/plain";
    if (!Q_strcmp(ext, "vdx"))
        return "application/vnd.ms-visio.viewer";
    if (!Q_strcmp(ext, "vml"))
        return "text/xml";
    if (!Q_strcmp(ext, "vscontent"))
        return "application/xml";
    if (!Q_strcmp(ext, "vsct"))
        return "text/xml";
    if (!Q_strcmp(ext, "vsd"))
        return "application/vnd.visio";
    if (!Q_strcmp(ext, "vsi"))
        return "application/ms-vsi";
    if (!Q_strcmp(ext, "vsix"))
        return "application/vsix";
    if (!Q_strcmp(ext, "vsixlangpack"))
        return "text/xml";
    if (!Q_strcmp(ext, "vsixmanifest"))
        return "text/xml";
    if (!Q_strcmp(ext, "vsmdi"))
        return "application/xml";
    if (!Q_strcmp(ext, "vspscc"))
        return "text/plain";
    if (!Q_strcmp(ext, "vss"))
        return "application/vnd.visio";
    if (!Q_strcmp(ext, "vsscc"))
        return "text/plain";
    if (!Q_strcmp(ext, "vssettings"))
        return "text/xml";
    if (!Q_strcmp(ext, "vssscc"))
        return "text/plain";
    if (!Q_strcmp(ext, "vst"))
        return "application/vnd.visio";
    if (!Q_strcmp(ext, "vstemplate"))
        return "text/xml";
    if (!Q_strcmp(ext, "vsto"))
        return "application/x-ms-vsto";
    if (!Q_strcmp(ext, "vsw"))
        return "application/vnd.visio";
    if (!Q_strcmp(ext, "vsx"))
        return "application/vnd.visio";
    if (!Q_strcmp(ext, "vtt"))
        return "text/vtt";
    if (!Q_strcmp(ext, "vtx"))
        return "application/vnd.visio";
    if (!Q_strcmp(ext, "wasm"))
        return "application/wasm";
    if (!Q_strcmp(ext, "wav"))
        return "audio/wav";
    if (!Q_strcmp(ext, "wave"))
        return "audio/wav";
    if (!Q_strcmp(ext, "wax"))
        return "audio/x-ms-wax";
    if (!Q_strcmp(ext, "wbk"))
        return "application/msword";
    if (!Q_strcmp(ext, "wbmp"))
        return "image/vnd.wap.wbmp";
    if (!Q_strcmp(ext, "wcm"))
        return "application/vnd.ms-works";
    if (!Q_strcmp(ext, "wdb"))
        return "application/vnd.ms-works";
    if (!Q_strcmp(ext, "wdp"))
        return "image/vnd.ms-photo";
    if (!Q_strcmp(ext, "webarchive"))
        return "application/x-safari-webarchive";
    if (!Q_strcmp(ext, "webm"))
        return "video/webm";
    if (!Q_strcmp(ext, "webp"))
        return "image/webp"; /* https://en.wikipedia.org/wiki/WebP */
    if (!Q_strcmp(ext, "webtest"))
        return "application/xml";
    if (!Q_strcmp(ext, "wiq"))
        return "application/xml";
    if (!Q_strcmp(ext, "wiz"))
        return "application/msword";
    if (!Q_strcmp(ext, "wks"))
        return "application/vnd.ms-works";
    if (!Q_strcmp(ext, "WLMP"))
        return "application/wlmoviemaker";
    if (!Q_strcmp(ext, "wlpginstall"))
        return "application/x-wlpg-detect";
    if (!Q_strcmp(ext, "wlpginstall3"))
        return "application/x-wlpg3-detect";
    if (!Q_strcmp(ext, "wm"))
        return "video/x-ms-wm";
    if (!Q_strcmp(ext, "wma"))
        return "audio/x-ms-wma";
    if (!Q_strcmp(ext, "wmd"))
        return "application/x-ms-wmd";
    if (!Q_strcmp(ext, "wmf"))
        return "application/x-msmetafile";
    if (!Q_strcmp(ext, "wml"))
        return "text/vnd.wap.wml";
    if (!Q_strcmp(ext, "wmlc"))
        return "application/vnd.wap.wmlc";
    if (!Q_strcmp(ext, "wmls"))
        return "text/vnd.wap.wmlscript";
    if (!Q_strcmp(ext, "wmlsc"))
        return "application/vnd.wap.wmlscriptc";
    if (!Q_strcmp(ext, "wmp"))
        return "video/x-ms-wmp";
    if (!Q_strcmp(ext, "wmv"))
        return "video/x-ms-wmv";
    if (!Q_strcmp(ext, "wmx"))
        return "video/x-ms-wmx";
    if (!Q_strcmp(ext, "wmz"))
        return "application/x-ms-wmz";
    if (!Q_strcmp(ext, "woff"))
        return "application/font-woff";
    if (!Q_strcmp(ext, "woff2"))
        return "application/font-woff2";
    if (!Q_strcmp(ext, "wpl"))
        return "application/vnd.ms-wpl";
    if (!Q_strcmp(ext, "wps"))
        return "application/vnd.ms-works";
    if (!Q_strcmp(ext, "wri"))
        return "application/x-mswrite";
    if (!Q_strcmp(ext, "wrl"))
        return "x-world/x-vrml";
    if (!Q_strcmp(ext, "wrz"))
        return "x-world/x-vrml";
    if (!Q_strcmp(ext, "wsc"))
        return "text/scriptlet";
    if (!Q_strcmp(ext, "wsdl"))
        return "text/xml";
    if (!Q_strcmp(ext, "wvx"))
        return "video/x-ms-wvx";
    if (!Q_strcmp(ext, "x"))
        return "application/directx";
    if (!Q_strcmp(ext, "xaf"))
        return "x-world/x-vrml";
    if (!Q_strcmp(ext, "xaml"))
        return "application/xaml+xml";
    if (!Q_strcmp(ext, "xap"))
        return "application/x-silverlight-app";
    if (!Q_strcmp(ext, "xbap"))
        return "application/x-ms-xbap";
    if (!Q_strcmp(ext, "xbm"))
        return "image/x-xbitmap";
    if (!Q_strcmp(ext, "xdr"))
        return "text/plain";
    if (!Q_strcmp(ext, "xht"))
        return "application/xhtml+xml";
    if (!Q_strcmp(ext, "xhtml"))
        return "application/xhtml+xml";
    if (!Q_strcmp(ext, "xla"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xlam"))
        return "application/vnd.ms-excel.addin.macroEnabled.12";
    if (!Q_strcmp(ext, "xlc"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xld"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xlk"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xll"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xlm"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xls"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xlsb"))
        return "application/vnd.ms-excel.sheet.binary.macroEnabled.12";
    if (!Q_strcmp(ext, "xlsm"))
        return "application/vnd.ms-excel.sheet.macroEnabled.12";
    if (!Q_strcmp(ext, "xlsx"))
        return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    if (!Q_strcmp(ext, "xlt"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xltm"))
        return "application/vnd.ms-excel.template.macroEnabled.12";
    if (!Q_strcmp(ext, "xltx"))
        return "application/vnd.openxmlformats-officedocument.spreadsheetml.template";
    if (!Q_strcmp(ext, "xlw"))
        return "application/vnd.ms-excel";
    if (!Q_strcmp(ext, "xml"))
        return "text/xml";
    if (!Q_strcmp(ext, "xmp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "xmta"))
        return "application/xml";
    if (!Q_strcmp(ext, "xof"))
        return "x-world/x-vrml";
    if (!Q_strcmp(ext, "XOML"))
        return "text/plain";
    if (!Q_strcmp(ext, "xpm"))
        return "image/x-xpixmap";
    if (!Q_strcmp(ext, "xps"))
        return "application/vnd.ms-xpsdocument";
    if (!Q_strcmp(ext, "xrm-ms"))
        return "text/xml";
    if (!Q_strcmp(ext, "xsc"))
        return "application/xml";
    if (!Q_strcmp(ext, "xsd"))
        return "text/xml";
    if (!Q_strcmp(ext, "xsf"))
        return "text/xml";
    if (!Q_strcmp(ext, "xsl"))
        return "text/xml";
    if (!Q_strcmp(ext, "xslt"))
        return "text/xml";
    if (!Q_strcmp(ext, "xsn"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "xss"))
        return "application/xml";
    if (!Q_strcmp(ext, "xspf"))
        return "application/xspf+xml";
    if (!Q_strcmp(ext, "xtp"))
        return "application/octet-stream";
    if (!Q_strcmp(ext, "xwd"))
        return "image/x-xwindowdump";
    if (!Q_strcmp(ext, "z"))
        return "application/x-compress";
    if (!Q_strcmp(ext, "zip"))
        return "application/zip";

    return "application/octet-stream";
}