/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 15:10:31
 * @LastEditors  : Chivier Humber
 * @LastEditTime : 2021-11-23 15:34:30
 * @Description  : file content
 */

#include<string.h>
#include "assembler.h"

void label_map_tp::AddLabel(const std::string& str, const value_tp& val) {
    labels_.insert(std::make_pair(str, val));
}

value_tp label_map_tp::GetValue(const std::string& str) const {
    // User (vAddress, -1) to represent the error case
    if (labels_.find(str) == labels_.end()) {
        // not found
        return value_tp(vAddress, -1);
    }
    else {
        return labels_.at(str);
    }
}

std::ostream& operator<<(std::ostream& os, const StringType& item) {
    switch (item) {
    case sComment:
        os << "Comment ";
        break;
    case sLabel:
        os << "Label";
        break;
    case sValue:
        os << "Value";
        break;
    case sOpcode:
        os << "Opcode";
        break;
    case sOprand:
        os << "Oprand";
        break;
    default:
        os << "Error";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ValueType& val) {
    switch (val) {
    case vAddress:
        os << "Address";
        break;
    case vValue:
        os << "Value";
        break;
    default:
        os << "Error";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const value_tp& value) {
    if (value.type_ == vValue) {
        os << "[ " << value.type_ << " -- " << value.val_ << " ]";
    }
    else {
        os << "[ " << value.type_ << " -- " << std::hex << "0x" << value.val_ << " ]";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const label_map_tp& label_map) {
    for (auto item : label_map.labels_) {
        os << "Name: " << item.first << " " << item.second << std::endl;
    }
    return os;
}

int RecognizeNumberValue(std::string str) {
    int ERROR = std::numeric_limits<int>::max();
    int Carry_num;
    if (str[0] == 'X') Carry_num = 16;
    else if (str[0] == '#') Carry_num = 10;
    else return atoi(str.c_str());

    bool sign_flag = (str[1] == '-') ? true : false;

    std::string num = sign_flag ? str.substr(2) : str.substr(1);
    if (!num.size()) return ERROR;

    int sum = 0, pow = 1;
    for (int j = num.size() - 1; j >= 0; j--) {
        if (num[j] >= '0' && num[j] <= '9')
            sum += (num[j] - '0') * pow;
        else if (num[j] >= 'A' && num[j] <= 'F' && Carry_num == 16)
            sum += (num[j] - 'A' + 10) * pow;
        else return ERROR;//ERROR char

        pow *= Carry_num;
    }

    return sign_flag ? -sum : sum;
}

std::string NumberToAssemble(const int& number) {
    // Convert the number into a 16 bit binary string
    int num = number;
    int minus_flag = num < 0 ? true : false;
    if (minus_flag) num = -num;
    std::string s = "";
    while (num) {
        s += (num % 2 ? '1' : '0');
        num >>= 1;
    }
    int length = s.size();
    for (int i = 0; i < 16 - length; i++) {
        s += '0';
    }

    if (minus_flag) {
        auto low_bit_position = s.find("1");
        if (low_bit_position == std::string::npos) low_bit_position = -1;
        for (int i = low_bit_position + 1; i < 16 - length; i++)
            s.at(i) = (s.at(i) == '0') ? '1' : '0';
    }//Turn it to opposite number

    std::reverse(s.begin(), s.end());
    return s;
}

std::string NumberToAssemble(const std::string& number) {
    // Convert the number into a 16 bit binary string
    // You might use `RecognizeNumberValue` in this function
    return NumberToAssemble(RecognizeNumberValue(number));
}

std::string ConvertBin2Hex(std::string bin) {
    // Convert the binary string into a hex string
    const int pow[4] = { 8, 4, 2, 1 };

    std::string str = "0000";
    for (int i = 0; i < 4; i++) {
        for (int j = 4 * i; j < 4 * i + 4; j++)
            str[i] += (bin[j] == '1' ? pow[j % 4] : 0);

        if (str[i] > '9')
            str[i] = str[i] - '9' + 'A' - 1;
    }
    return str;
}

std::string assembler::TranslateOprand(int current_address, std::string str, int opcode_length) {
    // Translate the oprand
    Trim(str);//To delete the special char
    auto item = label_map.GetValue(str);

    if (!(item.getType() == vAddress && item.getVal() == -1)) {
        // str is a label
        int label_address = item.getVal();
        int offset = label_address - current_address - 1;
        std::string offset_str = NumberToAssemble(offset);
        offset_str = offset_str.substr(16 - opcode_length);

        return offset_str;
    }
    else if (str[0] == 'R') {
        // str is a register
        const std::string regis_table[8] = { "000", "001", "010", "011", "100", "101", "110", "111" };
        return regis_table[str[1] - '0'];
    }
    else {
        // str is an immediate number
        int imm = RecognizeNumberValue(str);

        std::string imm_str = NumberToAssemble(imm);
        imm_str = imm_str.substr(16 - opcode_length);

        return imm_str;
    }
}

// TODO: add error line index
int assembler::assemble(std::string input_filename, std::string output_filename) {
    // assemble main function
    // parse program

    // store the original string
    std::vector<std::string> file_content;
    std::vector<std::string> origin_file;
    // store the tag for line
    std::vector<LineStatusType> file_tag;
    std::vector<std::string> file_comment;
    std::vector<int> file_address;
    int orig_address = -1;
    std::string line;

    std::ifstream input_file(input_filename);

    if (input_file.is_open()) {
        // Scan #0:
        // Read file
        // Store comments
        while (std::getline(input_file, line)) {
            // Remove the leading and trailing whitespace
            Trim(line);
            if (line.size() == 0) {
                // Empty line
                continue;
            }
            std::string origin_line = line;
            // Convert `line` into upper case
            //support both toupper and tolower inst?
            std::transform(line.begin(), line.end(), line.begin(), toupper);//对string有影响

            // Store comments
            auto comment_position = line.find(";");
            if (comment_position == std::string::npos) {
                // No comments here
                file_content.push_back(line);
                origin_file.push_back(origin_line);
                file_tag.push_back(lPending);
                file_comment.push_back("");
                file_address.push_back(-1);
                continue;
            }
            else {
                std::string comment_str = line.substr(comment_position);
                std::string content_str = line.substr(0, comment_position);
                //std::cout<<content_str<<std::endl;
                // Split content and comment

                // Delete the leading whitespace and the trailing whitespace
                Trim(comment_str);
                Trim(content_str);
                // Store content and comment separately
                file_content.push_back(content_str);
                origin_file.push_back(origin_line);
                file_comment.push_back(comment_str);
                if (content_str.size() == 0) {
                    // The whole line is a comment
                    file_tag.push_back(lComment);
                }
                else {
                    file_tag.push_back(lPending);
                }
                file_address.push_back(-1);
            }
        }
    }
    else {
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }

    // Scan #1:
    // Scan for the .ORIG & .END pseudo code
    // Scan for jump label, value label, line comments *Store to map
    int line_address = -1;
    for (int line_index = 0; line_index < file_content.size(); ++line_index) {
        if (file_tag[line_index] == lComment) {
            // This line is comment
            continue;
        }

        auto line = file_content[line_index];

        // * Pseudo Command
        if (line[0] == '.') {
            file_tag[line_index] = lPseudo;
            // This line is a pseudo instruction
            // Only .ORIG & .END are line-pseudo-command
            auto line_stringstream = std::istringstream(line);//串流
            std::string pseudo_command;
            line_stringstream >> pseudo_command;

            if (pseudo_command == ".ORIG") {
                // .ORIG
                std::string orig_value;
                line_stringstream >> orig_value;
                orig_address = RecognizeNumberValue(orig_value);
                //std::cout<<orig_address<<std::endl;
                if (orig_address == std::numeric_limits<int>::max()) {
                    // @ Error address
                    return -2;
                }
                file_address[line_index] = -1;
                line_address = orig_address;
            }
            else if (pseudo_command == ".END") {
                // .END
                file_address[line_index] = -1;
                // If set line_address as -1, we can also check if there are programs after .END
                // line_address = -1;
            }
            else if (pseudo_command == ".STRINGZ") {
                file_address[line_index] = line_address;
                std::string word;
                line_stringstream >> word;
                if (word[0] != '\"' || word[word.size() - 1] != '\"') {
                    // @ Error String format error   //"shflk"
                    return -6;
                }
                auto num_temp = word.size() - 1;
                line_address += num_temp;
            }
            else if (pseudo_command == ".FILL") {
                file_address[line_index] = line_address;
                line_address++;
                std::string word;
                line_stringstream >> word;
                auto num_temp = RecognizeNumberValue(word);
                if (num_temp == std::numeric_limits<int>::max()) {
                    // @ Error Invalid Number input @ FILL
                    return -4;
                }
                if (num_temp > 65535 || num_temp < -65536) {
                    // @ Error Too large or too small value  @ FILL
                    return -5;
                }
            }
            else if (pseudo_command == ".BLKW") {
                std::string word;
                line_stringstream >> word;
                auto num_temp = RecognizeNumberValue(word);
                if (num_temp == std::numeric_limits<int>::max()) {
                    // @ Error Invalid Number input @ BLKW
                    return -6;
                }
                if (num_temp > 65535 || num_temp < -65536) {
                    // @ Error Too large or too small value @ BLKW
                    return -7;
                }
                line_address += num_temp;
            }
            else {
                // @ Error Unknown Pseudo command
                return -100;
            }

            continue;
        }

        if (line_address == -1) {
            // @ Error Program begins before .ORIG
            std::cout<<line_index<<std::endl;
            return -3;
        }

        file_address[line_index] = line_address;
        line_address++;

        // Split the first word in the line
        auto line_stringstream = std::stringstream(line);
        std::string word;
        line_stringstream >> word;
        if (IsLC3Command(word) != -1 || IsLC3TrapRoutine(word) != -1) {
            // * This is an operation line
            file_tag[line_index] = lOperation;
            continue;
        }

        // * Label
        // Store the name of the label
        auto label_name = word;
        // Split the second word in the line
        line_stringstream >> word;

        if (IsLC3Command(word) != -1 || IsLC3TrapRoutine(word) != -1 || word == "") {
            // a label used for jump/branch
            file_tag[line_index] = lOperation;
            
            label_map.AddLabel(label_name, value_tp(vAddress, file_address[line_index]));
            //To avoid empty lable,didn't finish// @ Error Empty lable;
        }
        else {
            file_tag[line_index] = lPseudo;
            if (word == ".FILL") {
                line_stringstream >> word;
                auto num_temp = RecognizeNumberValue(word);
                if (num_temp == std::numeric_limits<int>::max()) {
                    // @ Error Invalid Number input @ FILL
                    return -4;
                }
                if (num_temp > 65535 || num_temp < -65536) {
                    // @ Error Too large or too small value  @ FILL
                    return -5;
                }
                label_map.AddLabel(label_name, value_tp(vAddress, file_address[line_index]));
            }
            if (word == ".BLKW") {
                // modify label map
                // modify line address
                label_map.AddLabel(label_name, value_tp(vAddress, file_address[line_index]));

                std::string word;
                line_stringstream >> word;
                auto num_temp = RecognizeNumberValue(word);
                if (num_temp == std::numeric_limits<int>::max()) {
                    // @ Error Invalid Number input @ BLKW
                    return -6;
                }
                if (num_temp > 65535 || num_temp < -65536) {
                    // @ Error Too large or too small value @ BLKW
                    return -7;
                }
                line_address += num_temp - 1;//some problem
                //line_address += num_temp - 2;
            }
            if (word == ".STRINGZ") {
                // modify label map
                // modify line address
                label_map.AddLabel(label_name, value_tp(vAddress, file_address[line_index]));

                std::string word;
                line_stringstream >> word;
                if (word[0] != '\"' || word[word.size() - 1] != '\"') {
                    // @ Error String format error
                    return -6;
                }
                auto num_temp = word.size() - 1;
                line_address += num_temp - 1;
            }
        }//The format of input-string
    }

    if (gIsDebugMode) {
        // Some debug information
        std::cout << std::endl;
        std::cout << "Label Map: " << std::endl;
        std::cout << label_map << std::endl;

        for (auto index = 0; index < file_content.size(); ++index) {
            std::cout << std::hex << file_address[index] << " ";
            std::cout << file_content[index] << std::endl;
        }
    }

    // Scan #2:
    // Translate *to bit

    // Check output file
    if (output_filename == "") {
        output_filename = input_filename;
        if (output_filename.find(".") == std::string::npos) {
            output_filename = output_filename + ".bit";
        }
        else {
            output_filename = output_filename.substr(0, output_filename.rfind("."));
            output_filename = output_filename + ".bit";
        }
    }//some problem,.asm to .bit

    std::ofstream output_file;
    // Create the output file
    output_file.open(output_filename);
    if (!output_file) {
        // @ Error at output file
        return -20;
    }

    for (int line_index = 0; line_index < file_content.size(); ++line_index) {
        if (file_address[line_index] == -1 || file_tag[line_index] == lComment) {
            // * This line is not necessary to be translated
            continue;
        }

        auto line = file_content[line_index];
        auto line_stringstream = std::stringstream(line);

        if (gIsDebugMode)
            output_file << std::hex << file_address[line_index] << ": ";
        if (file_tag[line_index] == lPseudo) {
            // Translate pseudo command
            std::string word;
            line_stringstream >> word;
            if (word[0] != '.') {
                // Fetch the second word
                // Eliminate the label
                line_stringstream >> word;
            }

            if (word == ".FILL") {
                std::string number_str;
                line_stringstream >> number_str;
                auto output_line = NumberToAssemble(number_str);
                if (gIsHexMode)//output with Hex
                    output_line = ConvertBin2Hex(output_line);
                output_file << output_line << std::endl;
            }
            else if (word == ".BLKW") {
                // Fill 0 here
                std::string output_line = "0000000000000000";
                if (gIsHexMode)
                    output_line = ConvertBin2Hex(output_line);

                std::string num_str;
                line_stringstream >> num_str;
                int num = RecognizeNumberValue(num_str);
                for (int i = 0; i < num; i++) {
                    output_file << output_line << std::endl;
                }
            }
            else if (word == ".STRINGZ") {
                // Fill string here
                int _word=0;
                std::string word;
                line_stringstream >> word;
                for (int i = 1; i < word.size() - 1; i++) {
                    _word=(int)word[i];
                    auto output_line = NumberToAssemble(_word);
                    if (gIsHexMode)
                        output_line = ConvertBin2Hex(output_line);
                    output_file << output_line << std::endl;
                }
                //auto output_line = NumberToAssemble(0);
                //if (gIsHexMode)
                //    output_line = ConvertBin2Hex(output_line);
                //output_file << output_line << std::endl;//'\0'

                continue;
            }
        }

            if (file_tag[line_index] == lOperation) {
                std::string word;
                line_stringstream >> word;
                if (IsLC3Command(word) == -1 && IsLC3TrapRoutine(word) == -1) {
                    // Eliminate the label
                    line_stringstream >> word;
                }

                std::string result_line = "";
                auto command_tag = IsLC3Command(word);
                auto parameter_str = line.substr(line.find(word) + word.size());
                parameter_str = Trim(parameter_str);

                // Convert comma into space for splitting
                std::replace(parameter_str.begin(), parameter_str.end(), ',', ' ');

                auto current_address = file_address[line_index];

                std::vector<std::string> parameter_list;
                auto parameter_stream = std::stringstream(parameter_str);
                while (parameter_stream >> word) {
                    parameter_list.push_back(word);
                }
                auto parameter_list_size = parameter_list.size();
                if (command_tag != -1) {
                    // This is a LC3 command
                    switch (command_tag) {
                    case 0:
                        // "ADD"
                        result_line += "0001";
                        if (parameter_list_size != 3) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1]);
                        if (parameter_list[2][0] == 'R') {
                            // The third parameter is a register
                            result_line += "000";
                            result_line += TranslateOprand(current_address, parameter_list[2]);
                        }
                        else {
                            // The third parameter is an immediate number
                            result_line += "1";
                            // std::cout << "hi " << parameter_list[2] << std::endl;
                            result_line += TranslateOprand(current_address, parameter_list[2], 5);
                        }
                        break;
                    case 1:
                        // "AND"
                        result_line += "0101";
                        if (parameter_list_size != 3) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1]);
                        if (parameter_list[2][0] == 'R') {
                            // The third parameter is a register
                            result_line += "000";
                            result_line += TranslateOprand(current_address, parameter_list[2]);
                        }
                        else {
                            // The third parameter is an immediate number
                            result_line += "1";
                            // std::cout << "hi " << parameter_list[2] << std::endl;
                            result_line += TranslateOprand(current_address, parameter_list[2], 5);
                        }
                        break;
                    case 2:
                        // "BR" No thing to do
                        result_line += "0000000";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 3:
                        // "BRN"
                        result_line += "0000100";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 4:
                        // "BRZ"
                        result_line += "0000010";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 5:
                        // "BRP"
                        result_line += "0000001";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 6:
                        // "BRNZ"
                        result_line += "0000110";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 7:
                        // "BRNP"
                        result_line += "0000101";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 8:
                        // "BRZP"
                        result_line += "0000011";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 9:
                        // "BRNZP"
                        result_line += "0000111";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 9);
                        break;
                    case 10:
                        // "JMP"
                        result_line += "1100000";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += "000000";
                        break;
                    case 11:
                        // "JSR"
                        result_line += "01001";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 11);
                        break;
                    case 12:
                        // "JSRR"
                        result_line += "0100000";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += "000000";
                        break;
                    case 13:
                        // "LD"
                        result_line += "0010";
                        if (parameter_list_size != 2) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1], 9);
                        break;
                    case 14:
                        // "LDI"
                        result_line += "1010";
                        if (parameter_list_size != 2) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1], 9);
                        break;
                    case 15:
                        // "LDR"
                        result_line += "0110";
                        if (parameter_list_size != 3) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1]);
                        result_line += TranslateOprand(current_address, parameter_list[2], 6);
                        break;
                    case 16:
                        // "LEA"
                        result_line += "1110";
                        if (parameter_list_size != 2) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1], 9);
                        break;
                    case 17:
                        // "NOT"
                        result_line += "1001";
                        if (parameter_list_size != 2) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1]);
                        result_line += "111111";
                        break;
                    case 18:
                        // RET
                        result_line += "1100000111000000";
                        if (parameter_list_size != 0) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        break;
                    case 19:
                        // RTI
                        result_line += "1000000000000000";
                        if (parameter_list_size != 0) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        break;
                    case 20:
                        // ST
                        result_line += "0011";
                        if (parameter_list_size != 2) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1], 9);
                        break;
                    case 21:
                        // STI
                        result_line += "1011";
                        if (parameter_list_size != 2) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1], 9);
                        break;
                    case 22:
                        // STR
                        result_line += "0111";
                        if (parameter_list_size != 3) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0]);
                        result_line += TranslateOprand(current_address, parameter_list[1]);
                        result_line += TranslateOprand(current_address, parameter_list[2], 6);
                        break;
                    case 23:
                        // TRAP
                        result_line += "11110000";
                        if (parameter_list_size != 1) {
                            // @ Error parameter numbers
                            return -30;
                        }
                        result_line += TranslateOprand(current_address, parameter_list[0], 8);
                        break;
                    default:
                        // Unknown opcode
                        // @ Error
                        break;
                    }
                }
                else {
                    // This is a trap routine
                    command_tag = IsLC3TrapRoutine(word);
                    switch (command_tag) {
                    case 0:
                        // x20
                        result_line += "1111000000100000";
                        break;
                    case 1:
                        // x21
                        result_line += "1111000000100001";
                        break;
                    case 2:
                        // x22
                        result_line += "1111000000100010";
                        break;
                    case 3:
                        // x23
                        result_line += "1111000000100011";
                        break;
                    case 4:
                        // x24
                        result_line += "1111000000100100";
                        break;
                    case 5:
                        // x25
                        result_line += "1111000000100101";
                        break;
                    default:
                        // @ Error Unknown command
                        return -50;
                    }
                }

                if (gIsHexMode)
                    result_line = ConvertBin2Hex(result_line);
                output_file << result_line << std::endl;
            }
    }
    // Close the output file
    output_file.close();
    // OK flag
    return 0;
}
