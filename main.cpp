#include <fstream>
#include <vector>
#include <string>
#include <codecvt>

#include <Windows.h>

bool unmountDrive(std::string path)
{
    // First get a lock on the device.
    // This needs to be done as low-level C, and does not come in C++
    
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(path);

    HANDLE volume = CreateFile(wide.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (volume == INVALID_HANDLE_VALUE)
        return false;

    // Get a lock on the volume
    DWORD bytesreturned;
    auto result = DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesreturned, NULL);

    if (!result)
        return false;

    // Unmount volume
    DWORD junk;
    result = DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
    if (!result)
    {
        return false;
    }

    return true;
}


int main(int argc, char *argv[])
{
    if ( argc < 3 )
    {
        return 1; //not enough arguments
    }

    if (argc == 4)
    {
        std::string path(argv[3]);
        path = "\\\\.\\" + path + ":";
        if (!unmountDrive(path))
            return 4; // could not unmount
    }

    std::string drive = std::string(argv[2]);
    std::string imageFile = std::string(argv[1]);

    std::ifstream in_size(imageFile, std::ifstream::ate | std::ifstream::binary);
    auto size = in_size.tellg();
    in_size.close();

    std::ifstream in(imageFile, std::ifstream::in | std::ifstream::binary);
    std::ofstream out(drive, std::ofstream::in | std::ofstream::out | std::ofstream::binary);

    std::vector<char> buffer(1024 * 512, 0);

    int64_t accumulated = 0;
    int64_t totalSize = size;
    int count = -1;

    while (!in.eof())
    {        
        if (accumulated * 100 / totalSize != count)
        {
            count = accumulated * 100 / totalSize;
            std::printf("%I64u/%I64u\n", accumulated, totalSize);
            std::setbuf(stdout, NULL);
        }

        in.read(buffer.data(), buffer.size());
        auto read = in.gcount();

        accumulated += read;

        out.write(buffer.data(), read);

        int error = out.rdstate();
        if (error != std::ios::goodbit)
            return 3;
    }

    out.flush();
    out.close();
    in.close();

    return 0;
}
