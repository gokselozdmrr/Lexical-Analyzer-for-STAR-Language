#include <stdio.h>
#include <ctype.h>
#include <string.h>

int peek(FILE *file) {
    int c = fgetc(file);
    ungetc(c, file);
    return c;
}

typedef enum {
    IDENTIFIER, INT_CONST, OPERATOR, LEFT_CURLY_BRACKET, RIGHT_CURLY_BRACKET,
    STRING, KEYWORD, END_OF_LINE, COMMA, END_OF_FILE
} TokenType;

typedef struct {
    TokenType type;
    char tokenCharList[1048];
    int lineNumber;
} Token;


int isKeyword(char *str) {
    char *keywords[] = {"int", "text", "is", "loop", "times", "read", "write", "newLine"};
    int i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Sonraki Tokeni bulan fonksiyon
Token getNextToken(FILE *file,FILE *outFile) {
    Token token;
    char c;
    int i = 0;

    
    // Bosluk ve yorum satirlarini atla
    while ((c = fgetc(file)) != EOF) {
        if (c == '/') { // Yorum satiri kontrolu
            c = fgetc(file);
            if (c == '*') {
                while ((c = fgetc(file)) != EOF) {
                    if (c == '*' && fgetc(file) == '/') {
                        break; // yorum satiri sonu
                    }
                }
                if (c == EOF) { // Yorum satiri sonlanmamis ise hata ver
                    token.type = END_OF_FILE;
                    fprintf(outFile, "Error: Unterminated comment\n");
                    return token;
                }
            } else {
                ungetc(c, file); // Yorum satiri degilse karakteri geri al
                c = '/';
                break;
            }
        } else if (!isspace(c)) {
            break;
        }
    }

    token.lineNumber = 1; // satir numarasi

    // Token tipini belirle
    if (isalpha(c) && c != '_' && c != '"') {
        // Identifier veya keyword
        while (isalnum(c) || c == '_') {
            token.tokenCharList[i++] = c;
            c = fgetc(file);
        }
        token.tokenCharList[i] = '\0';
        ungetc(c, file);
        token.type = isKeyword(token.tokenCharList) ? KEYWORD : IDENTIFIER;
    } else if (isdigit(c)) {
        // Integer
        while (isdigit(c)) {
            token.tokenCharList[i++] = c;
            c = fgetc(file);
        }
        token.tokenCharList[i] = '\0';
        ungetc(c, file);
        token.type = INT_CONST;
    } else if (c == '"') {
        // String 
        i = 0;
        while ((c = fgetc(file)) != '"' && c != EOF) {
            token.tokenCharList[i++] = c;
        }
        token.tokenCharList[i] = '\0';
        if (c == EOF) { // String kapanmamış ise 
            token.type = END_OF_FILE;
        } else {
            token.type = STRING;
        }
    } else if (c == '+' || c == '-' || c == '*' || c == '/') {
        // Operator veya negatif sayı kontrolü
        if (c == '-' && (token.type == IDENTIFIER || token.type == INT_CONST  || token.type == STRING )) { //Onceki token identifier veya int ise - operatoru
            token.tokenCharList[0] = c;
            token.tokenCharList[1] = '\0';
            token.type = OPERATOR;
        } else if(isdigit(peek(file)) ){ // Negatif sayı
            // Negatif sayı
            token.tokenCharList[i++] = c;
            while (isdigit(c = fgetc(file))) {        }
            token.tokenCharList[0] = '0';
            token.tokenCharList[1] = '\0';
            ungetc(c, file);
            token.type = INT_CONST;
        } else{ // onceki token farkli bir sey ama digit gelmiyorsa operator (Negatif sayi icin digit olmali)
            // Operator
            token.tokenCharList[0] = c;
            token.tokenCharList[1] = '\0';
            token.type = OPERATOR;
        }
    } else if (c == '{') {
        token.type = LEFT_CURLY_BRACKET;
    } else if (c == '}') {
        token.type = RIGHT_CURLY_BRACKET;
    } else if (c == '.') {
        token.type = END_OF_LINE;
    } else if (c == ',') {
        token.type = COMMA;
    } else if (c == EOF) {
        token.type = END_OF_FILE;
    } else {    }

    return token;
}

// Tokeni dosyaya yazan fonksiyon
void writeTokenToFile(Token token, FILE *outFile) {
    switch (token.type) {
        case IDENTIFIER:
            fprintf(outFile, "Identifier(%s)\n", token.tokenCharList);
            if(strlen(token.tokenCharList) > 10) {
                fprintf(outFile, "Error: Identifier is too long\n");
            }
            break;
        case INT_CONST:
            fprintf(outFile, "IntConst(%s)\n", token.tokenCharList);
            if (strlen(token.tokenCharList)  > 8)
            {
                fprintf(outFile, "Error: Integer constant is too long\n");
            }
            
            break;
        case OPERATOR:
            fprintf(outFile, "Operator(%s)\n", token.tokenCharList);
            break;
        case LEFT_CURLY_BRACKET:
            fprintf(outFile, "LeftCurlyBracket\n");
            break;
        case RIGHT_CURLY_BRACKET:
            fprintf(outFile, "RightCurlyBracket\n");
            break;
        case STRING:
            fprintf(outFile, "String(%s)\n", token.tokenCharList);
            if (strlen(token.tokenCharList) > 256)
            {
                fprintf(outFile, "Error: String is too long\n");
            }
            break;
        case KEYWORD:
            fprintf(outFile, "Keyword(%s)\n", token.tokenCharList);
            break;
        case END_OF_LINE:
            fprintf(outFile, "EndOfLine\n");
            break;
        case COMMA:
            fprintf(outFile, "Comma\n");
            break;
        default:
            break;
    }
}

int main() {
    FILE *inputFile = fopen("code.sta", "r");
    FILE *outputFile = fopen("code.lex", "w");

    Token token;

    while ((token = getNextToken(inputFile,outputFile)).type !=END_OF_FILE) {
        writeTokenToFile(token, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
