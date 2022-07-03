/*
 * @Author       : Chivier Humber
 * @Date         : 2021-09-15 21:15:24
 * @LastEditors  : Chivier Humber
 * @LastEditTime : 2021-11-23 16:08:51
 * @Description  : file content
 */
#include "common.h"
#include "memory.h"

namespace virtual_machine_nsp {
    void memory_tp::ReadMemoryFromFile(std::string filename, int beginning_address) {
        // Read from the file
        std::ifstream fin(filename);
        int now_address = beginning_address;
        std::string line;
        while (getline(fin, line)) {
            int16_t inst = 0;

            for (int i = 0; i < 16; i++)
                inst += ((line[i] - '0') << (15 - i));

            memory[now_address++] = inst;
        }
    }// virtual machine namespace

    int16_t memory_tp::GetContent(int address) const {
        // get the content
        return memory[address];
    }//some problem

    int16_t& memory_tp::operator[](int address) {
        // get the content
        return memory[address];
    }
};
