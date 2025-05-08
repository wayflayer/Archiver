#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>
#include <iostream>
#include <fstream>

namespace io = boost::iostreams;

void compress_file(const std::string& input_filename, const std::string& output_filename)
{
    // Открываем исходный файл
    std::ifstream input(input_filename, std::ios_base::in | std::ios_base::binary);
    if (!input)
    {
        throw std::runtime_error("Failed to open input file: " + input_filename);
    }

    // Создаем выходной файл с gzip-сжатием
    io::filtering_ostream out;
    out.push(io::gzip_compressor()); // добавляем gzip компрессор
    out.push(io::file_sink(output_filename, std::ios_base::out | std::ios_base::binary)); // добавляем файл как приемник

    if (!out)
    {
        throw std::runtime_error("Failed to open output file: " + output_filename);
    }

    // Копируем данные из входного файла в выходной поток с сжатием
    out << input.rdbuf();
}

int main()
{
    try
    {
        compress_file("cntlm_0.92.3-1.3_arm64.deb", "output.gz");
        std::cout << "File compressed successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}