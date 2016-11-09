#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class ArgParser {
    public:

        ArgParser  (int &argc, char **argv){
            for (int i=1; i < argc; ++i) {
                this->tokens.push_back(string(argv[i]));
            }
        }

        const std::string get_option_value(const std::string &option) const {
            std::vector<std::string>::const_iterator itr;
            itr = std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            return "";
        }

        bool option_exists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
        }

    private:
        std::vector <std::string> tokens;
};

enum ALGORITHM {
    HUFFMAN = 0,
    SHENNON = 1,
};


class EncodeShennon {
    public:


};

class DecodeShennon {
    public:


};

class EncodeHuffman {
    public:


};

class DecodeHuffman {
    public:


};

int main(int argc, char **argv){
    ArgParser input(argc, argv);
    if (input.option_exists("-h")) {
    }
    const string infile = input.get_option_value("-i");
    const string outfile = input.get_option_value("-i");
    const string alg = input.get_option_value("-a");
    if (input.option_exists("-h") 
            || infile.empty() || outfile.empty() || alg.empty()
            || (alg.compare("huffman") != 0 || alg.compare("shennon") != 0)) {
        cout << "Usage: program -a (huffman || shennon) -i input_file -o output_file" << endl;
    }
    enum ALGORITHM algo = HUFFMAN;
    if (alg.compare("shennon") == 0) {
        algo = SHENNON;
    }


    return 0;
}
