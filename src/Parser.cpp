
#include <iostream>
#include <unordered_map>

#include "Parser.h"
#include "Token.h"
#include "getNumberSize.h"

Parser::Parser(std::vector<Token>& tokens):
tokens(tokens)
{
}

std::vector<Instruction>& Parser::parse()
{
	while (!isEnd())
	{
		nextExpression();
		advance();
	}
	return instructions;
}

const Token& Parser::peek() 
{
	return tokens.at(current);
}

const Token& Parser::peekNext()
{
	if (current + 1 >= tokens.size())
	{
		return Token{};
	} 
	return tokens.at(current + 1);
}

void Parser::advance()
{
	current++;
}

void Parser::nextExpression() 
{
	switch (peek().type)
	{
		case TokenType::INSTRUCTION:
		{
			parseInstruction();
			break;
		}
		case TokenType::DATA_DEFINING_INSTRUCTION: 
		{
			parseDataDefiningInstruction();
			break;
		}
		case TokenType::LOCAL_LABEL_DECLARATION: 
		case TokenType::GLOBAL_LABEL_DECLARATION:
		{
			if (peekNext().type == TokenType::DATA_DEFINING_INSTRUCTION)
			{
				instructions.push_back({ Token { TokenType::DATA_LABEL_DECLARATION, TokenGroup::MAIN, peek().line, 0, 0, peek().stringValue }});
			}
			else
			{
				instructions.push_back({ peek() });
			}
			break;
		}
		case TokenType::DEFINECTCONSTANT_OPERATOR:
		{
			parseDefineCompileTimeConstant();
			break;
		}
		case TokenType::END_OF_FILE: 
		{
			break;
		}
		default: 
		{
			error(peek().line, "Unexpected token: " + peek().stringValue);
			break;
		}
	}
}

void Parser::parseInstruction()
{
	Token instructionToken = peek();

	std::vector<Node> arguments;

	if (peekNext().group == TokenGroup::ADDITIONAL) 
	{
		advance();
		while (!isEnd())
		{
			switch (peek().type)
				{
				case TokenType::NUMBER:  
				case TokenType::LEFT_PAREN:
				case TokenType::GETSTARTADDRESS_OPERATOR:
				case TokenType::GETCURRENTADDRESS_OPERATOR:
				case TokenType::GETPROGRAMSIZE_OPERATOR:
				case TokenType::ARITHMETIC_UNARY_OPERATOR:
				{
					arguments.push_back(parseArithmeticExpression());
					break;
				}
				case TokenType::REGISTER:
				case TokenType::SEGMENT_REGISTER:
				{
					arguments.push_back({ peek(), nullptr, nullptr });
					break;
				}
				case TokenType::LEFT_BRACKET:
				{
					advance();
					arguments.push_back({ Token{ TokenType::MEMORY_ADDRESSING, TokenGroup::ADDITIONAL, peek().line, 1, 0, "" }, nullptr, std::make_shared<Node>(parseMemoryAddressing()) });
					break;
				}
				case TokenType::COMMA:
				{
					break;
				}
				case TokenType::GETOFFSET_OPERATOR: {
					arguments.push_back(parseGetoffset());
					break;
				}
				case TokenType::LOCAL_LABEL:
				case TokenType::GLOBAL_LABEL:
				{
					if (const auto& it = compileTimeConstants.find(peek().stringValue); it != compileTimeConstants.end())
					{
						arguments.push_back({ it->second, nullptr, nullptr });
					}
					else
					{
						arguments.push_back({ peek(), nullptr, nullptr });
					}
					break;
				}
				default:
				{
					error(peek().line, "Unexpected token: " + peek().stringValue);
				}
			}
			if (peekNext().group != TokenGroup::ADDITIONAL)
			{
				break;
			}
			advance();
		}
	}

	instructions.push_back({ instructionToken, arguments });
}

void Parser::parseDataDefiningInstruction()
{

	Token instructionToken = peek();

	std::vector<Node> arguments;

	if (peekNext().group == TokenGroup::ADDITIONAL)
	{
		advance();
		while (!isEnd())
		{
			switch (peek().type)
			{
				case TokenType::NUMBER:
				case TokenType::LEFT_PAREN:
				case TokenType::GETSTARTADDRESS_OPERATOR:
				case TokenType::GETCURRENTADDRESS_OPERATOR:
				case TokenType::GETPROGRAMSIZE_OPERATOR:
				case TokenType::ARITHMETIC_UNARY_OPERATOR:
				{
					Node expression = parseArithmeticExpression();
					if (peek().type == TokenType::DUPDATA_OPERATOR) 
					{
						Token dupOperatorToken = peek();
						advance();
						switch (peek().type) 
						{
							case TokenType::NUMBER:
							case TokenType::LEFT_PAREN:
							case TokenType::GETSTARTADDRESS_OPERATOR:
							case TokenType::GETCURRENTADDRESS_OPERATOR:
							case TokenType::GETPROGRAMSIZE_OPERATOR:
							case TokenType::ARITHMETIC_UNARY_OPERATOR: 
							{
								Node secondExpression = parseArithmeticExpression();
								arguments.push_back(Node{ dupOperatorToken, std::make_shared<Node>(expression), std::make_shared<Node>(secondExpression) });
								break;
							}
							default:
							{
								error(peek().line, "Unexpected token: " + peek().stringValue);
							}
						}
					}
					else 
					{
						arguments.push_back(expression);
					}
					break;
				}
				case TokenType::LOCAL_LABEL:
				case TokenType::GLOBAL_LABEL:
				{
					if (const auto& it = compileTimeConstants.find(peek().stringValue); it != compileTimeConstants.end())
					{
						arguments.push_back({ it->second, nullptr, nullptr });
					}
					else 
					{
						error(peek().line, "Unexpected token: " + peek().stringValue);
					}
					break;
				}
				default:
				{
					error(peek().line, "Unexpected token: " + peek().stringValue);
				}
			}
			if (peekNext().group != TokenGroup::ADDITIONAL)
			{
				break;
			}
			advance();
		}
	}

	instructions.push_back({ instructionToken, arguments });
}

void Parser::parseDefineCompileTimeConstant()
{
	advance();

	if (peek().type != TokenType::LOCAL_LABEL && peek().type != TokenType::GLOBAL_LABEL) 
	{
		error(peek().line, "Unexpected token: " + peek().stringValue);
	}

	const std::string& name = peek().stringValue;

	advance();

	if (peek().type != TokenType::EQUAL_OPERATOR)
	{
		error(peek().line, "Unexpected token: " + peek().stringValue);
	}

	advance();

	const Token& value = peek();

	if (value.type != TokenType::NUMBER)
	{
		error(peek().line, "Unexpected token: " + value.stringValue);
	}

	compileTimeConstants.insert({ name, value });
}

Node Parser::parseGetoffset()
{
	const Token& ampersandToken = peek();

	advance();

	if (!isEnd() && (peek().type == TokenType::LOCAL_LABEL || peek().type == TokenType::GLOBAL_LABEL)) 
	{
		return { ampersandToken, nullptr, std::make_shared<Node>(Node { peek(), nullptr, nullptr }) };
	}
	else 
	{
		error(peek().line, "Unexpected token: " + peek().stringValue + " after &");
	}
}

Node Parser::parseArithmeticExpression()
{
	std::stack<Node> output;
	std::stack<Token> operators;

	while (!isEnd() && peek().group == TokenGroup::ADDITIONAL && peek().type != TokenType::COMMA && peek().type != TokenType::DUPDATA_OPERATOR) {
		switch (peek().type)
		{
			case TokenType::NUMBER:
			case TokenType::GETPROGRAMSIZE_OPERATOR:
			case TokenType::GETCURRENTADDRESS_OPERATOR:
			case TokenType::GETSTARTADDRESS_OPERATOR:
			{
				if (output.size() > 0 && operators.size() < 1)
				{
					error(peek().line, "No operator between operands");
				}
				output.push({ peek() });
				break;
			}
			case TokenType::ARITHMETIC_BINARY_OPERATOR:
			case TokenType::LEFT_PAREN:
				if
					(
						operators.size() > 0 && operators.top().type != TokenType::LEFT_PAREN
						&&
						operators.top().numberValue <= peek().numberValue
						)
				{
					popOperator(output, operators);
				}
			case TokenType::ARITHMETIC_UNARY_OPERATOR:
			{
				operators.push(peek());
				break;
			}
			case TokenType::RIGHT_PAREN:
			{
				while (true)
				{
					if (operators.top().type == TokenType::LEFT_PAREN) {
						operators.pop();
						break;
					}
					popOperator(output, operators);
					if (operators.size() == 0)
					{
						error(peek().line, "Could not find opening parenthesis");
					}
				}
				break;
			}
			case TokenType::LOCAL_LABEL:
			case TokenType::GLOBAL_LABEL:
			{
				if (const auto& it = compileTimeConstants.find(peek().stringValue); it != compileTimeConstants.end())
				{
					output.push({ it->second, nullptr, nullptr });
				}
				else
				{
					error(peek().line, "Unexpected token: " + peek().stringValue);
				}
				break;
			}
			default: {
				error(peek().line, "Unexpected token: " + peek().stringValue);
			}
		}
		if (peekNext().group != TokenGroup::ADDITIONAL)
		{
			break;
		}
		advance();
	}

	while (operators.size() > 0)
	{
		popOperator(output, operators);
	}

	return performArithmeticOperations(output.top());
}

Node Parser::parseMemoryAddressing()
{
	std::stack<Node> output;
	std::stack<Token> operators;

	std::unordered_map<std::string, bool> registersUsed = 
	{
		{ "bx", false },
		{ "si", false },
		{ "di", false },
		{ "bp", false },
		{ "sp", false },
	};

	while (!isEnd() && peek().type != TokenType::RIGHT_BRACKET)
	{
		switch (peek().type)
		{
			case TokenType::NUMBER:
			case TokenType::GETPROGRAMSIZE_OPERATOR:
			case TokenType::GETCURRENTADDRESS_OPERATOR:
			case TokenType::GETSTARTADDRESS_OPERATOR:
			{
				if (output.size() > 0 && operators.size() < 1) 
				{
					error(peek().line, "No operator between operands");
				}
				if (output.size() > 0 && output.top().token.type == TokenType::REGISTER && operators.top().stringValue == "-")
				{
					output.push(Node{ peek(), nullptr, nullptr });
					operators.top().stringValue = "+";
					output.top().token.numberValue *= -1;
				}
				else 
				{
					output.push(Node{ peek(), nullptr, nullptr });
				}
				break;
			}
			case TokenType::REGISTER:
			{
				if (output.size() > 0 && operators.size() < 1)
				{
					error(peek().line, "No operator between operands");
				}
				if (const auto &it = registersUsed.find(peek().stringValue); it != registersUsed.end()) 
				{
					if (output.size() > 0 && output.top().token.type == TokenType::NUMBER) 
					{
						error(peek().line, "Memory addressing should look like [register + displacement]");
					}
					else if (!it->second) 
					{
						output.push(Node{ peek(), nullptr, nullptr });
						it->second = true;
					}
				}
				else
				{
					error(peek().line, "Can only use bx, si and di registers. Got: " + peek().stringValue);
				}
				break;
			}
			case TokenType::ARITHMETIC_BINARY_OPERATOR:
			case TokenType::LEFT_PAREN:
				if
					(
						operators.size() > 0 && operators.top().type != TokenType::LEFT_PAREN
						&&
						operators.top().numberValue <= peek().numberValue
						)
				{
					popOperator(output, operators);
				}
			case TokenType::ARITHMETIC_UNARY_OPERATOR:
			{
				operators.push(peek());
				if (output.top().token.type == TokenType::REGISTER) 
				{
					operators.top().numberValue = 99;
				}
				break;
			}
			case TokenType::RIGHT_PAREN:
			{
				while (true)
				{
					if (operators.top().type == TokenType::LEFT_PAREN) {
						operators.pop();
						break;
					}
					popOperator(output, operators);
					if (operators.size() == 0)
					{
						error(peek().line, "Could not find opening parenthesis");
					}
				}
				break;
			}
			default:
			{
				error(peek().line, "Unexpected token: " + peek().stringValue);
			}
		}

		advance();
	}

	if (peek().type != TokenType::RIGHT_BRACKET) {
		error(peek().line, "Could not find closing bracket");
	}

	while (operators.size() > 0)
	{
		popOperator(output, operators);
	}

	return performArithmeticOperations(output.top());
}

void Parser::popOperator(std::stack<Node>& output, std::stack<Token>& operators)
{
	Token op = operators.top();
	if (op.type == TokenType::LEFT_PAREN) {
		error(op.line, "Could not find closing parenthesis");
	}
	operators.pop();

	Node right;
	Node left;
	int64_t result = 0;

	if (output.size() > 0)
	{
		right = output.top();
		output.pop();
	}
	if (output.size() > 0)
	{
		left = output.top();
		output.pop();
	}

	output.push(Node{ op, std::make_shared<Node>(left), std::make_shared<Node>(right) });
}

bool Parser::isEnd()
{
	return current >= tokens.size() - 1;
}

Node Parser::performArithmeticOperations(Node node, bool registersAsNumbers)
{
	const auto &token = node.token;
	const auto left = node.left;
	const auto right = node.right;

	if (token.type != TokenType::ARITHMETIC_BINARY_OPERATOR && token.type != TokenType::ARITHMETIC_UNARY_OPERATOR) 
	{
		return node;
	}
	return performArithmeticOperation(Node{ token, std::make_shared<Node>(performArithmeticOperations(*left, registersAsNumbers)), std::make_shared<Node>(performArithmeticOperations(*right, registersAsNumbers)) }, registersAsNumbers);
}

Node Parser::performArithmeticOperation(Node node, bool registersAsNumbers)
{
	const auto& op = node.token;
	const auto left = node.left;
	const auto right = node.right;

	int64_t result = 0;

	if ((left->token.type != TokenType::NONE && left->token.type != TokenType::NUMBER && !(registersAsNumbers && left->token.type == TokenType::REGISTER)) || (right->token.type != TokenType::NONE && right->token.type != TokenType::NUMBER && !(registersAsNumbers && right->token.type == TokenType::REGISTER)))
	{
		return node;
	}
	if (op.type == TokenType::ARITHMETIC_UNARY_OPERATOR) 
	{
		if (op.stringValue == "+") 
		{
			result = right->token.numberValue;
		}
		else if (op.stringValue == "-") 
		{
			result = right->token.numberValue * -1;
		}
	}
	else 
	{
		if (op.stringValue == "+")
		{
			result = left->token.numberValue + right->token.numberValue;
		}
		else if (op.stringValue == "-")
		{
			result = left->token.numberValue - right->token.numberValue;
		}
		else if (op.stringValue == "*")
		{
			result = left->token.numberValue * right->token.numberValue;
		}
		else if (op.stringValue == "/")
		{
			result = left->token.numberValue / right->token.numberValue;
		}
		else if(op.stringValue == "^") 
		{
			result = 1;
			for (int i = 0; i < abs(right->token.numberValue); i++) 
			{
				result *= left->token.numberValue;
			}
			if (right->token.numberValue < 0) 
			{
				result = 1 / result;
			}
		}
	}

	return Node{ Token { TokenType::NUMBER, TokenGroup::ADDITIONAL, op.line, getNumberSize(result), result, ""} };
}

void Parser::error(uint16_t line, const std::string& message)
{
	std::cout << "Line " << line << ": " << message << '\n';
	exit(-1);	
}
