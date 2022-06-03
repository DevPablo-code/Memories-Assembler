#pragma once

#include <string>
#include <unordered_map>

struct Register 
{
	uint8_t size;
	uint8_t index;
};

const std::unordered_map<std::string, Register> registers = {
	{ "al", { 1, 0 }},
	{ "cl", { 1, 1 }},
	{ "dl", { 1, 2 }},
	{ "bl", { 1, 3 }},

	{ "ah", { 1, 4 }},
	{ "ch", { 1, 5 }},
	{ "dh", { 1, 6 }},
	{ "bh", { 1, 7 }},

	{ "ax", { 2, 0 }},
	{ "cx", { 2, 1 }},
	{ "dx", { 2, 2 }},
	{ "bx", { 2, 3 }},

	{ "sp", { 2, 4 }},
	{ "bp", { 2, 5 }},
	{ "si", { 2, 6 }},
	{ "di", { 2, 7 }},
};

const std::unordered_map<std::string, Register> segmentRegisters =
{

	{ "cs", { 2, 0 }},
	{ "ds", { 2, 1 }},
	{ "ss", { 2, 2 }},
	{ "es", { 2, 3 }},
};