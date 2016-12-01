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
    char charbuf[4];
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv;

    using ifstream::ifstream;

    bit_ifstream& getutf8char(char32_t& ch) {
       read(&charbuf[0], 4);
       try {
           // convert from [first, last) byte in buffer
           basic_string<char32_t> ucs4
            = ucs4conv.from_bytes(&charbuf[0], &charbuf[gcount()]);
           if (ucs4conv.converted() > 0) {
               ch = *ucs4.begin();
           } else {
               cout << "Error!\n" << endl;
               this->setstate(ios::failbit);
           }
       } catch(const std::range_error& e) {
           cout << "Error!\n" << endl;
           this->setstate(ios::failbit);
       }
    }

    bit_ifstream& getbit(bool& bit) {
        if (nbit == 0) {
            get(bitbuf);
            nbit = 8;
        }
        if (*this) {
            bit = ((bitbuf >> --nbit) & 1) == 1 ? true : false;
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
    char buffer;
    wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs4conv;

    using ofstream::ofstream;

    bit_ofstream& putchar32(char32_t& ch) {
        try {
            basic_string<char> utf8 = ucs4conv.to_bytes(ch);
            // write(utf8.c_str(), utf8.size());
            for (const auto& byte : utf8) {
                put(byte);
            }
            flush();
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
        }
        return *this;
    }

    void fill_upto_next_byte() {
        while (nbit != 8) {
            put(false);
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
            uint64_t n_entries = static_cast<uint64_t>(ch2code.size());
            // write how many pairs
            os << n_entries;
            for (const auto& entry : ch2code) {
                char32_t character = entry.first;
                uint64_t code = 0;
                // this is <=64
                int code_bit_len = entry.second.size();
                for (int i=0; i < code_bit_len; i++) {
                    if (entry.second[code_bit_len - i - 1]) {
                        code |= (1 << i);
                    }
                }
                os.put
                os << code << character;
            }
        }
};

//=============================================================================
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

    std::locale uft8_aware_locale{std::locale(), new std::codecvt_utf8<char32_t>};
    basic_ifstream<char32_t> ifs{"in_test.txt"};
    ifs.imbue(uft8_aware_locale);

    return 0;
}
