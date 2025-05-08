#pragma once
#include <string>
#include <atomic>
#include <mutex>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

class Archiver {
public:
    static std::atomic<bool> abort_flag;
    static std::atomic<size_t> bytes_processed;
    static size_t total_bytes;
    static std::mutex io_mutex;
    
    static bool createArchive(const fs::path& input, const fs::path& output);
    static bool extractArchive(const fs::path& archive, const fs::path& output_dir);
    static void setupSignalHandlers();
    
private:
    static void print_progress();
    static void calculate_total_size(const fs::path& path);
    static bool add_to_archive(io::filtering_ostream& out, const fs::path& file_path);
    static bool extract_file(io::filtering_istream& in, const fs::path& output_dir, 
                          const std::string& filename, size_t file_size);
};