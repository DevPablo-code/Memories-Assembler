
#include <iostream>

#include "Lexer.h"
#include "registers.h"
#include "instructionsSet.h"
#include "toLower.h"
#include "getNumberSize.h"

Lexer::Lexer(std::string source):
source(source) 
{
}

bool Lexer::isEnd() 
{
	return current >= source.length();
}

char Lexer::peek()
{
	if (isEnd()) 
	{
		return '\0';
	}
	return source.at(current);
}

char Lexer::peekNext()
{
	if (current + 1 >= source.length())
	{
		return '\0';
	}
	return source.at(current + 1);
}

char Lexer::advance() 
{
	return source.at(current++);
}

void Lexer::nextLine() 
{
	line++;
}

std::vector<Token>& Lexer::tokenize()
{
	while(!isEnd()) 
	{
		tokenStart = current;
		nextToken();
	}
	tokens.push_back({ TokenType::END_OF_FILE, TokenGroup::MAIN, line, 0, 0, "EOF"});
	return tokens;
}

void Lexer::nextToken() 
{
	char c = advance();
	switch (c)
	{
		case ' ':
		case '\r':
		case '\t': 
		{
			break;
		}
		case '\n': 
		{
			nextLine();
			break;
		}
		case '+': 
		{
			if (tokens.size() == 0 || tokens.back().type == TokenType::ARITHMETIC_BINARY_OPERATOR || tokens.back().type == TokenType::ARITHMETIC_UNARY_OPERATOR || tokens.back().type == TokenType::LEFT_PAREN || tokens.back().type == TokenType::COMMA || tokens.back().type == TokenType::INSTRUCTION || tokens.back().type == TokenType::DATA_DEFINING_INSTRUCTION)
			{
				tokens.push_back({ TokenType::ARITHMETIC_UNARY_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "+" });
			}
			else
			{
				tokens.push_back({ TokenType::ARITHMETIC_BINARY_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 2, "+" });
			}
			break;
		}
		case '-':
		{
			if (tokens.size() == 0 || tokens.back().type == TokenType::ARITHMETIC_BINARY_OPERATOR || tokens.back().type == TokenType::ARITHMETIC_UNARY_OPERATOR || tokens.back().type == TokenType::LEFT_PAREN || tokens.back().type == TokenType::COMMA || tokens.back().type == TokenType::INSTRUCTION || tokens.back().type == TokenType::DATA_DEFINING_INSTRUCTION)
			{
				tokens.push_back({ TokenType::ARITHMETIC_UNARY_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "-" });
			}
			else
			{
				tokens.push_back({ TokenType::ARITHMETIC_BINARY_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 2, "-" });
			}
			break;
		}
		case '*':
		{
			tokens.push_back({ TokenType::ARITHMETIC_BINARY_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 1, "*" });
			break;
		}
		case '/':
		{
			tokens.push_back({ TokenType::ARITHMETIC_BINARY_OPERATOR, TokenGroup::ADDITIONAL,line, 0, 1, "/" });
			break;
		}
		case '^':
		{
			tokens.push_back({ TokenType::ARITHMETIC_BINARY_OPERATOR, TokenGroup::ADDITIONAL,line, 0, 0, "^" });
			break;
		}
		case '(': 
		{
			tokens.push_back({ TokenType::LEFT_PAREN, TokenGroup::ADDITIONAL, line, 0, 0, "(" });
			break;
		}
		case ')':
		{
			tokens.push_back({ TokenType::RIGHT_PAREN, TokenGroup::ADDITIONAL, line, 0, 0, ")" });
			break;
		}
		case '[':
		{
			tokens.push_back({ TokenType::LEFT_BRACKET, TokenGroup::ADDITIONAL, line, 0, 0, "[" });
			break;
		}
		case ']':
		{
			tokens.push_back({ TokenType::RIGHT_BRACKET, TokenGroup::ADDITIONAL, line, 0, 0, "]" });
			break;
		}
		case '&':
		{
			tokens.push_back({ TokenType::GETOFFSET_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "&" });
			break;
		}
		case '#': 
		{
			tokens.push_back({ TokenType::GETPROGRAMSIZE_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "#" });
			break;
		}
		case '$':
		{
			if (peekNext() == '$') 
			{
				tokens.push_back({ TokenType::GETSTARTADDRESS_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "$$" });
			}
			else 
			{
				tokens.push_back({ TokenType::GETCURRENTADDRESS_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "$" });
			}
			break;
		}
		case '@': 
		{
			tokens.push_back({ TokenType::DUPDATA_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "@" });
			break;
		}
		case '=':
		{
			tokens.push_back({ TokenType::EQUAL_OPERATOR, TokenGroup::ADDITIONAL, line, 0, 0, "=" });
			break;
		}
		case '%':
		{
			tokens.push_back({ TokenType::DEFINECTCONSTANT_OPERATOR, TokenGroup::MAIN, line, 0, 0, "%" });
			break;
		}
		case ',':
		{
			tokens.push_back({ TokenType::COMMA, TokenGroup::ADDITIONAL, line, 0, 0, "," });
			break;
		}
		case '\'':
		{
			makeString();
			break;
		}

		case ';':
		{
			comment();
			break;
		}
		default: 
		{
			if (isDigit(c))
			{
				makeNumber();
			}
			else if (isAlpha(c) || c == '.')
			{
				makeKeywordIdentifier();
			}
			break;
		}
	}
}

void Lexer::comment()
{
	while (peek() != '\n' && !isEnd())
	{
		advance();
	}
}

void Lexer::makeNumber() 
{
	while (isHexDigit(peek())) 
	{
		advance();
	}

	std::string numberString = source.substr(tokenStart, current - tokenStart);
	int64_t number;
	char base = peek();

	switch(base) 
	{
		case 'b': 
		{
			number = std::strtoll(numberString.c_str(), nullptr, 2);
			advance();
			break;
		}
		case 'q': 
		{
			number = std::strtoll(numberString.c_str(), nullptr, 8);
			advance();
			break;
		}
		case 'h': 
		{
			number = std::strtoll(numberString.c_str(), nullptr, 16);
			advance();
			break;
		}
		default: 
		{
			if (numberString.back() == 'b')
			{
				numberString.pop_back();
				number = std::strtoll(numberString.c_str(), nullptr, 2);
			}
			else
			{
				number = std::strtoll(numberString.c_str(), nullptr, 10);
			}
			break;
		}
	}
	if (number == LLONG_MIN || number == LLONG_MAX)
	{
		error(line, "Bad number: " + numberString);
	}
	else 
	{
		tokens.push_back({ TokenType::NUMBER, TokenGroup::ADDITIONAL, line, getNumberSize(number), number, numberString + base });
	}
}

void Lexer::makeString()
{
	char c = 0;
	while (c != '\'' && !isEnd())
	{
		c = advance();
	}
	if (c != '\'') 
	{
		error(line, "Unterminated string");
	}

	c = peek();

	std::string string = source.substr(tokenStart + 1, current - tokenStart - 2);

	for (const auto& c : string) 
	{
		tokens.push_back({ TokenType::NUMBER, TokenGroup::ADDITIONAL, line, 1, c, std::string(c, 1) });
		tokens.push_back({ TokenType::COMMA, TokenGroup::ADDITIONAL, line, 0, 0, "," });
	}
}

void Lexer::makeKeywordIdentifier()
{
	while (isAlphaNumeric(peek())) 
	{
		advance();
	}

	std::string identifierString = source.substr(tokenStart, current - tokenStart);
	std::string identifierStringLower = toLowerCopy(identifierString);

	if (const auto it = registers.find(identifierStringLower); it != registers.end())
	{
		makeRegister(identifierStringLower, it->second.size, it->second.index);
	}
	else if (const auto it = segmentRegisters.find(identifierStringLower); it != segmentRegisters.end())
	{
		makeSegmentRegister(identifierStringLower, it->second.size, it->second.index);
	}
	else if (const auto it = dataDefiningInstructions.find(identifierStringLower); it != dataDefiningInstructions.end()) {
		makeDataDefiningInstruction(it->first, it->second);
	}
	else if (!isEnd() && peek() == ':') 
	{
		advance();
		if (source.at(tokenStart) == '.')
		{
			makeLocalLabelDeclaration(identifierString);
		} 
		else
		{
			makeGlobalLabelDeclaration(identifierString);
		}
	}
	else if (checkInstuction(identifierStringLower)) 
	{
		makeInstruction(identifierStringLower);
	}
	else if (source.at(tokenStart) == '.') 
	{
		makeLocalLabel(identifierString);
	}
	else
	{
		makeGlobalLabel(identifierString);
	}
}

void Lexer::makeRegister(const std::string &name, uint8_t size, uint8_t index)
{
	tokens.push_back({ TokenType::REGISTER, TokenGroup::ADDITIONAL, line, size, index, name });
}

void Lexer::makeSegmentRegister(const std::string& name, uint8_t size, uint8_t index)
{
	tokens.push_back({ TokenType::SEGMENT_REGISTER, TokenGroup::ADDITIONAL, line, size, index, name });
}

void Lexer::makeGlobalLabelDeclaration(const std::string& name)
{
	tokens.push_back({ TokenType::GLOBAL_LABEL_DECLARATION, TokenGroup::MAIN, line, (uint8_t)name.size() , NULL, name });
}

void Lexer::makeLocalLabelDeclaration(const std::string& name)
{
	tokens.push_back({ TokenType::LOCAL_LABEL_DECLARATION, TokenGroup::MAIN, line, (uint8_t)name.size() , NULL, name });
}

void Lexer::makeGlobalLabel(const std::string &name)
{
	tokens.push_back({ TokenType::GLOBAL_LABEL, TokenGroup::ADDITIONAL, line, (uint8_t)name.size() , NULL, name });
}

void Lexer::makeLocalLabel(const std::string& name)
{
	tokens.push_back({ TokenType::LOCAL_LABEL,  TokenGroup::ADDITIONAL, line, (uint8_t)name.size() , NULL, name });
}

void Lexer::makeInstruction(const std::string& name)
{
	tokens.push_back({ TokenType::INSTRUCTION, TokenGroup::MAIN, line, 0 , NULL, name });
}

void Lexer::makeDataDefiningInstruction(const std::string& name, uint8_t size)
{
	tokens.push_back({ TokenType::DATA_DEFINING_INSTRUCTION, TokenGroup::MAIN, line, size , NULL, name });
}

bool Lexer::isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool Lexer::isBinaryDigit(char c)
{
	return c == '0' || c == '1';
}

bool Lexer::isOctalDigit(char c)
{
	return c >= '0' && c <= '8';
}

bool Lexer::isHexDigit(char c)
{
	return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool Lexer::isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) {
	return isDigit(c) || isAlpha(c);
}

void Lexer::error(uint16_t line, const std::string &message)
{
	std::cout << "Line " << line << ": " << message << '\n';
	exit(-1);
}
