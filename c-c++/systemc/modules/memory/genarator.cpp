#include <iostream>
#include <fstream>
#include <iomanip>
#include <bitset>
#include <random>
#include <string>

std::string generate_binary_data(int data_bits, std::mt19937 &gen, std::uniform_int_distribution<> &chance_dist) {
    if (chance_dist(gen) == 0) {
        // 10% chance to return full 'x'
        return std::string(data_bits, 'x');
    } else {
        uint64_t max_val = (data_bits >= 64) ? UINT64_MAX : ((1ULL << data_bits) - 1);
        uint64_t value = gen() & max_val;
        std::string bits = std::bitset<64>(value).to_string();
        return bits.substr(64 - data_bits);
    }
}

void write_memory_to_csv(const std::string &filename, int num_rows, int data_bits) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    file << "address,data\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> chance_dist(0, 9);  // 10% for 'x'

    for (int i = 0; i < num_rows; ++i) {
        std::string data = generate_binary_data(data_bits, gen, chance_dist);
        file << "0x" << std::uppercase << std::hex << std::setw(3) << std::setfill('0') << i << ",";
        file << data << "\n";
    }

    file.close();
    std::cout << "Memory dump written to " << filename << "\n";
}

int main() {
    int num_entries = 600;  // number of memory entries
    int data_bits = 16;     // you can change this to 24, 32, etc.
    write_memory_to_csv("memory_dump.csv", num_entries, data_bits);
    return 0;
}
