#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <fstream>
#include <sstream>
#include <map>
#include <stdexcept>
#include <utility>
#include <queue>
#include <iterator>

using namespace std;

class ArgParser
{
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

enum class ALGORITHM : bool
{
    huffman = true,
    shennon = false
};

typedef std::vector<bool> HuffCode;
typedef std::map<char, HuffCode> EncodeHuffmanMap;
typedef std::map<HuffCode, char> DecodeHuffmanMap;
typedef std::map<char,int> FrequencyTable;

class INode;
typedef std::shared_ptr<INode> NodePtr;

class INode
{
public:
    const int f;

    // mark this class as polymorphic, add virtual function
    virtual ~INode() {}

protected:
    INode(int f) : f(f) {}
};


class InternalNode : public INode
{
public:
    NodePtr const left;
    NodePtr const right;

    InternalNode(NodePtr c0, NodePtr c1) : INode(c0->f + c1->f), left(c0), right(c1) {}
};

class LeafNode : public INode
{
public:
    const char c;

    LeafNode(int f, char c) : INode(f), c(c) {}
};

struct NodeCmp
{
    bool operator()(const NodePtr lhs, const NodePtr rhs) const { return lhs->f > rhs->f; }
};


class IEncoder
{
    public:
        void FillFrequencyTable(iostream& is) {
            if (is.good()) {
                // we can continue
                char ch;
                while (is.get(ch)) {
                    ++(this->table)[ch];
                }
                if (is.eof()) {
                    // clear eof and rewind input stream
                    is.clear();
                    is.seekg(0, std::ios::beg);
                } else {
                    // we stopped reading the file, but it is not EOF yet.
                    throw std::runtime_error("Could not read file");
                }
                // we reached eof. all ok
            }
        }

        void GenerateCodes()
        {
            InnerGenerateCodes(root, HuffCode{});
        }

        void InnerGenerateCodes(const NodePtr node, const HuffCode& prefix)
        {
            if (const shared_ptr<const LeafNode> lf = dynamic_pointer_cast<const LeafNode>(node))
            {
                this->ch2code[lf->c] = prefix;
            }
            else if (const shared_ptr<const InternalNode> in = dynamic_pointer_cast<const InternalNode>(node))
            {
                HuffCode leftPrefix = prefix;
                leftPrefix.push_back(false);
                InnerGenerateCodes(in->left, leftPrefix);
                HuffCode rightPrefix = prefix;
                rightPrefix.push_back(true);
                InnerGenerateCodes(in->right, rightPrefix);
            }
        }

        void WriteHuffmanTree(iostream& os)
        {
            InnerWriteHuffmanTree(root, os);
        }

        void InnerWriteHuffmanTree(const NodePtr node, iostream& os)
        {
            if (const shared_ptr<const LeafNode> lf = dynamic_pointer_cast<const LeafNode>(node))
            {
                os << "1-" << lf->c << "-";
            }
            else if (const shared_ptr<const InternalNode> in = dynamic_pointer_cast<const InternalNode>(node))
            {
                os << '0';
                InnerWriteHuffmanTree(in->left, os);
                InnerWriteHuffmanTree(in->right, os);
            }
        }

    shared_ptr<INode> root;
    FrequencyTable table;
    EncodeHuffmanMap ch2code;
};

class IDecoder
{
    public:
        IDecoder() : code2ch{} { }

        void ReadHuffmanTree(iostream& is)
        {
            auto var = HuffCode();
            InnerReadHuffmanTree(var, is);
        }

        void InnerReadHuffmanTree(const HuffCode& prefix, iostream& is)
        {
            char now;
            if (! is.get(now)) {
                throw runtime_error("Could not reconstruct tree.");
            }
            if (now == '1')
            {
                // read letter
                char data[3];
                is.read(data, 3);
                char letter = data[1];
                // add to map
                code2ch[prefix] = letter;
            }
            else if (now == '0')
            {
                HuffCode leftPrefix = prefix;
                leftPrefix.push_back(false);
                InnerReadHuffmanTree(leftPrefix, is);
                HuffCode rightPrefix = prefix;
                rightPrefix.push_back(true);
                InnerReadHuffmanTree(rightPrefix, is);
            }
        }

    DecodeHuffmanMap code2ch{};
};

class EncodeShannon : public IEncoder
{

}

class EncodeHuffman : public IEncoder
{
    public:
        void BuildTree()
        {
            std::priority_queue<NodePtr, std::vector<NodePtr>, NodeCmp> trees;
            for (const auto& stats : table)
            {
                NodePtr np{new LeafNode{stats.second, stats.first}};
                trees.push(np);
            }
            while (trees.size() > 1)
            {
                NodePtr childR = trees.top();
                trees.pop();
                NodePtr childL = trees.top();
                trees.pop();
                NodePtr parent{new InternalNode{childR, childL}};
                trees.push(parent);
            }
            root = shared_ptr<INode>{trees.top()};
        }


        void TransformEncode(iostream& is, iostream& os)
        {
            if (is.good() && os.good()) {
                // we can continue
                char in_ch;
                while (is.get(in_ch))
                {
                    string code;
                    for (const auto& bit : this->ch2code[in_ch]) {
                        code += bit ? "1" : "0";
                    }
                    os << code;
                }
                if (is.eof()) {
                    // clear eof and rewind input stream
                    is.clear();
                    is.seekg(0, std::ios::beg);
                } else {
                    // we stopped reading the file, but it is not EOF yet.
                    throw std::runtime_error("Could not encode");
                }
                // we reached eof. all ok
            }
        }

};

class DecodeHuffman : public IDecoder
{
    public:
        void TransformDecode(iostream& is, iostream& os)
        {
            if (is.good() && os.good()) {
                // we can continue
                char in_ch;
                HuffCode code{};
                while (is.get(in_ch))
                {
                    code.push_back(in_ch=='1' ? true : false);
                    if(code2ch.find(code) != code2ch.end()) {
                        os << code2ch[code];
                        code.clear();
                    }
                }
                if (! is.eof()) {
                    // we stopped reading the file, but it is not EOF yet.
                    throw std::runtime_error("Could not encode");
                }
                // we reached eof. all ok
            }
        }

};


int main(int argc, char **argv)
{
    // Parse agruments
    bool show_help = false;
    // Check that options are valid
    ArgParser input(argc, argv);
    if (input.option_exists("-h")) {
        show_help = true;
    }
    // input file
    const string infile = input.get_option_value("-i");
    if (infile.empty()) show_help = true;
    // output file
    const string outfile = input.get_option_value("-i");
    if (outfile.empty()) show_help = true;
    // algorithm name
    const string alg = input.get_option_value("-a");
    ALGORITHM algo = ALGORITHM::huffman;
    if (alg.compare("shennon") == 0) {
        algo = ALGORITHM::shennon;
    }
    else if (alg.compare("huffman") == 0) {
        algo = ALGORITHM::huffman;
    }
    else {
        show_help = true;
    }
    if (show_help) {
        cout << "Usage: program -a (huffman || shennon) -i input_file(.haff || .shan || .txt) -o output_file" << endl;
        return -1;
    }

    string encoded
        = "01-6-001-3-01-1-1-2-01-4-1-5-101010101010101010101011101110111011101110111011100100100100100100100100100100110110110110110110110110110110110110110110110";
    string indata
        = "1111122222223333333333444444444444444555555555555555555555666666666666666666666666666666666666666666666";

    // choose to test encode / decode
    stringstream indatastream{indata};
    // stringstream indatastream{encoded};
    stringstream outdatastream{};



    // Check for valid combination of options
    // and do the work in each case
    if (algo==ALGORITHM::huffman && infile.find(".txt") != string::npos) {
        // encode with huffman, write name.haff
        EncodeHuffman enc{};
        enc.FillFrequencyTable(indatastream);
        enc.BuildTree();
        enc.GenerateCodes();
        enc.WriteHuffmanTree(outdatastream);
        enc.TransformEncode(indatastream, outdatastream);
        for (EncodeHuffmanMap::const_iterator it = enc.ch2code.begin(); it != enc.ch2code.end(); ++it)
        {
            std::cout << it->first << " ";
            std::copy(it->second.begin(), it->second.end(), std::ostream_iterator<bool>(std::cout));
            std::cout << std::endl;
        }
        cout << outdatastream.str() << endl;
    }
    else if (algo==ALGORITHM::shennon && infile.find(".txt") != string::npos) {
        // encode with shennon, write name.shan
    }
    else if (algo==ALGORITHM::huffman && infile.find(".haff") != string::npos) {
        // decode with huffman, write name-unz-h.txt
        DecodeHuffman dec{};
        dec.ReadHuffmanTree(indatastream);
        dec.TransformDecode(indatastream, outdatastream);
        for (DecodeHuffmanMap::const_iterator it = dec.code2ch.begin(); it != dec.code2ch.end(); ++it)
        {
            std::cout << it->second << " ";
            std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<bool>(std::cout));
            std::cout << std::endl;
        }
        cout << outdatastream.str() << endl;
    }
    else if (algo==ALGORITHM::shennon && infile.find(".shan") != string::npos) {
        // decode with shennon, write name-unz-s.txt
    }
    else {
        cout << "Usage: program -a (huffman || shennon) -i input_file(.haff || .shan || .txt) -o output_file" << endl;
    }

    return 0;
}




