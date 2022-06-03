#pragma once

#include "Token.h"

struct Node
{
	Token token;
	std::shared_ptr<Node> left;
	std::shared_ptr<Node> right;
};

struct Instruction
{
	Token token;
	std::vector<Node> arguments;
};