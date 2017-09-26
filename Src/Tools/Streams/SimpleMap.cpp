/**
 * This file implements a simple backend for the stream classes that work on
 * the "ConfigMap" format. It parses the following format and provides the data
 * as a simple syntax tree:
 *
 * map ::= record
 * record ::= field ';' { field ';' }
 * field ::= literal '=' ( literal | '{' record '}' | array )
 * array ::= '[' [ ( literal | '{' record '}' ) { ',' ( literal | '{' record '}' ) } [ ',' ] ']'
 * literal ::= '"' { anychar1 } '"' | { anychar2 }
 *
 * anychar1 must escape doublequotes and backslash with a backslash
 * anychar2 cannot contains whitespace and other characters used by the grammar.
 *
 * @author Thomas Röfer
 */

#include "SimpleMap.h"
#include <stdexcept>
#include "InStreams.h"
#include "Tools/Debugging/Debugging.h"

SimpleMap::Literal::operator In&() const
{
  if(!stream)
    stream = new InTextMemory(literal.c_str(), literal.size());
  else
    *(InTextMemory*)stream = InTextMemory(literal.c_str(), literal.size());
  return *stream;
}

SimpleMap::Record::~Record()
{
  for(std::unordered_map<std::string, Value*>::const_iterator i = begin(); i != end(); ++i)
    delete i->second;
}

SimpleMap::Array::~Array()
{
  for(std::vector<Value*>::const_iterator i = begin(); i != end(); ++i)
    delete *i;
}

void SimpleMap::nextChar()
{
  if(c || !stream.eof())
  {
    if(c == '\n')
    {
      ++row;
      column = 1;
    }
    else if(c == '\t')
      column += 8 - (column - 1) % 8;
    else if(c != '\r')
      ++column;

    if(stream.eof())
      c = 0;
    else
      stream.read(&c, 1);
  }
}

void SimpleMap::nextSymbol()
{
  for(;;)
  {
    // Skip whitespace
    while(c && isspace(c))
      nextChar();

    string = "";
    switch(c)
    {
      case 0:
        symbol = eof;
        return; // skip nextChar()
      case '=':
        symbol = equals;
        break;
      case ',':
        symbol = comma;
        break;
      case ';':
        symbol = semicolon;
        break;
      case '[':
        symbol = lBracket;
        break;
      case ']':
        symbol = rBracket;
        break;
      case '{':
        symbol = lBrace;
        break;
      case '}':
        symbol = rBrace;
        break;
      case '"':
        string = c;
        nextChar();
        while(c && c != '"')
        {
          if(c == '\\')
            nextChar();
          if(c)
          {
            string += c;
            nextChar();
          }
        }
        if(!c)
          throw std::logic_error("Unexpected EOF in string");
        string += c;
        symbol = literal;
        break;

      case '/':
        nextChar();
        if(c == '*')
        {
          nextChar();
          char prevChar = 0;
          while(c && (c != '/' || prevChar != '*'))
          {
            prevChar = c;
            nextChar();
          }
          if(!c)
            throw new std::logic_error("Unexpected EOF in comment");
          nextChar();
          continue; // jump back to skipping whitespace
        }
        else if(c == '/')
        {
          nextChar();
          while(c && c != '\n')
            nextChar();
          if(!c)
            nextChar();
          continue; // jump back to skipping whitespace
        }
        string = "/";
        // no break;

      default:
        while(c && !isspace(c) && c != '=' && c != ',' && c != ';' && c != ']' && c != '}')
        {
          string += c;
          nextChar();
        }
        symbol = literal;
        return; // skip nextChar
    }

    nextChar();
    return;
  }
}

void SimpleMap::unexpectedSymbol()
{
  if(symbol == literal)
    throw std::logic_error(std::string("Unexpected literal '") + string + "'");
  else
    throw std::logic_error(std::string("Unexpected symbol '") + getName(symbol) + "'");
}

void SimpleMap::expectSymbol(Symbol expected)
{
  if(expected != symbol)
    unexpectedSymbol();
  nextSymbol();
}

SimpleMap::Record* SimpleMap::parseRecord()
{
  Record* r = new Record;
  try
  {
    while(symbol == literal)
    {
      std::string key = string;
      nextSymbol();
      if(r->find(key) != r->end())
        throw std::logic_error(std::string("dublicate attribute '") + key + "'");
      expectSymbol(equals);
      if(symbol == literal)
      {
        (*r)[key] = new Literal(string);
        nextSymbol();
      }
      else if(symbol == lBrace)
      {
        nextSymbol();
        (*r)[key] = parseRecord();
        expectSymbol(rBrace);
      }
      else if(symbol == lBracket)
        (*r)[key] = parseArray();
      else
        unexpectedSymbol();
      expectSymbol(semicolon);
    }
  }
  catch(const std::logic_error& e)
  {
    delete r;
    throw e;
  }
  return r;
}

SimpleMap::Array* SimpleMap::parseArray()
{
  nextSymbol();
  Array* a = new Array();
  try
  {
    while(symbol == literal || symbol == lBrace)
    {
      if(symbol == literal)
      {
        a->push_back(new Literal(string));
        nextSymbol();
      }
      else if(symbol == lBrace)
      {
        nextSymbol();
        a->push_back(parseRecord());
        expectSymbol(rBrace);
      }
      if(symbol != rBracket)
        expectSymbol(comma);
    }
    expectSymbol(rBracket);
  }
  catch(const std::logic_error& e)
  {
    delete a;
    throw e;
  }
  return a;
}

SimpleMap::SimpleMap(In& stream, const std::string& name) :
  stream(stream), c(0), row(1), column(0), root(0)
{
  try
  {
    nextChar();
    nextSymbol();
    root = parseRecord();
  }
  catch(const std::logic_error& e)
  {
    OUTPUT_ERROR(name << "(" << row << ", " << column << "): " << e.what());
  }
}

SimpleMap::~SimpleMap()
{
  if(root)
    delete root;
}
