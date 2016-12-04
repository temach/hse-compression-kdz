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
using std::int64_t;
using std::string;
using std::vector;
using std::iostream;
using std::stringstream;
using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::runtime_error;


class OpsCounter {
public:
    uint64_t operations = 0;

    void add(int64_t new_ops) {
        operations += new_ops;
    }

    void reset() {
        operations = 0;
    }
} ops; // <- global variable


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
typedef vector<bool> VariableCode;
typedef std::map<char32_t, VariableCode> EncodeHuffmanMap;
typedef std::map<VariableCode, char32_t> DecodeMap;
typedef std::map<char32_t,int> FrequencyTable;

class INode;
typedef std::shared_ptr<INode> NodePtr;

//=============================================================================
class INode
{
public:
    const int f;

    // mark this class as polymorphic, add virtual function
    virtual ~INode() {}

    friend bool operator < (const NodePtr& lhs, const NodePtr& rhs)
    {
        ops.add(2);
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
    const char32_t c;

    LeafNode(int f, char32_t c) : INode(f), c(c) {}
};

//=============================================================================
struct NodeCmp
{
    bool operator()(const NodePtr lhs, const NodePtr rhs) const {
        ops.add(2);
        return lhs->f > rhs->f;
    }
};


//=============================================================================
class ucs4_ifstream : public basic_ifstream<char32_t>
{
public:
    locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};

    using basic_ifstream<char32_t>::basic_ifstream;

    ucs4_ifstream(string& fname) : basic_ifstream<char32_t>(fname, ios::binary | ios::out) {
        imbue(uft8_aware_locale);
    }
};

//=============================================================================
class ucs4_ofstream : public basic_ofstream<char32_t> 
{
public:
    locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};

    using basic_ofstream<char32_t>::basic_ofstream;

    ucs4_ofstream(string& fname) : basic_ofstream<char32_t>(fname, ios::binary | ios::out) {
        imbue(uft8_aware_locale);
    }
};

//=============================================================================
class bit_ifstream : public ifstream
{
public:
    int nbit = 0;
    uint8_t trash_size = 0;
    bool lastbyte = false;
    char bitbuf;
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv{};

    bit_ifstream(const string& fname) : ifstream(fname, ios::binary | ios::in) {
        // plus initialisation
        ops.add(5);
        getarray(reinterpret_cast<char*>(&trash_size), 8);
    }

    bit_ifstream& getucs4(char32_t& ch) {
        try {
            ops.add(3);
            string utf8;
            getutf8(utf8);
            basic_string<char32_t> ucs4 = ucs4conv.from_bytes(utf8);
            ch = *ucs4.begin();
        } catch(const std::range_error& e) {
            ops.add(1);
            cout << "Error!\n" << endl;
            ops.add(1);
            this->setstate(ios::failbit);
        }
        ops.add(1);
        return *this;
    }

    // i is the distance from left byte border
    bool hasbit(char ch, int i) {
        ops.add(3);
        return ch & (1 << (7-i));
    }

    bit_ifstream& getutf8(string& utf8) {
        ops.add(2);
        char firstbyte = '\0';
        getarray(&firstbyte, 8);
        // examine first byte
        ops.add(2);
        if (hasbit(firstbyte, 0)) {
            // if multibyte
            ops.add(7);
            int nbytes = 0;
            while (hasbit(firstbyte, nbytes)) {
                ops.add(1);
                nbytes++;
            }
            char utf8_code[nbytes];
            std::fill(utf8_code, utf8_code+nbytes, 0);
            utf8_code[0] = firstbyte;
            getarray(&utf8_code[1], (nbytes-1) * 8);
            utf8.append(&utf8_code[0], nbytes);
        } else {
            // if single byte
            ops.add(1);
            utf8.push_back(firstbyte);
        }
        return *this;
    }

    bit_ifstream& getarray(char* pbuf, int bits) {
        ops.add(3);
        char* cur = pbuf;
        int i=0;
        while (i<bits) {
            ops.add(4);
            bool bit;
            getbit(bit);
            if (bit) {
                ops.add(4);
                *cur |= (1 << (7 - i%8));
            }
            i++;
            if (i % 8 == 0) {
                ops.add(1);
                cur++;
            }
        }
        return *this;
    }

    bit_ifstream& getbit(bool& bit) {
        ops.add(2);
        if (nbit == 0) {
            ops.add(3);
            get(bitbuf);
            nbit = 8;
            if (peek() == EOF) {
                ops.add(2);
                lastbyte = true;
                clear();
            }
        }
        // disregard the last trash_size bits in the last byte
        if (! lastbyte) {
            ops.add(4);
            bit = (bitbuf & (1 << --nbit)) ? true : false;
        } else if (lastbyte && nbit > trash_size) {
            ops.add(4);
            bit = (bitbuf & (1 << --nbit)) ? true : false;
            if (nbit == 0) {
                ops.add(1);
                setstate(ios::eofbit);
            }
        } else {
            ops.add(1);
            setstate(ios::eofbit);
        }
        return *this;
    }

};



//=============================================================================
class bit_ofstream : public ofstream
{
public:
    int nbit = 8;
    char buffer = '\0';
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv;

    bit_ofstream(const string& fname) : ofstream(fname, ios::binary | ios::out) {
        // plus initialisation
        ops.add(3);
        start_writing();
    }

    bit_ofstream& putarray(const char* data, int bits) {
        ops.add(3);
        const char* cur = data;
        int i=0;
        while (i<bits) {
            ops.add(7);
            putbit( *cur & (1 << (7-(i%8))) );
            i++;
            if (i % 8 == 0) {
                ops.add(1);
                cur++;
            }
        }
        return *this;
    }

    bit_ofstream& pututf8(const string& utf8) {
        ops.add(4);
        putarray(utf8.c_str(), utf8.size() * 8);
        return *this;
    }

    bit_ofstream& putchar32(const char32_t& ch) {
        ops.add(1);
        try {
            ops.add(3);
            basic_string<char> utf8 = ucs4conv.to_bytes(ch);
            pututf8(utf8);
        } catch(const std::range_error& e) {
            ops.add(2);
            cout << "Error occured\n" << endl;
            this->setstate(ios::failbit);
        }
        return *this;
    }

    bit_ofstream& putbit(const bool bit) {
        ops.add(2);
        nbit--;
        if (bit) {
            ops.add(2);
            buffer |= (1 << nbit);
        }
        if (nbit == 0) {
            ops.add(3);
            put(buffer);
            nbit = 8;
            buffer = '\0';
        }
        return *this;
    }

    void start_writing() {
        // write an empty byte, which will get overriden in stop_writing()
        ops.add(1);
        put('\0');
    }

    void stop_writing() {
        // remember the trash size
        ops.add(3);
        char trash_size = nbit;
        // write trash
        while (nbit != 8) {
            ops.add(1);
            putbit(false);
        }
        // rewind
        seekp(0, ios::beg);
        // write the trash size
        putarray(&trash_size, 8);
    }
};


//=============================================================================
class IEncoder
{
    public:

        void Encode(ucs4_ifstream& in, bit_ofstream& out) {
            ops.add(5);
            FillFrequencyTable(in);
            BuildTree();
            GenerateCodes();
            WriteTree(out);
            TransformTextEncode(in, out);
        }

    protected:

        shared_ptr<INode> root;
        FrequencyTable table;

        // mark the class as polymorphic, add virtual function
        virtual void BuildTree() = 0;

        void FillFrequencyTable(ucs4_ifstream& is) {
            ops.add(1);
            // write the trash size
            if (is.good()) {
                // we can continue
                char32_t ch;
                ops.add(2);
                while (is.get(ch).good()) {
                    ops.add(2);
                    ++(this->table)[ch];
                }
                ops.add(1);
                if (is.eof()) {
                    // clear eof and rewind input stream
                    ops.add(2);
                    is.clear();
                    is.seekg(0, std::ios::beg);
                } else {
                    // we stopped reading the file, but it is not EOF yet.
                    ops.add(1);
                    throw std::runtime_error("Could not read file");
                }
                // we reached eof. all ok
            }
        }

        void GenerateCodes()
        {
            ops.add(1);
            InnerGenerateCodes(root, VariableCode{});
        }

        void InnerGenerateCodes(const NodePtr node, const VariableCode& prefix)
        {
            if (const shared_ptr<const LeafNode> lf = dynamic_pointer_cast<const LeafNode>(node))
            {
                ops.add(2);
                this->ch2code[lf->c] = prefix;
            }
            else if (const shared_ptr<const InternalNode> in = dynamic_pointer_cast<const InternalNode>(node))
            {
                ops.add(6);
                VariableCode leftPrefix = prefix;
                leftPrefix.push_back(false);
                InnerGenerateCodes(in->left, leftPrefix);
                VariableCode rightPrefix = prefix;
                rightPrefix.push_back(true);
                InnerGenerateCodes(in->right, rightPrefix);
            }
        }

        void WriteTree(bit_ofstream& os)
        {
            ops.add(1);
            InnerWriteTree(root, os);
        }

        void InnerWriteTree(const NodePtr node, bit_ofstream& os)
        {
            if (const shared_ptr<const LeafNode> lf = dynamic_pointer_cast<const LeafNode>(node))
            {
                ops.add(2);
                os.putbit(true);
                os.putchar32(lf->c);
            }
            else if (const shared_ptr<const InternalNode> in = dynamic_pointer_cast<const InternalNode>(node))
            {
                ops.add(3);
                os.putbit(false);
                InnerWriteTree(in->left, os);
                InnerWriteTree(in->right, os);
            }
        }

        void TransformTextEncode(ucs4_ifstream& is, bit_ofstream& os)
        {
            ops.add(2);
            if (is.good() && os.good()) {
                // we can continue
                char32_t in_ch;
                ops.add(2);
                while (is.get(in_ch).good())
                {
                    for (const auto& bit : this->ch2code[in_ch]) {
                        ops.add(2);
                        os.putbit(bit);
                    }
                }
                ops.add(1);
                if (is.eof()) {
                    // clear eof and rewind input stream
                    ops.add(2);
                    is.clear();
                    is.seekg(0, std::ios::beg);
                } else {
                    // we stopped reading the file, but it is not EOF yet.
                    ops.add(1);
                    throw std::runtime_error("Could not encode");
                }
                // we reached eof. all ok
            }
        }

    private:
        EncodeHuffmanMap ch2code;
};

//=============================================================================
class EncodeHuffman : public IEncoder 
{
public:
    void BuildTree()
    {
        std::priority_queue<NodePtr, std::vector<NodePtr>, NodeCmp> trees;
        for (const auto& stats : table)
        {
            ops.add(2);
            NodePtr np{new LeafNode{stats.second, stats.first}};
            trees.push(np);
        }
        while (trees.size() > 1)
        {
            ops.add(7);
            NodePtr childR = trees.top();
            trees.pop();
            NodePtr childL = trees.top();
            trees.pop();
            // Internal node constructor also requiers +1 ops
            NodePtr parent{new InternalNode{childR, childL}};
            trees.push(parent);
        }
        ops.add(2);
        root = shared_ptr<INode>{trees.top()};
    }
};

//=============================================================================
class EncodeShannon : public IEncoder
{
    typedef vector<NodePtr> LeafVec;
    typedef LeafVec::const_iterator LeafIter;

    public:

        virtual void BuildTree() override
        {
            LeafVec leaves;
            for (const auto& stats : table)
            {
                ops.add(2);
                NodePtr np{new LeafNode{stats.second, stats.first}};
                leaves.push_back(np);
            }
            ops.add(1);
            sort(leaves.begin(), leaves.end(), NodeCmp{});
            // start recursion
            ops.add(2);
            root = InnerBuildTree(leaves.begin(), leaves.end() - 1);
        }

    protected:

        NodePtr InnerBuildTree(LeafIter first, LeafIter last)
        {
            ops.add(1);
            if (distance(first, last) == 0 )
            {
                ops.add(1);
                return *first;
            }
            else
            {
                ops.add(8);
                LeafIter split = FindBreakingIndex(first, last);
                NodePtr childL = InnerBuildTree(first, split);
                NodePtr childR = InnerBuildTree(split+1, last);
                // Internal node constructor also requiers +1 ops
                return shared_ptr<INode>{new InternalNode{childL, childR}};
            }
        }

        LeafIter FindBreakingIndex(LeafIter first, LeafIter last)
        {
            ops.add(6);
            LeafIter left_ptr = first;
            LeafIter right_ptr = last;
            int sumleft = (*left_ptr)->f;
            int sumright = (*right_ptr)->f;
            // func = right - left;
            // we want abs(func) to be 0 (minimal possible)
            while (left_ptr+1 < right_ptr) {
                ops.add(10);
                int func = sumleft - sumright;
                int valueleft = (*(left_ptr+1))->f;
                int valueright = (*(right_ptr-1))->f;
                if (abs(func + valueleft) < abs(func - valueright)) {
                    ops.add(2);
                    sumleft += valueleft;
                    left_ptr++;
                }
                else {
                    ops.add(2);
                    sumright += valueright;
                    right_ptr--;
                }
            }
            return left_ptr;
        }
};

//=============================================================================
class Decoder
{
    public:
        void ReadTree(bit_ifstream& is)
        {
            ops.add(2);
            auto var = VariableCode();
            InnerReadTree(var, is);
        }

        void Decode(bit_ifstream& is, ucs4_ofstream& os) {
            ops.add(2);
            ReadTree(is);
            TransformDecode(is, os);
        }

    protected:

        DecodeMap code2ch{};

        void InnerReadTree(const VariableCode& prefix, bit_ifstream& is)
        {
            ops.add(1);
            if (! is.good()) {
                throw runtime_error("Could not reconstruct tree.");
            }
            bool bit;
            ops.add(1);
            is.getbit(bit);
            if (bit)
            {
                // read letter
                ops.add(3);
                char32_t ch;
                is.getucs4(ch);
                // add to map
                code2ch[prefix] = ch;
            }
            else
            {
                ops.add(6);
                VariableCode leftPrefix = prefix;
                leftPrefix.push_back(false);
                InnerReadTree(leftPrefix, is);
                VariableCode rightPrefix = prefix;
                rightPrefix.push_back(true);
                InnerReadTree(rightPrefix, is);
            }
        }

        void TransformDecode(bit_ifstream& is, ucs4_ofstream& os)
        {
            ops.add(2);
            if (is.good() && os.good()) {
                // we can continue
                bool bit;
                VariableCode code{};
                ops.add(2);
                while (is.getbit(bit).good())
                {
                    ops.add(2);
                    code.push_back(bit);
                    if (code2ch.find(code) != code2ch.end()) {
                        ops.add(3);
                        os << code2ch[code];
                        code.clear();
                    }
                }
                ops.add(1);
                if (! is.eof()) {
                    // we stopped reading the file, but it is not EOF yet.
                    ops.add(1);
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
        cout << "Usage: program -a (huffman || shennon) -i input_file(.haff || .shan || .txt)" << endl;
        return -1;
    }

    string name = infile;
    string::size_type ext_txt = infile.find(".txt");
    string::size_type ext_haff = infile.find(".haff");
    string::size_type ext_shan = infile.find(".shan");

    // Check for valid combination of options
    // and do the work in each case
    if (algo==ALGORITHM::huffman && infile.find(".txt") != string::npos) {
        name.erase(ext_txt, 4);
        string fout = name + ".haff";
        // encode with huffman, write name.haff
        ops.add(4);
        ucs4_ifstream rawtext{infile};
        bit_ofstream outs{fout};
        EncodeShannon enc{};
        enc.Encode(rawtext, outs);
        rawtext.close();
        outs.stop_writing();
        outs.close();
    }
    else if (algo==ALGORITHM::shennon && infile.find(".txt") != string::npos) {
        name.erase(ext_txt, 4);
        string fout = name + ".shan";
        // encode with shennon, write name.shan
        ops.add(4);
        ucs4_ifstream rawtext{infile};
        bit_ofstream outs{fout};
        EncodeShannon enc{};
        enc.Encode(rawtext, outs);
        rawtext.close();
        outs.stop_writing();
        outs.close();
    }
    else if (algo==ALGORITHM::huffman && infile.find(".haff") != string::npos) {
        name.erase(ext_haff, 5);
        string fout = name + "-unz-h.txt";
        // decode with huffman, write name-unz-h.txt
        ops.add(3);
        bit_ifstream enc_stream{infile};
        ucs4_ofstream dec_stream{fout};
        Decoder dec{};
        dec.Decode(enc_stream, dec_stream);
        enc_stream.close();
        dec_stream.close();
    }
    else if (algo==ALGORITHM::shennon && infile.find(".shan") != string::npos) {
        name.erase(ext_shan, 5);
        string fout = name + "-unz-s.txt";
        // decode with shennon, write name-unz-s.txt
        ops.add(3);
        bit_ifstream enc_stream{infile};
        ucs4_ofstream dec_stream{fout};
        Decoder dec{};
        dec.Decode(enc_stream, dec_stream);
        enc_stream.close();
        dec_stream.close();
    }
    else {
        cout << "Usage: program -a (huffman || shennon) -i input_file(.haff || .shan || .txt)" << endl;
    }
    return 0;
}
