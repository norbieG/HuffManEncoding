#include "filelib.h"
#include "encoding.h"
#include "pqueue.h"


// count frequencies: Examine a source file's contents and count the number of occurrences of each character.
Map<int, int> buildFrequencyTable(istream& input) {

    Map<int, int> freqTable;
    int ch = input.get();

    while (ch!=-1) {

        freqTable[ch]++;
        ch = input.get();
    }

    freqTable[PSEUDO_EOF] =1;
    rewindStream(input);

    return freqTable;
}

// build encoding tree: Build a binary tree with a particular structure, where each node represents a character and 
// its count of occurrences in the file. A priority queue is used to help build the tree along the way.

HuffmanNode* buildEncodingTree(const Map<int, int>& freqTable) {

    PriorityQueue<HuffmanNode*> pq;

    for (int ch : freqTable) {
        HuffmanNode* node = new HuffmanNode(ch, freqTable[ch]);  //create object that will not die at the end of the function
        pq.enqueue(node,freqTable[ch]);

    }

    while (pq.size() > 1) {

        HuffmanNode* left = pq.dequeue();
        HuffmanNode* right = pq.dequeue();

        int priority = left->count + right->count;
        HuffmanNode* joined = new HuffmanNode(NOT_A_CHAR,priority,left,right);

        pq.enqueue(joined, priority);

    }

    return pq.dequeue();
}

void buildEncodingMapHelper(HuffmanNode* root, Map<int, string>& map, string track) {

    if (root->isLeaf()) {
        map[root->character] = track;



    } else {

        buildEncodingMapHelper(root->zero, map,track+="0");
        track.erase(track.size()-1,1);
        buildEncodingMapHelper(root->one, map,track+="1");
    }

}

// build encoding map: Traverse the binary tree to discover the binary encodings of each character

Map<int, string> buildEncodingMap(HuffmanNode* encodingTree) {

    Map<int, string> encodingMap;
    buildEncodingMapHelper(encodingTree, encodingMap,"");

    return encodingMap;
}


// encode data: Re-examine the source file's contents, and for each character, output the encoded binary version 
// of that character to the destination file.

void encodeData(istream& input, const Map<int, string>& encodingMap, obitstream& output) {

    int ch = input.get();

    while (ch!=-1) {

        for (char digit : encodingMap[ch]) {

            output.writeBit(int(digit)-48);
        }

        ch = input.get();
    }

    for (char digit : encodingMap[PSEUDO_EOF]) {

        output.writeBit(int(digit)-48);

    }
    rewindStream(input);

}

void decodeData(ibitstream& input, HuffmanNode* encodingTree, ostream& output) {

    HuffmanNode* currentNode = encodingTree;
    HuffmanNode* nextNode = currentNode;

    int bit = input.readBit();

    while (bit != -1) {

        if (bit == 0) {
            nextNode = currentNode->zero;

        } else if (bit == 1) {
            nextNode = currentNode->one;
        }

        if (nextNode->isLeaf()) {

            if (nextNode->character == PSEUDO_EOF) {
                break;

            } else {
                output.put(char(nextNode->character));
                currentNode = encodingTree; // go up to the root
            }

        } else {
            currentNode = nextNode; // point where NextNode is
        }

        bit = input.readBit();
    }
}


void compress(istream& input, obitstream& output) {

    Map<int, int> freqTable = buildFrequencyTable(input);
    output << freqTable;
    HuffmanNode* tree = buildEncodingTree(freqTable);
    Map<int, string>encodingMap = buildEncodingMap(tree);
    encodeData(input,encodingMap, output);
    freeTree(tree);
}

void decompress(ibitstream& input, ostream& output) {

    Map<int, int> freqMap;
    input >> freqMap;
    HuffmanNode* tree = buildEncodingTree(freqMap);
    decodeData(input,tree, output);
    freeTree(tree);
}

void freeTree(HuffmanNode* node) {

    if (node == NULL) {

    } else {
        freeTree(node->zero);
        freeTree(node->one);
        delete node;
    }
}
