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
#include <cstdint>
#include <codecvt>
#include <locale>

using namespace std;

using std::locale;
using std::uint64_t;
using std::uint32_t;
using std::uint8_t;
using std::string;
using std::vector;
using std::iostream;
using std::stringstream;
using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::runtime_error;

class ArgParser
{
    public:
        ArgParser  (int &argc, char **argv){
            for (int i=1; i < argc; ++i) {
                this->tokens.push_back(string(argv[i]));
            }
        }

        const string get_option_value(const string &option) const {
            vector<string>::const_iterator itr;
            itr = std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            return "";
        }

        bool option_exists(const string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
        }

    private:
        vector <string> tokens;
};

enum class ALGORITHM : bool
{
    huffman = true,
    shennon = false
};

typedef vector<bool> HuffCode;
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

    friend bool operator < (const INode& lhs, const INode& rhs)
    {
        return lhs.f > rhs.f;
    }

    friend bool operator < (const NodePtr& lhs, const NodePtr& rhs)
    {
        return lhs->f > rhs->f;
    }

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


class Input {
public:
    iostream& ios;
    bool multibyte;

    Input(iostream& s, bool mb) : ios(s) {
        multibyte = mb;
        if (mb) {

        } else {

        }
    }

    void ReadChar() {
        if (multibyte) {
            ReadMultibyte();
        } else {
            ReadByte();
        }
    }

    void ReadMultibyte() {
        vector<char32_t> text;
        char32_t ch;
        while (ifs.get(ch)) {
            text.push_back(ch);
        }
        ifs.close();
    }

    void Read
};

class IEncoder
{
    public:
        void FillFrequencyTable(basic_iostream<char32_t>& is) {
            if (is.good()) {
                // we can continue
                char32_t ch;
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
            uint64_t n_entries = static_cast<uint64_t>(ch2code.size());
            // write how many pairs
            os << n_entries;

            // find max key length
            uint64_t max_code_len_bits = -1;
            for (const auto& entry : ch2code) {
                if (entry.second.size() > max_code_len_bits) {
                    max_code_len_bits = entry.second.size();
                }
            }

            locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};
            basic_ifstream<char32_t> ifs{"in_test.txt"};
            ifs.imbue(uft8_aware_locale);
            vector<char32_t> text;
            char32_t ch;
            while (ifs.get(ch)) {
                text.push_back(ch);
            }
            ifs.close();
            //uint64_t bytes_for_code = (max_code_len_bits/8) + ((max_code_len_bits % 8) ? 1 : 0);
            //uint64_t bits_for_code = bytes_for_code * 8;

            ofstream ofs{"out_test.txt", ios_base::out | ios_base::binary};
            wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs2conv;

            cout << "Writing to file (UTF-8)... ";
            ofs.put('\x59');
            ofs.flush();
            for (const auto& ch : text) {
                ofs.put('\x59');
                ofs.flush();
                try {
                    string bytes = ucs2conv.to_bytes(ch);
                    ofs << bytes;
                    ofs.flush();
                } catch(const std::range_error& e) {
                    cout << "Error occured\n" << endl;
                }
            }
            cout << "done!\n";
            ofs.close();
            for (const auto& entry : ch2code) {
                w character = entry.first;
                uint64_t code = 0;
                uint64_t code_bit_len = entry.second.size();
                for (uint32_t i=0; i < code_bit_len; i++) {
                    if (entry.second[code_bit_len - i - 1]) {
                        code |= (1 << i);
                    }
                }
                os << character << code;
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
            if (is.good()) {

            }
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

class codingstream : public 

class EncodeShannon : public IEncoder
{
    public:
        typedef vector<NodePtr> LeafVec;
        typedef LeafVec::const_iterator LeafIter;

        LeafIter FindBreakingIndex(LeafIter first, LeafIter last)
        {
            LeafIter left_ptr = first;
            LeafIter right_ptr = last;
            int sumleft = (*left_ptr)->f;
            int sumright = (*right_ptr)->f;
            //f func = right - left;
            // we want abs(func) = 0
            while (left_ptr+1 < right_ptr) {
                int func = sumleft - sumright;
                int valueleft = (*(left_ptr+1))->f;
                int valueright = (*(right_ptr-1))->f;
                if (abs(func + valueleft) < abs(func - valueright)) {
                    sumleft += valueleft;
                    left_ptr++;
                }
                else {
                    sumright += valueright;
                    right_ptr--;
                }
            }
            return left_ptr;

        }

        void Encode(basic_iostream<char32_t>& in, basic_iostream<char>& out) {
            FillFrequencyTable(in);
            BuildTree();
            GenerateCodes();
            WriteHuffmanTree(out);
            TransformTextEncode(in, out);
            for (EncodeHuffmanMap::const_iterator it = ch2code.begin(); it != ch2code.end(); ++it)
            {
                std::cout << it->first << " ";
                std::copy(it->second.begin(), it->second.end(), std::ostream_iterator<bool>(std::cout));
                std::cout << std::endl;
            }
        }

        void BuildTree()
        {
            LeafVec leaves;
            for (const auto& stats : table)
            {
                NodePtr np{new LeafNode{stats.second, stats.first}};
                leaves.push_back(np);
            }
            sort(leaves.begin(), leaves.end(), NodeCmp{});
            // start recursion
            root = InnerBuildTree(leaves.begin(), leaves.end() - 1);
        }

        NodePtr InnerBuildTree(LeafIter first, LeafIter last)
        {
            if (distance(first, last) == 0 )
            {
                return *first;
            }
            else
            {
                LeafIter split = FindBreakingIndex(first, last);
                NodePtr childL = InnerBuildTree(first, split);
                NodePtr childR = InnerBuildTree(split+1, last);
                return shared_ptr<INode>{new InternalNode{childL, childR}};
            }
        }

        void TransformTextEncode(basic_iostream<char32_t>& is, basic_iostream<char>& os)
        {
            if (is.good() && os.good()) {
                // we can continue
                char32_t in_ch;
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

class DecodeShannon : public IDecoder
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
    using std::cout;
    using std::endl;

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

    string indata
        = "1111122222223333333333444444444444444555555555555555555555666666666666666666666666666666666666666666666";


    std::locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};
    basic_ifstream<char32_t> ifs{"in_test.txt"};
    ifs.imbue(uft8_aware_locale);

    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs2conv;
    try {
        string bytes = ucs2conv.to_bytes(ch);
        ofs << bytes;
        ofs.flush();
    } catch(const std::range_error& e) { }
    // choose to test encode / decode
    basic_stringstream<char32_t> rawtext{indata};

    cout << rawtext.str() << endl;


    // encode with huffman, write name.haff
    EncodeShannon enc{};
    stringstream encoded = enc.Encode(rawtext);
    cout << encoded.str() << endl;

    stringstream decoded{};

    // decode with huffman, write name-unz-h.txt
    DecodeShannon dec{};
    dec.ReadHuffmanTree(encoded);
    dec.TransformDecode(encoded, decoded);
    for (DecodeHuffmanMap::const_iterator it = dec.code2ch.begin(); it != dec.code2ch.end(); ++it)
    {
        std::cout << it->second << " ";
        std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<bool>(std::cout));
        std::cout << std::endl;
    }
    cout << decoded.str() << endl;

    return 0;
}




