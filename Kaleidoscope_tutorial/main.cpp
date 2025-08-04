//
// Created by zuxinggu on 2025/8/4.
//

#include <iostream>
#include <string>

// return tokens [0-255] to represent unknown character, otherwise known tokens
enum Token {
    tok_eof = -1,

    // key word
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
};

static std::string Id; // current id if tok_identifier
static double Number; // current num value if tok_number

/// gettok _ return the next token from standard input.
static int gettok() {
    static int LastChar = ' ';
    // skip any space
    while (isspace(LastChar)) {
        LastChar = getchar();
    }
    // check id
    if (isalpha(LastChar)) {
        Id = LastChar;
        while (isalnum(LastChar = getchar())) {
            Id += LastChar;
        }
        // special id, def
        if (Id == "def") {
            return tok_def;
        }
        if (Id == "extern") {
            return tok_extern;
        }
        return tok_identifier;
    }
    // check number
    if (isdigit(LastChar)) {
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');
        Number = std::stod(NumStr);
        return tok_number;
    }
    // comment
    if (LastChar == '#') {
        do {
            LastChar = getchar();
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        if (LastChar != EOF) {
            // continue to next token
            return gettok();
        }
    }
    if (LastChar == EOF) {
        return tok_eof;
    }
    int ThisChar = LastChar;
    LastChar = getchar(); // always move LastChar to the next one
    return ThisChar;
}

int main() {
    std::cout << "Hello World!" << std::endl;
    while (true) {
        int tok = gettok();
        std::cout << tok << " ";
        if (tok == tok_identifier)
            std::cout << Id << std::endl;
        else if (tok == tok_def)
            std::cout << "def" << std::endl;
        else if (tok == tok_extern)
            std::cout << "extern" << std::endl;
        else if (tok == tok_number)
            std::cout << Number << std::endl;
        else
          std::cout << static_cast<char>(tok) << std::endl;
    }
    std::cout << "finish" << std::endl;
    return 0;
}
