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

//=============================================================================
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

//=============================================================================
enum class ALGORITHM : bool
{
    huffman = true,
    shennon = false
};

//=============================================================================
typedef vector<bool> HuffCode;
typedef std::map<char, HuffCode> EncodeHuffmanMap;
typedef std::map<HuffCode, char> DecodeHuffmanMap;
typedef std::map<char,int> FrequencyTable;

class INode;
typedef std::shared_ptr<INode> NodePtr;

//=============================================================================
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


//=============================================================================
class InternalNode : public INode
{
public:
    NodePtr const left;
    NodePtr const right;

    InternalNode(NodePtr c0, NodePtr c1) : INode(c0->f + c1->f), left(c0), right(c1) {}
};

//=============================================================================
class LeafNode : public INode
{
public:
    const char c;

    LeafNode(int f, char c) : INode(f), c(c) {}
};

//=============================================================================
struct NodeCmp
{
    bool operator()(const NodePtr lhs, const NodePtr rhs) const { return lhs->f > rhs->f; }
};


//=============================================================================
class ucs4_ifstream : public basic_ifstream<char32_t> 
{

    locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};

    ucs4_ifstream(string& fname) : basic_ifstream<char32_t>(fname) {
        imbue(uft8_aware_locale);
    }
};

//=============================================================================
class ucs4_ofstream : public basic_ofstream<char32_t> 
{
    locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};

    ucs4_ofstream(string& fname) : basic_ofstream<char32_t>(fname) {
        imbue(uft8_aware_locale);
    }
};

//=============================================================================
class bit_ifstream : public ifstream
{
public:
    int nbit = 0;
    char bitbuf;
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv;

    using ifstream::ifstream;

    bit_ifstream& getucs4(char32_t& ch) {
        try {
            string utf8;
            getutf8(utf8);
            basic_string<char32_t> ucs4 = ucs4conv.from_bytes(utf8);
            ch = *ucs4.begin();
        } catch(const std::range_error& e) {
            cout << "Error!\n" << endl;
            this->setstate(ios::failbit);
        }
        return *this;
    }

    // i is the distance from left byte border
    bool hasbit(char ch, int i) {
        return ch & (1 << (8-i));
    }

    bit_ifstream& getutf8(string& utf8) {
        char firstbyte = '\0';
        getarray(&firstbyte, 8);

        // examine first byte
        if (hasbit(firstbyte, 0)) {
            // if multibyte
            int nbytes = 0;
            while (hasbit(firstbyte, nbytes)) {
                nbytes++;
            }
            char utf8_code[nbytes];
            std::fill(utf8_code, utf8_code+nbytes, 0);
            utf8_code[0] = firstbyte;
            getarray(&utf8_code[1], nbytes * 8);
            utf8.append(&utf8_code[0], nbytes);
        } else {
            // if single byte
            utf8.push_back(firstbyte);
        }
        return *this;
    }

    bit_ifstream& getarray(char* pbuf, int bits) {
        char* cur = pbuf;
        int i=0;
        while (i<bits) {
            bool bit;
            getbit(bit);
            if (bit) {
                *cur |= (1 << (7 - i%8));
            }
            i++;
            if (i % 8 == 0) {
                cur++;
            }
        }
        return *this;
    }

    bit_ifstream& getbit(bool& bit) {
        if (nbit == 0) {
            get(bitbuf);
            nbit = 8;
        }
        if (*this) {
            bit = (bitbuf & (1 << --nbit)) ? true : false;
        }
        return *this;
    }

    void skip_upto_next_byte() {
        // discard the rest of unread bitbuf
        nbit = 0;
    }

};



//=============================================================================
class bit_ofstream : public ofstream
{
public:
    int nbit = 8;
    char buffer = '\0';
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv;

    using ofstream::ofstream;

    bit_ofstream& putarray(const char* data, int bits) {
        const char* cur = data;
        int i=0;
        while (i<bits) {
            putbit( *cur & (1 << (7-(i%8))) );
            i++;
            if (i % 8 == 0) {
                cur++;
            }
        }
        return *this;
    }

    bit_ofstream& pututf8(string utf8) {
        putarray(utf8.c_str(), utf8.size() * 8);
        return *this;
    }

    bit_ofstream& putchar32(char32_t& ch) {
        try {
            basic_string<char> utf8 = ucs4conv.to_bytes(ch);
            pututf8(utf8);
        } catch(const std::range_error& e) {
            cout << "Error occured\n" << endl;
            this->setstate(ios::failbit);
        }
        return *this;
    }

    bit_ofstream& putbit(bool bit) {
        nbit--;
        if (bit) {
            buffer |= (1 << nbit);
        }
        if (nbit == 0) {
            put(buffer);
            flush();
            nbit = 8;
            buffer = '\0';
        }
        return *this;
    }

    void fill_upto_next_byte() {
        while (nbit != 8) {
            putbit(false);
        }
    }
};


//=============================================================================
class IEncoder
{
    public:
        shared_ptr<INode> root;
        FrequencyTable table;
        EncodeHuffmanMap ch2code;

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

        void WriteHuffmanTree(bit_ostream& os)
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

//=============================================================================
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

//=============================================================================
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

//=============================================================================
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

//=============================================================================
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
    cout << encoded.str() << endl;

    string indata
        = "1111122222223333333333444444444444444555555555555555555555666666666666666666666666666666666666666666666";

    bit_ofstream os{"out_test.txt"};
    vector<bool> data = {true, false, true, false, false, false, false, true, true};
    for (const auto& b : data) {
        os.putbit(b);
    // choose to test encode / decode
    stringstream rawtext{indata};
    cout << rawtext.str() << endl;

    stringstream encoded{};

    // encode with huffman, write name.haff
    EncodeShannon enc{};
    enc.FillFrequencyTable(rawtext);
    enc.BuildTree();
    enc.GenerateCodes();
    enc.WriteHuffmanTreeStr(encoded);
    enc.TransformEncodeStr(rawtext, encoded);
    for (EncodeHuffmanMap::const_iterator it = enc.ch2code.begin(); it != enc.ch2code.end(); ++it)
    {
        std::cout << it->first << " ";
        std::copy(it->second.begin(), it->second.end(), std::ostream_iterator<bool>(std::cout));
        std::cout << std::endl;
    }

    char32_t rus = L'д';
    os.putchar32(rus);
    os.fill_upto_next_byte();
    os.close();

    bit_ifstream is{"out_test.txt"};
    vector<bool> data_2{};
    for (uint32_t i=0; i < data.size(); i++) {
        bool b;
        is.getbit(b);
        data_2.push_back(b);
    stringstream decoded{};

    // decode with huffman, write name-unz-h.txt
    DecodeShannon dec{};
    dec.ReadHuffmanTreeStr(encoded);
    dec.TransformDecodeStr(encoded, decoded);
    for (DecodeHuffmanMap::const_iterator it = dec.code2ch.begin(); it != dec.code2ch.end(); ++it)
    {
        std::cout << it->second << " ";
        std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<bool>(std::cout));
        std::cout << std::endl;
    }
    char32_t back_rus;
    is.getucs4(back_rus);
    is.skip_upto_next_byte();
    is.close();
    cout << decoded.str() << endl;

    return 0;
}
