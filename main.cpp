#include "include/Archiver.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage:\n"
                  << "  Pack:   " << argv[0] << " pack <input> <output.gz>\n"
                  << "  Unpack: " << argv[0] << " unpack <archive.gz> <output>\n";
        return 1;
    }

    Archiver::setupSignalHandlers();
    
    const std::string mode(argv[1]);
    const fs::path input(argv[2]);
    const fs::path output(argv[3]);
    
    bool result = false;
    try {
        if (mode == "pack") {
            std::cout << "Creating archive... (Press Ctrl+C to cancel)\n";
            result = Archiver::createArchive(input, output);
        } 
        else if (mode == "unpack") {
            std::cout << "Extracting archive... (Press Ctrl+C to cancel)\n";
            result = Archiver::extractArchive(input, output);
        }
        else {
            std::cerr << "Error: Unknown mode. Use 'pack' or 'unpack'\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    if (!result) {
        std::cerr << "Operation failed or was interrupted\n";
        return 1;
    }
    
    return 0;
}