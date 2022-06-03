#pragma once

#include <string>
#include <vector>

#include "Token.h"

class Lexer 
{
	public:
		Lexer(std::string source);
		std::vector<Token>& tokenize();
	private:
		std::vector<Token> tokens;

		uint16_t line = 1;
		uint16_t tokenStart = 0;
		uint16_t current = 0;

		std::string source;

		bool isEnd();
		char peek();
		char peekNext();
		char advance();
		void nextLine();
		void nextToken();

		void comment();

		void makeNumber();

		void makeString();

		void makeKeywordIdentifier();
		void makeRegister(const std::string& name, uint8_t size, uint8_t index);
		void makeSegmentRegister(const std::string& name, uint8_t size, uint8_t index);
		void makeGlobalLabel(const std::string& name);
		void makeLocalLabel(const std::string& name);
		void makeGlobalLabelDeclaration(const std::string& name);
		void makeLocalLabelDeclaration(const std::string& name);
		void makeInstruction(const std::string& name);
		void makeDataDefiningInstruction(const std::string& name, uint8_t size);

		static bool isDigit(char c);
		static bool isBinaryDigit(char c);
		static bool isOctalDigit(char c);
		static bool isHexDigit(char c);
		static bool isAlpha(char c);
		static bool isAlphaNumeric(char c);

		static void error(uint16_t line, const std::string& message);
};