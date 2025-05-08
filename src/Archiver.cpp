#define NOMINMAX
#include "../include/Archiver.hpp"
#include <boost/iostreams/device/file.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <csignal>
#endif

namespace io = boost::iostreams;
namespace fs = boost::filesystem;

std::atomic<bool> Archiver::abort_flag(false);
std::atomic<size_t> Archiver::bytes_processed(0);
size_t Archiver::total_bytes = 0;
std::mutex Archiver::io_mutex;
const size_t BUFFER_SIZE = 64 * 1024;

#ifdef _WIN32
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::lock_guard<std::mutex> lock(Archiver::io_mutex);
        std::cout << "\nInterrupt signal received, stopping...\n";
        Archiver::abort_flag = true;
        return TRUE;
    }
    return FALSE;
}
#endif

void Archiver::setupSignalHandlers() {
#ifdef _WIN32
    SetConsoleCtrlHandler(consoleHandler, TRUE);
#else
    struct sigaction sa;
    sa.sa_handler = [](int) {
        std::lock_guard<std::mutex> lock(Archiver::io_mutex);
        std::cout << "\nInterrupt signal received, stopping...\n";
        Archiver::abort_flag = true;
    };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif
}

void Archiver::print_progress() {
    std::lock_guard<std::mutex> lock(io_mutex);
    double progress = static_cast<double>(bytes_processed) / total_bytes;
    int bar_width = 50;
    
    std::cout << "\r[";
    int pos = static_cast<int>(bar_width * progress);
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::setw(3) << static_cast<int>(progress * 100.0) << "% "
              << bytes_processed / (1024 * 1024) << "MB/" 
              << total_bytes / (1024 * 1024) << "MB";
    std::cout.flush();
}

void Archiver::calculate_total_size(const fs::path& path) {
    total_bytes = 0;
    if (fs::is_directory(path)) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry) && !abort_flag) {
                total_bytes += fs::file_size(entry);
            }
        }
    } else {
        total_bytes = fs::file_size(path);
    }
}

bool Archiver::add_to_archive(io::filtering_ostream& out, const fs::path& file_path) {
    std::ifstream in(file_path.string(), std::ios::binary);
    if (!in) return false;

    const auto file_size = fs::file_size(file_path);
    out << "FILE:" << file_path.filename().string() << "\n";
    out << "SIZE:" << file_size << "\n";

    std::vector<char> buffer(BUFFER_SIZE);
    size_t bytes_read = 0;
    
    while (in.read(buffer.data(), buffer.size())) {
        if (abort_flag) return false;
        
        out.write(buffer.data(), in.gcount());
        bytes_read += in.gcount();
        bytes_processed += in.gcount();
        
        if (bytes_read % (1024 * 1024) == 0) { 
            print_progress();
        }
    }
    out.write(buffer.data(), in.gcount());
    bytes_processed += in.gcount();
    
    print_progress();
    return true;
}

bool Archiver::createArchive(const fs::path& input, const fs::path& output) {
    bytes_processed = 0;
    calculate_total_size(input);

    try {
        io::filtering_ostream out;
        out.push(io::gzip_compressor(io::gzip_params(io::gzip::best_compression)));
        out.push(io::file_sink(output.string(), std::ios::binary));

        if (fs::is_directory(input)) {
            out << "DIR:" << input.filename().string() << "\n";
            
            for (const auto& entry : fs::recursive_directory_iterator(input)) {
                if (abort_flag) break;
                if (fs::is_regular_file(entry) && !add_to_archive(out, entry)) {
                    return false;
                }
            }
        } else {
            if (!add_to_archive(out, input)) return false;
        }

        if (!abort_flag) {
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "\nArchive created successfully: " << output << "\n";
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(io_mutex);
        std::cerr << "\nError: " << e.what() << "\n";
        return false;
    }
}

bool Archiver::extract_file(io::filtering_istream& in, const fs::path& output_dir, 
                          const std::string& filename, size_t file_size) {
    fs::path out_path = output_dir / filename;
    std::ofstream out(out_path.string(), std::ios::binary);
    if (!out) return false;

    std::vector<char> buffer(BUFFER_SIZE);
    size_t remaining = file_size;
    
    while (remaining > 0 && !abort_flag) {
        size_t to_read = (std::min)(remaining, buffer.size());
        in.read(buffer.data(), to_read);
        out.write(buffer.data(), in.gcount());
        remaining -= in.gcount();
        bytes_processed += in.gcount();
        
        if (bytes_processed % (1024 * 1024) == 0) {
            print_progress();
        }
    }
    return remaining == 0;
}

bool Archiver::extractArchive(const fs::path& archive, const fs::path& output_dir) {
    bytes_processed = 0;
    try {
        io::filtering_istream in;
        in.push(io::gzip_decompressor());
        in.push(io::file_source(archive.string(), std::ios::binary));

        fs::create_directories(output_dir);
        std::string line;
        std::string current_file;
        size_t current_size = 0;

        while (std::getline(in, line) && !abort_flag) {
            if (line.find("DIR:") == 0) {
                fs::create_directory(output_dir / line.substr(4));
            }
            else if (line.find("FILE:") == 0) {
                current_file = line.substr(5);
            }
            else if (line.find("SIZE:") == 0) {
                current_size = std::stoull(line.substr(5));
                if (!extract_file(in, output_dir, current_file, current_size)) {
                    return false;
                }
            }
        }

        if (!abort_flag) {
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "\nExtraction completed successfully to: " << output_dir << "\n";
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(io_mutex);
        std::cerr << "\nError: " << e.what() << "\n";
        return false;
    }
}