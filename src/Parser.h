#pragma once

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <map>

#include "Token.h"
#include "Instruction.h"

class Parser
{
	public:
		Parser(std::vector<Token> &tokens);
		std::vector<Instruction>& parse();

		static Node performArithmeticOperations(Node node, bool registersAsNumbers = false);
		static Node performArithmeticOperation(Node node, bool registersAsNumbers = false);
	private:
		std::vector<Token>&tokens;

		std::map<std::string, const Token&> compileTimeConstants;

		std::vector<Instruction> instructions;

		uint16_t current = 0;   

		const Token& peek();
		const Token& peekNext();
		void advance();
		void nextExpression();
		void parseInstruction();
		void parseDataDefiningInstruction();
		void parseDefineCompileTimeConstant();

		Node parseGetoffset();

		Node parseMemoryAddressing();
		Node parseArithmeticExpression();
		void popOperator(std::stack<Node>& output, std::stack<Token>& operators);

		bool isEnd();

		static void error(uint16_t line, const std::string& message);
};