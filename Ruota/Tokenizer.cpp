#include "Tokenizer.h"

Tokenizer::Tokenizer(std::unordered_map<std::string, int> operators)
{
	this->operators = operators;
}

std::vector<std::string> Tokenizer::tokenize(const std::string str)
{
	char pChar = 0;
	short mode = 0;
	char last_c = 0;
	bool comment = false;
	std::vector<std::string> tokens = {"#["};

	for (int i = 0; i < str.length(); i++)
	{
		const char c = str[i];
		if (comment)
		{
			if (c == '\n' || c == '\r')
			{
				comment = false;
				mode = 0;
			}
			continue;
		}
		if (pChar != 0)
		{
			if (c == pChar && last_c != '\\')
				pChar = 0;
			else
			{
				if (last_c == '\\')
				{
					tokens.back().pop_back();
					if (isdigit(c))
					{
						int new_c = (std::stoi(std::string(1, str[i])) * 100) + (std::stoi(std::string(1, str[i + 1])) * 10) + (std::stoi(std::string(1, str[i + 2])));
						i += 2;
						tokens.back().push_back(new_c);
						last_c = new_c;
						continue;
					}
					else
					{
						switch (c)
						{
						case '\\':
							tokens.back().push_back('\\');
							last_c = 0;
							continue;
							break;
						case 'v':
							tokens.back().push_back('\v');
							break;
						case 'f':
							tokens.back().push_back('\f');
							break;
						case 'e':
							tokens.back().push_back('\e');
							break;
						case 'b':
							tokens.back().push_back('\b');
							break;
						case '?':
							tokens.back().push_back('\?');
							break;
						case 'n':
							tokens.back().push_back('\n');
							break;
						case 't':
							tokens.back().push_back('\t');
							break;
						case 'r':
							tokens.back().push_back('\r');
							break;
						case 'a':
							tokens.back().push_back('\a');
							break;
						case '\'':
							tokens.back().push_back('\'');
							break;
						case '0':
							tokens.back().push_back('\0');
							break;
						case '"':
							tokens.back().push_back('"');
							break;
						default:
							tokens.back().push_back('\\');
							tokens.back().push_back(c);
							break;
						}
					}
				}
				else
					tokens.back().push_back(c);
			}
		}
		else
		{
			if (c == '#')
			{
				comment = true;
				continue;
			}
			if (isspace(c))
			{
				mode = 0;
				continue;
			}
			else if (c == '\"' || c == '\'' || c == '`')
			{
				mode = 0;
				pChar = c;
				tokens.push_back(std::string(1, c));
				continue;
			}
			/*if (c == ';')
			{
				tokens.push_back("#]");
				tokens.push_back("#[");
				last_c = '(';
				mode = 0;
				continue;
			}*/
			else if (c == '{')
			{
				tokens.push_back("{");
				tokens.push_back("#[");
				last_c = '(';
				mode = 0;
				continue;
			}
			else if (c == '}')
			{
				tokens.push_back("#]");
				tokens.push_back("}");
				tokens.push_back("#]");
				tokens.push_back("#[");
				last_c = '(';
				mode = 0;
				continue;
			}
			switch (mode)
			{
			case 0:
				tokens.push_back(std::string(1, c));
				break;
			case 1:
				if ((isalnum(c) || c == '_' || c == '~'))
					tokens.back().push_back(c);
				else
					tokens.push_back(std::string(1, c));
				break;
			case 2:
				if (!(isalnum(c) || c == '_' || c == '~') && operators.find(tokens.back() + std::string(1, c)) != operators.end())
					tokens.back().push_back(c);
				else
					tokens.push_back(std::string(1, c));
				break;
			}
			mode = !(isalnum(c) || c == '_' || c == '~') + 1;
		}
		last_c = c;
	}

	if (pChar != 0)
		throw std::runtime_error("Error: unclosed string or char literal!");

	std::vector<std::string> new_tokens;
	std::string last = "+";
	for (auto t : tokens)
	{
		if (t == "#[" || t == "#]")
			continue;
		if (t == "[" && ((isalnum(last[0]) || last[0] == '_' || last[0] == '~' || last[0] == '\"' || last[0] == '`') || last == ")" || last == "]" || last == "}" || last == ">") && (operators.find(last) == operators.end() || last == ">"))
			new_tokens.push_back(".index");
		if (t == "(" && ((isalnum(last[0]) || last[0] == '_' || last[0] == '~' || last[0] == '`') || last == ")" || last == "]" || last == "}" || last == ">") && (operators.find(last) == operators.end() || last == ">"))
			new_tokens.push_back(".exec");
		if (t == "-" && (operators.find(last) != operators.end() || last == "(" || last == "[" || last == "{"))
		{
			new_tokens.push_back(".negate");
			last = t;
			continue;
		}
		if (t == "+" && (operators.find(last) != operators.end() || last == "(" || last == "[" || last == "{"))
		{
			new_tokens.push_back(".positive");
			last = t;
			continue;
		}
		if (isdigit(t[0]) && last == "." && isdigit(new_tokens[new_tokens.size() - 2][0]))
		{
			new_tokens.pop_back();
			new_tokens.back().push_back('.');
			new_tokens.back() = new_tokens.back() + t + "D";
			last = t;
			continue;
		}
		new_tokens.push_back(t);
		last = t;
	}

	std::vector<std::string> post_generic;

	for (int i = 0; i < new_tokens.size(); i++)
	{
		if (new_tokens[i] == "<")
		{
			std::vector<std::string> temp;
			bool flag = true;
			int level = 1;
			for (i = i + 1; i < new_tokens.size(); i++)
			{
				if (new_tokens[i] == ">")
					level--;
				if (new_tokens[i] == "<")
					level++;
				if (level == 0)
					break;
				temp.push_back(new_tokens[i]);
				if (operators.find(new_tokens[i]) != operators.end() && new_tokens[i] != "." && new_tokens[i] != "<" && new_tokens[i] != ">" && new_tokens[i] != "," && new_tokens[i] != ".index")
					flag = false;
				if (new_tokens[i] == "{" || new_tokens[i] == "}" || new_tokens[i] == "(" || new_tokens[i] == ")")
					flag = false;
			}
			if (new_tokens.size() - 1 == i && level != 0)
				flag = false;
			if (flag)
			{
				post_generic.push_back(".generic");
				post_generic.push_back("[");
				for (auto &t : temp)
				{
					if (t == "<")
					{
						post_generic.push_back(".generic");
						post_generic.push_back("[");
					}
					else if (t == ">")
						post_generic.push_back("]");
					else
						post_generic.push_back(t);
				}
				post_generic.push_back("]");
			}
			else
			{
				post_generic.push_back("<");
				for (auto &t : temp)
					post_generic.push_back(t);
				if (level == 0)
					post_generic.push_back(">");
			}
		}
		else
		{
			post_generic.push_back(new_tokens[i]);
		}
	}

	//new_tokens.push_back("#]");
	return post_generic;
}

std::vector<std::string> Tokenizer::infixToPostfix(std::vector<std::string> tokens)
{
	std::vector<std::string> stack;
	std::vector<std::string> output;
	int p_count = 0;
	int b_count = 0;
	int c_count = 0;

	for (auto token : tokens)
	{
		if (((isalnum(token[0]) || token == "@" || token[0] == '_' || token[0] == '~') || token[0] == '\"' || token[0] == '\'' || token[0] == '`') && operators.find(token) == operators.end())
			output.push_back(token);
		else if (operators.find(token) != operators.end())
		{
			if (!stack.empty())
			{
				auto sb = stack.back();
				while ((std::abs(getPrecedence(token)) < std::abs(getPrecedence(sb)) || (getPrecedence(sb) > 0 && std::abs(getPrecedence(token)) == std::abs(getPrecedence(sb)))) && (sb != "(" && sb != "[" && sb != "{" && sb != "#<"))
				{
					output.push_back(sb);
					stack.pop_back();
					if (stack.empty())
						break;
					else
						sb = stack.back();
				}
			}
			stack.push_back(token);
		}
		else if (token == "(" || token == "[" || token == "{" || token == "#<")
		{
			stack.push_back(token);
			output.push_back(token);
			switch (token[0])
			{
			case '(':
				p_count++;
				break;
			case '[':
				b_count++;
				break;
			case '{':
				c_count++;
				break;
			}
		}
		else if (token == ")" || token == "]" || token == "}" || token == "#>")
		{
			switch (token[0])
			{
			case ')':
				p_count--;
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched paranthesis!");
				while (stack.back() != "(")
				{
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched paranthesis!");
				}
				break;
			case ']':
				b_count--;
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched bracket!");
				while (stack.back() != "[")
				{
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched bracket!");
				}
				break;
			case '#':
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched command bracket!");
				while (stack.back() != "#<")
				{
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched command bracket!");
				}
				break;
			case '}':
				c_count--;
				if (stack.empty())
					throw std::runtime_error("Error: Unmatched scoping bracket!");
				while (stack.back() != "{")
				{
					output.push_back(stack.back());
					stack.pop_back();
					if (stack.empty())
						throw std::runtime_error("Error: Unmatched scoping bracket!");
				}
				break;
			}
			stack.pop_back();
			output.push_back(token);
		}
	}

	if (p_count != 0)
		throw std::runtime_error("Error: Unmatched paranthesis!");
	if (b_count != 0)
		throw std::runtime_error("Error: Unmatched bracket!");
	if (c_count != 0)
		throw std::runtime_error("Error: Unmatched scoping bracket!");

	while (!stack.empty())
	{
		output.push_back(stack.back());
		stack.pop_back();
	}

	return output;
}

int Tokenizer::getPrecedence(std::string token)
{
	if (operators.find(token) != operators.end())
		return operators[token];

	return 999;
}