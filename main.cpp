#include <fstream>
#include <vector>
#include <string>

int main(int argc, char *argv[])
{
    if ( argc != 3 )
    {
        return 1; //not enough arguments
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
    }

    out.flush();
    out.close();
    in.close();

    return 0;
}
