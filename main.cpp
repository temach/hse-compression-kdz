#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <fstream>
#include <map>
#include <stdexcept>
#include <utility>
#include <queue>
#include <iterator>

using namespace std;

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

enum class ALGORITHM : bool {
    huffman = true,
    shennon = false
};


typedef std::vector<bool> HuffCode;
typedef std::map<char, HuffCode> HuffCodeMap;
typedef std::map<HuffCode, char> Code2LetterHuffmanMap;
typedef std::map<char,int> FrequencyTable;

class INode
{
public:
    const int f;

    // mark this class as polymorphic, add virtual function
    virtual ~INode() {}

protected:
    INode(int f) : f(f) {}
};

typedef std::shared_ptr<INode> NodePtr;

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

NodePtr BuildTree(const FrequencyTable& freq_table)
{
    std::priority_queue<NodePtr, std::vector<NodePtr>, NodeCmp> trees;

    for (const auto& stats : freq_table)
    {
        NodePtr np(new LeafNode(stats.second, stats.first));
        trees.push(np);
    }
    while (trees.size() > 1)
    {
        NodePtr childR = trees.top();
        trees.pop();
        NodePtr childL = trees.top();
        trees.pop();
        NodePtr parent(new InternalNode(childR, childL));
        trees.push(parent);
    }
    return trees.top();
}

void GenerateCodes(const NodePtr node, const HuffCode& prefix, HuffCodeMap& outCodes)
{
    if (const shared_ptr<const LeafNode> lf = dynamic_pointer_cast<const LeafNode>(node))
    {
        outCodes[lf->c] = prefix;
    }
    else if (const shared_ptr<const InternalNode> in = dynamic_pointer_cast<const InternalNode>(node))
    {
        HuffCode leftPrefix = prefix;
        leftPrefix.push_back(false);
        GenerateCodes(in->left, leftPrefix, outCodes);
        HuffCode rightPrefix = prefix;
        rightPrefix.push_back(true);
        GenerateCodes(in->right, rightPrefix, outCodes);
    }
}

void WriteHuffmanTree(const NodePtr node, string& out)
{
    if (const shared_ptr<const LeafNode> lf = dynamic_pointer_cast<const LeafNode>(node))
    {
        out += "1-";
        out += lf->c;
        out += "-";
    }
    else if (const shared_ptr<const InternalNode> in = dynamic_pointer_cast<const InternalNode>(node))
    {
        out += '0';
        WriteHuffmanTree(in->left, out);
        WriteHuffmanTree(in->right, out);
    }
}

void ReadHuffmanTree(const HuffCode& prefix,  Code2LetterHuffmanMap& outCodes, ifstream& input)
{
    char now;
    if (! input.get(now)) {
        throw runtime_error("Could not reconstruct tree.");
    }
    if (now == '1')
    {
        // read letter
        char data[4];
        input >> data;
        char letter = data[1];
        // add to map
        outCodes[prefix] = letter;
    }
    else if (now == '0')
    {
        HuffCode leftPrefix = prefix;
        leftPrefix.push_back(false);
        ReadHuffmanTree(leftPrefix, outCodes, input);
        HuffCode rightPrefix = prefix;
        rightPrefix.push_back(true);
        ReadHuffmanTree(rightPrefix, outCodes, input);
    }
}

void FillFrequencyTable(FrequencyTable& freq_table, const string& infile) {
    ifstream in;
    in.open(infile);
    if (in.is_open()) {

        // we can continue
        char ch;
        while (in.get(ch)) {
            ++freq_table[ch];
        }
        if (! in.eof()) {
            // we stopped reading the file, but it is not EOF yet.
            throw std::runtime_error("Could not read file");
        }
        // we reached eof. all ok
    }
    in.close();
}

void TransformEncode(string infile, string outfile, HuffCodeMap& encodemap) 
{
    ifstream in;
    in.open(infile);
    ofstream out;
    out.open(outfile, std::ofstream::app | std::ofstream::out);
    if (in.is_open() && out.is_open()) {
        // we can continue
        char in_ch;
        while (in.get(in_ch)) 
        {
            string enc;
            for (const auto& bit : encodemap[in_ch]) {
                enc += bit ? "1" : "0";
            }
            out << enc;
        }
        if (! in.eof()) {
            // we stopped reading the file, but it is not EOF yet.
            throw std::runtime_error("Could not encode");
        }
        // we reached eof. all ok
    }
    in.close();
    out.close();
}

void TransformEncode(string infile, string outfile, HuffCodeMap& encodemap) {
    ifstream in;
    in.open(infile);
    ofstream out;
    out.open(outfile, std::ofstream::app | std::ofstream::out);
    if (in.is_open() && out.is_open()) {
        // we can continue
        char in_ch;
        while (in.get(in_ch)) {
            string enc;
            for (const auto& bit : encodemap[in_ch]) {
                enc += bit ? "1" : "0";
            }
            out << enc;
        }
        if (! in.eof()) {
            // we stopped reading the file, but it is not EOF yet.
            throw std::runtime_error("Could not encode");
        }
        // we reached eof. all ok
    }
    in.close();
    out.close();
}


int main(int argc, char **argv){
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

    // Count the frquencies
    FrequencyTable freq_table;
    // FillFrequencyTable(freq_table, infile);
    freq_table.emplace('1',5);
    freq_table.emplace('2',7);
    freq_table.emplace('3',10);
    freq_table.emplace('4',15);
    freq_table.emplace('5',20);
    freq_table.emplace('6',45);
    NodePtr root = BuildTree(freq_table);
    HuffCodeMap codes;
    GenerateCodes(root, HuffCode(), codes);

    for (HuffCodeMap::const_iterator it = codes.begin(); it != codes.end(); ++it)
    {
        std::cout << it->first << " ";
        std::copy(it->second.begin(), it->second.end(), std::ostream_iterator<bool>(std::cout));
        std::cout << std::endl;
    }

    string hufftree;
    WriteHuffmanTree(root, hufftree);
    cout << "___" << hufftree << "___" << endl;

    ofstream os;
    os.open(outfile);
    if (os.is_open()) {
        os << hufftree;
    }
    TransformEncode(infile, outfile, codes);


    queue<char> q;
    for (const auto ch : hufftree) {
        q.push(ch);
    }
    Code2LetterHuffmanMap read_map;
    ifstream is;
    is.open(outfile);
    if (is.is_open()) {
        ReadHuffmanTree(HuffCode(), read_map, is);
        ofstream other;
        other.open("testing.again");
        TransformDecode(is, read_map);
    }

    for (Code2LetterHuffmanMap::const_iterator it = read_map.begin(); it != read_map.end(); ++it)
    {
        std::cout << it->second << " ";
        std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<bool>(std::cout));
        std::cout << std::endl;
    }

    // Check for valid combination of options
    // and do the work in each case
    if (algo==ALGORITHM::huffman && infile.find(".txt") != string::npos) {
        // encode with huffman, write name.haff
        EncodeHuffman enc = EncodeHuffman();

    }
    else if (algo==ALGORITHM::shennon && infile.find(".txt") != string::npos) {
        // encode with shennon, write name.shan
    }
    else if (algo==ALGORITHM::huffman && infile.find(".haff") != string::npos) {
        // decode with huffman, write name-unz-h.txt
    }
    else if (algo==ALGORITHM::shennon && infile.find(".shan") != string::npos) {
        // decode with shennon, write name-unz-s.txt
    }
    else {
        cout << "Usage: program -a (huffman || shennon) -i input_file(.haff || .shan || .txt) -o output_file" << endl;
    }

    return 0;
}




